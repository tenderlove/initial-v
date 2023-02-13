#include <stdio.h>
#include <driver/twai.h>
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include <shifter.h>
#include <BleKeyboard.h>
#include "NimBLECharacteristic.h"

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
    RESET,
    DRIVE,
    NEUTRAL,
    REVERSE,
    PARK,
} handle_state_t;

#define PRESSED (1 << 7)
#define BUTTON_MASK (PRESSED - 1)

handle_state_t handle_mode = NONE;
handle_position_t current_pos = SHIFTER_CENTER;
uint16_t command_pos = 0;

uint8_t brightness = 0xFF;

class HIDDataCallbacks : public NimBLECharacteristicCallbacks
{
    public:
        HIDDataCallbacks(void) { }

        void onWrite(NimBLECharacteristic* me) {
            size_t len = me->getDataLength();
            const uint8_t *buff = me->getValue()->data();
            handle_state_t tx_action;

            handle_state_t requested_state = (handle_state_t)buff[0];

            switch (requested_state) {
                case NONE:
                case RESET:
                    tx_action = RESET;
                    xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
                    break;
                case BACKLIGHT:
                    if (len > 1) {
                        brightness = buff[1];
                    } else {
                        brightness = 0xFF;
                    }
                    tx_action = BACKLIGHT;
                    xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
                    break;
                case DRIVE:
                case NEUTRAL:
                case REVERSE:
                case PARK:
                    handle_mode = requested_state;
                    break;
            }

            printf("onwrite!!\n");
            for (int i = 0; i < len; i++) {
                printf("onWrite byte: %d\n", buff[i]);
            }
            printf("done!!\n");
        }
};

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
        handle_position_t pos = SHIFTER_CENTER;
        if (SHIFTER_POSITION(message, &pos)) {
            if (current_pos != pos) {
                handle_position_t to_pos = (handle_position_t)((uint8_t)pos & SHIFTER_POSITION_MASK);
                handle_position_t from_pos = (handle_position_t)((uint8_t)current_pos & SHIFTER_POSITION_MASK);

                switch(to_pos) {
                    case SHIFTER_CENTER:
                        // Side -> Center, record it
                        if (from_pos == SHIFTER_SIDE) {
                            // Center is 0, so we need to specifically do this
                            command_pos = to_pos;
                            xQueueSend(buttons_queue, &command_pos, portMAX_DELAY);
                            break;
                        }

                        if (command_pos) {
                            printf("Send the key for pos %d\n", command_pos);
                            xQueueSend(buttons_queue, &command_pos, portMAX_DELAY);
                            command_pos = 0;
                        }
                        if (pos != to_pos) { // Pressed park
                            printf("Park pressed\n");
                            command_pos = pos;
                        }
                        break;
                    case SHIFTER_UP:
                        // Center -> Up, record it
                        if (from_pos == SHIFTER_CENTER) {
                            command_pos = to_pos;
                        }
                        // UpUp -> Up, ignore
                        break;
                    case SHIFTER_UP_UP:
                        command_pos = to_pos;
                        break;
                    case SHIFTER_DOWN:
                        // Center -> Down, record it
                        if (from_pos == SHIFTER_CENTER) {
                            command_pos = to_pos;
                        }
                        // DownDown -> Down, ignore
                        break;
                    case SHIFTER_DOWN_DOWN:
                        command_pos = to_pos;
                        break;
                    case SHIFTER_SIDE:
                        // Center -> Side, record it
                        if (from_pos == SHIFTER_CENTER) {
                            command_pos = to_pos;
                        }

                        if (command_pos) {
                            xQueueSend(buttons_queue, &command_pos, portMAX_DELAY);
                            command_pos = 0;
                        }
                        if (pos != to_pos) { // Pressed park
                            printf("Side Park pressed\n");
                            command_pos = pos;
                        }
                        break;
                    case SHIFTER_SIDE_UP:
                        command_pos = to_pos;
                        break;
                    case SHIFTER_SIDE_DOWN:
                        command_pos = to_pos;
                        break;
                    case SHIFTER_MAX:
                        break;
                    default:
                        break;
                }
            }

            current_pos = pos;
        }
    }
}

typedef void selection_handler_t(handle_position_t, handle_position_t);

// DRIVE means the user was in NORMAL mode.
static void
vim_drive_mode(handle_position_t from, handle_position_t to)
{
    handle_position_t no_park = (handle_position_t)(to & SHIFTER_POSITION_MASK);

    // Save the file
    if (to == SHIFTER_CENTER_PARK) {
        kb->press(':');
        kb->release(':');
        kb->press('w');
        kb->release('w');
        kb->press(KEY_RETURN);
        kb->release(KEY_RETURN);
        return;
    }

    switch(no_park) {
        case SHIFTER_CENTER:
            kb->press(KEY_ESC);
            kb->release(KEY_ESC);
            break;
        case SHIFTER_UP_UP:
            kb->press('i');
            kb->release('i');
            break;
        case SHIFTER_DOWN_DOWN:
            kb->press('o');
            kb->release('o');
            break;
        case SHIFTER_SIDE:
            kb->press(KEY_LEFT_CTRL);
            kb->press('v');
            kb->releaseAll();
            break;
        case SHIFTER_SIDE_UP:
        case SHIFTER_UP:
            kb->press(KEY_UP_ARROW);
            kb->release(KEY_UP_ARROW);
            break;
        case SHIFTER_SIDE_DOWN:
        case SHIFTER_DOWN:
            kb->press(KEY_DOWN_ARROW);
            kb->release(KEY_DOWN_ARROW);
            break;
        default:
            break;
    }
}

// NEUTRAL means the user was in INSERT mode.
static void
vim_neutral_mode(handle_position_t from, handle_position_t to)
{
    handle_position_t no_park = (handle_position_t)(to & SHIFTER_POSITION_MASK);

    // If they hit park while in drive mode, hit the ESC button
    if (to & PARK_BUTTON_BIT) {
        kb->press(KEY_ESC);
        kb->release(KEY_ESC);
    }

    switch(no_park) {
        case SHIFTER_UP_UP:
            kb->press(KEY_PAGE_UP);
            kb->release(KEY_PAGE_UP);
            break;
        case SHIFTER_DOWN_DOWN:
            kb->press(KEY_PAGE_DOWN);
            kb->release(KEY_PAGE_DOWN);
            break;
        case SHIFTER_UP:
            kb->press(KEY_UP_ARROW);
            kb->release(KEY_UP_ARROW);
            break;
        case SHIFTER_DOWN:
            kb->press(KEY_DOWN_ARROW);
            kb->release(KEY_DOWN_ARROW);
            break;
        default:
            break;
    }
}

static void
vim_reverse_mode(handle_position_t from, handle_position_t to)
{
}

static void
vim_park_mode(handle_position_t from, handle_position_t to)
{
    // Save the file
    if (to == SHIFTER_CENTER_PARK) {
        kb->press(':');
        kb->release(':');
        kb->press('q');
        kb->release('q');
        kb->press(KEY_RETURN);
        kb->release(KEY_RETURN);
        return;
    }

    vim_drive_mode(from, to);
}

selection_handler_t * vim_lut[(PARK - DRIVE) + 1] = {
    vim_drive_mode,    // NORMAL mode
    vim_neutral_mode,  // INSERT mode
    vim_reverse_mode,  // ??? mode
    vim_park_mode,     // NORMAL mode, but buffer is saved
};

void
kb_transmit_task(void *arg)
{
    while (1) {
        uint16_t pressed;
        xQueueReceive(buttons_queue, &pressed, portMAX_DELAY);

        if (kb->isConnected()) {
            selection_handler_t * cb = vim_lut[handle_mode - DRIVE];
            if (cb) {
                printf("found handler\n");
                handle_position_t from;
                handle_position_t to;

                from = (handle_position_t)(pressed >> 8);
                to = (handle_position_t)(pressed & 0xFF);
                cb(from, to);
            }
            else {
                printf("No handler for handle state %d\n", handle_mode);
            }
        }
        else {
            printf("KB Not connected\n");
        }
    }
}

void
transmit_task(void *arg)
{
    while (1) {
        handle_state_t action;
        xQueueReceive(tx_task_queue, &action, portMAX_DELAY);

        printf("transmitting %d\n", action);

        switch(action) {
            case BACKLIGHT:
                printf("transmitting %d brightness %d\n", action, brightness);
                shifter_send_light(0, brightness);
                break;
            case RESET:
                shifter_send_reset();
                break;
            case DRIVE:
                shifter_send_drive(true);
                break;
            case NEUTRAL:
                shifter_send_neutral();
                break;
            case REVERSE:
                shifter_send_reverse();
                break;
            case PARK:
                shifter_send_park();
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
    //twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    // We only care about 0x197 messages
    twai_filter_config_t f_config = { .acceptance_code = ((uint32_t)0x197 << 21),
                                      .acceptance_mask = ~(TWAI_STD_ID_MASK << 21),
                                      .single_filter   = true };

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

    kb = new BleKeyboard("Initial V", "Adequate INC", 100, new HIDDataCallbacks());
    kb->begin();

    xTaskCreatePinnedToCore(receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(kb_transmit_task, "kb_tx", 4096, NULL, KB_TASK_PRIO, NULL, tskNO_AFFINITY);
    TimerHandle_t handle = xTimerCreate("Update Timer", pdMS_TO_TICKS(250), pdTRUE, (void *)0, timer_callback);
    xTimerStart(handle, 0);
    xSemaphoreGive(ctrl_task_sem);
    vTaskDelay(pdMS_TO_TICKS(100));
    xSemaphoreTake(ctrl_task_sem, portMAX_DELAY);   //Wait for completion
}
