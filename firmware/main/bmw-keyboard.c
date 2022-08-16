#include <stdio.h>
#include <driver/twai.h>
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include <bmw_crc.h>

static SemaphoreHandle_t ctrl_task_sem;
static QueueHandle_t tx_task_queue;
uint8_t counter = 0;

#define TX_TASK_PRIO                    9
#define CTRL_TASK_PRIO                  10

typedef enum {
    NONE,
    BACKLIGHT,
    DRIVE,
    PARK,
    RESET,
} tx_task_action_t;

tx_task_action_t handle_state = NONE;

static void
send_light(uint8_t counter)
{
    uint8_t buf[] = { 0, 0xFF, 0x00, 0x00 };
    buf[0] = counter;

    printf("light crc %d\n", crc8(0, buf, 4));
    twai_message_t message;
    message.identifier = 0x202;
    message.extd = 0;
    message.ss = 1;
    message.data_length_code = 5;
    message.data[0] = crc8(0, buf, 4);
    for (int i = 0; i < 4; i++) {
        message.data[i + 1] = buf[i];
    }

    //Queue message for transmission
    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) != ESP_OK) {
        printf("Failed to queue message for transmission\n");
    }

    while (1) {
        uint32_t alerts;
        if (twai_read_alerts(&alerts, pdMS_TO_TICKS(1000)) == ESP_OK) {
            if ((alerts & TWAI_ALERT_TX_SUCCESS) == TWAI_ALERT_TX_SUCCESS)
            {
                printf("Grreat!\n");
                break;
            }
            if ((alerts & TWAI_ALERT_TX_FAILED) == TWAI_ALERT_TX_FAILED)
            {
                printf("Not Great!\n");
                break;
            }
        }
        else {
            printf("Failed to read alerts\n");
        }
    }

    return;
}

static void
send_drive(void)
{
    twai_message_t message;
    message.identifier = 0x3FD;
    message.extd = 0;
    message.data_length_code = 4;
    message.data[1] = counter;
    message.data[2] = 0x80;
    message.data[3] = 0x00;
    message.data[4] = 0x00;
    message.data[0] = crc8(0, &message.data[1], 4);

    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) != ESP_OK) {
        printf("Failed to queue message for transmission\n");
    }
    counter++;
    if ((counter & 0xF) == 0xF)
        counter++;
}

static void
send_park(void)
{
    twai_message_t message;
    message.identifier = 0x3FD;
    message.extd = 0;
    message.data_length_code = 4;
    message.data[1] = counter;
    message.data[2] = 0x20;
    message.data[3] = 0x00;
    message.data[4] = 0x00;
    message.data[0] = crc8(0, &message.data[1], 4);

    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) != ESP_OK) {
        printf("Failed to queue message for transmission\n");
    }
    counter++;
    if ((counter & 0xF) == 0xF)
        counter++;
}

static void
send_reset(void)
{
    twai_message_t message;
    message.identifier = 0x6F1;
    message.extd = 0;
    message.data_length_code = 4;
    message.data[0] = 0x5e;
    message.data[1] = 0x02;
    message.data[2] = 0x11;
    message.data[3] = 0x01;

    //Queue message for transmission
    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    } else {
        printf("Failed to queue message for transmission\n");
    }

    while (1) {
        uint32_t alerts;
        if (twai_read_alerts(&alerts, pdMS_TO_TICKS(1000)) == ESP_OK) {
            if ((alerts & TWAI_ALERT_TX_SUCCESS) == TWAI_ALERT_TX_SUCCESS)
            {
                printf("Grreat!\n");
                break;
            }
            if ((alerts & TWAI_ALERT_TX_FAILED) == TWAI_ALERT_TX_FAILED)
            {
                printf("Not Great!\n");
                break;
            }
        }
        else {
            printf("Failed to read alerts\n");
        }
    }
    return;
}

void
ctrl_task(void *arg)
{
    tx_task_action_t tx_action;

    xSemaphoreTake(ctrl_task_sem, portMAX_DELAY);

    tx_action = BACKLIGHT;
    xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);

    while(1) {
        //Wait for message to be received
        twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK) {
            //printf("Message received\n");
        } else {
            printf("Failed to receive message\n");
            continue;
        }

        //Process received message
        printf("rtr: %d ID: %#05lx  ", message.rtr, message.identifier);
        printf("DL: %d", message.data_length_code);
        for (int i = 0; i < message.data_length_code; i++) {
            printf("  %#04x", message.data[i]);
        }
        if (message.data_length_code == 4 && message.data[2] == 0x3e) {
            handle_state = DRIVE;
        }
        if (message.data_length_code == 4 && message.data[3] == 0xd5) {
            handle_state = PARK;
        }
        printf("\n");
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
                send_light(0);
                break;
            case RESET:
                send_reset();
                break;
            case DRIVE:
                send_drive();
                break;
            case PARK:
                send_park();
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

    switch(handle_state) {
        case BACKLIGHT:
            send_light(0);
            break;
        case RESET:
            send_reset();
            break;
        case DRIVE:
            tx_action = DRIVE;
            xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
            break;
        case PARK:
            tx_action = PARK;
            xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
            break;
        default:
            break;
    }
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

    xTaskCreatePinnedToCore(ctrl_task, "TWAI_ctrl", 4096, NULL, CTRL_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    TimerHandle_t handle = xTimerCreate("Update Timer", pdMS_TO_TICKS(100), pdTRUE, (void *)0, timer_callback);
    xTimerStart(handle, 0);
    xSemaphoreGive(ctrl_task_sem);
    vTaskDelay(pdMS_TO_TICKS(100));
    xSemaphoreTake(ctrl_task_sem, portMAX_DELAY);   //Wait for completion
}
