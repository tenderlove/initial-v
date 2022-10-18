#include <stdio.h>
#include <driver/twai.h>
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include <shifter.h>
#include <BleKeyboard.h>

static SemaphoreHandle_t ctrl_task_sem;
static QueueHandle_t tx_task_queue;
static QueueHandle_t buttons_queue;
static BleKeyboard * kb;

#define KB_TASK_PRIO                    8
#define TX_TASK_PRIO                    9
#define RX_TASK_PRIO                  10

typedef enum {
    NONE,
    BACKLIGHT,
    DRIVE,
    RESET,
} handle_state_t;

#define PRESSED (1 << 7)
#define BUTTON_MASK (PRESSED - 1)

uint8_t key_lut[] = {
    'r', // CENTER
    'u', // UP
    'U', // UP_UP
    'd', // DOWN
    'D', // DOWN_DOWN
    'x', // SIDE_UP
    'y', // SIDE_DOWN
    'l', // SIDE
};

handle_state_t handle_mode = NONE;
handle_position_t current_pos = SHIFTER_CENTER;

void
dump_message(twai_message_t message)
{
    printf("rtr: %d ID: %#05lx  ", message.rtr, message.identifier);
    printf("DL: %d", message.data_length_code);
    for (int i = 0; i < message.data_length_code; i++) {
        printf("  %#04x", message.data[i]);
    }
    printf("\n");
}

void
receive_task(void *arg)
{
    handle_state_t tx_action;

    xSemaphoreTake(ctrl_task_sem, portMAX_DELAY);

    tx_action = BACKLIGHT;
    xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);

    while(1) {
        //Wait for message to be received
        twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(10000)) != ESP_OK) {
            printf("Failed to receive message\n");
            continue;
        }

        //Process received message
        handle_position_t pos;
        if (SHIFTER_POSITION(message, &pos)) {
            if (current_pos != pos) {
                uint8_t button = pos;
                handle_position_t to_pos = (handle_position_t)((uint8_t)pos & SHIFTER_POSITION_MASK);
                handle_position_t from_pos = (handle_position_t)((uint8_t)current_pos & SHIFTER_POSITION_MASK);

                // Center -> side
                if (to_pos == SHIFTER_SIDE && from_pos == SHIFTER_CENTER) {
                    uint8_t press = button | PRESSED;
                    xQueueSend(buttons_queue, &press, portMAX_DELAY);
                    vTaskDelay(pdMS_TO_TICKS(3));
                    xQueueSend(buttons_queue, &button, portMAX_DELAY);
                }
                else {
                    // Side -> center
                    if (to_pos == SHIFTER_CENTER && from_pos == SHIFTER_SIDE) {
                        uint8_t press = button | PRESSED;
                        xQueueSend(buttons_queue, &press, portMAX_DELAY);
                        vTaskDelay(pdMS_TO_TICKS(3));
                        xQueueSend(buttons_queue, &button, portMAX_DELAY);
                    }
                    else {
                        // Center -> something else == button press
                        if (from_pos == SHIFTER_CENTER) {
                            button |= PRESSED;
                        }

                        // something else -> center == release previous button
                        if (to_pos == SHIFTER_CENTER) {
                            button = current_pos;
                        }
                        xQueueSend(buttons_queue, &button, portMAX_DELAY);
                    }
                }
            }

            current_pos = pos;
        }
        else {
            dump_message(message);
        }
    }
}

void
kb_transmit_task(void *arg)
{
    while (1) {
        uint8_t pressed;
        uint8_t button;
        xQueueReceive(buttons_queue, &pressed, portMAX_DELAY);

        button = pressed & BUTTON_MASK;

        uint8_t key = key_lut[button];

        if (kb->isConnected()) {
            if (pressed & PRESSED) {
                printf("pressed! ");
                kb->press(key);
            }
            else {
                printf("released! ");
                kb->release(key);
            }
            printf("%d key: %d\n", button, key);
        }
    }
}

void
transmit_task(void *arg)
{
    while (1) {
        handle_state_t action;
        xQueueReceive(tx_task_queue, &action, portMAX_DELAY);

        switch(action) {
            case BACKLIGHT:
                shifter_send_light(0);
                break;
            case RESET:
                shifter_send_reset();
                break;
            case DRIVE:
                shifter_send_drive(true);
                break;
            case NONE:
                break;
        }
    }
}

static void
timer_callback(TimerHandle_t pxTimer)
{
    handle_state_t tx_action;

    tx_action = handle_mode;
    xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
}

extern "C" void app_main(void)
{
    ctrl_task_sem = xSemaphoreCreateBinary();
    tx_task_queue = xQueueCreate(1, sizeof(uint32_t));
    buttons_queue = xQueueCreate(1, sizeof(uint32_t));

    //Initialize configuration structures using macro initializers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_22, GPIO_NUM_19, TWAI_MODE_NORMAL);
    g_config.alerts_enabled |= TWAI_ALERT_TX_SUCCESS | TWAI_ALERT_TX_FAILED;

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }

    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }

    handle_mode = DRIVE;

    kb = new BleKeyboard("Initial V", "Adequate INC", 100);
    kb->begin();

    xTaskCreatePinnedToCore(receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(kb_transmit_task, "kb_tx", 4096, NULL, KB_TASK_PRIO, NULL, tskNO_AFFINITY);
    TimerHandle_t handle = xTimerCreate("Update Timer", pdMS_TO_TICKS(100), pdTRUE, (void *)0, timer_callback);
    xTimerStart(handle, 0);
    xSemaphoreGive(ctrl_task_sem);
    vTaskDelay(pdMS_TO_TICKS(100));
    xSemaphoreTake(ctrl_task_sem, portMAX_DELAY);   //Wait for completion
}
