#include <stdio.h>
#include <driver/twai.h>
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include <shifter.h>

static SemaphoreHandle_t ctrl_task_sem;
static QueueHandle_t tx_task_queue;

#define TX_TASK_PRIO                    9
#define RX_TASK_PRIO                  10

typedef enum {
    NONE,
    BACKLIGHT,
    DRIVE,
    PARK,
    RESET,
} tx_task_action_t;

tx_task_action_t handle_state = NONE;

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
    tx_task_action_t tx_action;

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
        if (message.identifier == 0x197) {
            if (!SHIFTER_CENTER_P(message)) {
                dump_message(message);
            }
            if (SHIFTER_BACK_P(message)) {
                handle_state = DRIVE;
            }

            if (SHIFTER_PARK_P(message)) {
                handle_state = PARK;
            }
        }
        else {
            dump_message(message);
        }
    }
}

void
transmit_task(void *arg)
{
    while (1) {
        tx_task_action_t action;
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
            case PARK:
                shifter_send_park();
                break;
            default:
                break;
        }
    }
}

static void
timer_callback(TimerHandle_t pxTimer)
{
    tx_task_action_t tx_action;

    tx_action = handle_state;
    xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
}

void app_main(void)
{
    ctrl_task_sem = xSemaphoreCreateBinary();
    tx_task_queue = xQueueCreate(1, sizeof(tx_task_action_t));

    //Initialize configuration structures using macro initializers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
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

    xTaskCreatePinnedToCore(receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    TimerHandle_t handle = xTimerCreate("Update Timer", pdMS_TO_TICKS(100), pdTRUE, (void *)0, timer_callback);
    xTimerStart(handle, 0);
    xSemaphoreGive(ctrl_task_sem);
    vTaskDelay(pdMS_TO_TICKS(100));
    xSemaphoreTake(ctrl_task_sem, portMAX_DELAY);   //Wait for completion
}
