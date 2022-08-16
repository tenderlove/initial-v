#include <stdio.h>
#include <driver/twai.h>
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

int drive = 0;
static SemaphoreHandle_t ctrl_task_sem;
static QueueHandle_t tx_task_queue;

#define TX_TASK_PRIO                    9
#define CTRL_TASK_PRIO                  10

typedef enum {
    BACKLIGHT,
    RESET,
} tx_task_action_t;

/*
def make_lut
  256.times.map { |crc|
    8.times.inject(crc) { |i,|
      (i & 0x80 > 0 ? i << 1 ^ 0x1D : i << 1) & 0xFF
    }
  }
end
*/

#include <crc.h>

const uint8_t crc8_lut[] = {
    0x00, 0x1d, 0x3a, 0x27, 0x74, 0x69, 0x4e, 0x53, 0xe8, 0xf5, 0xd2, 0xcf,
    0x9c, 0x81, 0xa6, 0xbb, 0xcd, 0xd0, 0xf7, 0xea, 0xb9, 0xa4, 0x83, 0x9e,
    0x25, 0x38, 0x1f, 0x02, 0x51, 0x4c, 0x6b, 0x76, 0x87, 0x9a, 0xbd, 0xa0,
    0xf3, 0xee, 0xc9, 0xd4, 0x6f, 0x72, 0x55, 0x48, 0x1b, 0x06, 0x21, 0x3c,
    0x4a, 0x57, 0x70, 0x6d, 0x3e, 0x23, 0x04, 0x19, 0xa2, 0xbf, 0x98, 0x85,
    0xd6, 0xcb, 0xec, 0xf1, 0x13, 0x0e, 0x29, 0x34, 0x67, 0x7a, 0x5d, 0x40,
    0xfb, 0xe6, 0xc1, 0xdc, 0x8f, 0x92, 0xb5, 0xa8, 0xde, 0xc3, 0xe4, 0xf9,
    0xaa, 0xb7, 0x90, 0x8d, 0x36, 0x2b, 0x0c, 0x11, 0x42, 0x5f, 0x78, 0x65,
    0x94, 0x89, 0xae, 0xb3, 0xe0, 0xfd, 0xda, 0xc7, 0x7c, 0x61, 0x46, 0x5b,
    0x08, 0x15, 0x32, 0x2f, 0x59, 0x44, 0x63, 0x7e, 0x2d, 0x30, 0x17, 0x0a,
    0xb1, 0xac, 0x8b, 0x96, 0xc5, 0xd8, 0xff, 0xe2, 0x26, 0x3b, 0x1c, 0x01,
    0x52, 0x4f, 0x68, 0x75, 0xce, 0xd3, 0xf4, 0xe9, 0xba, 0xa7, 0x80, 0x9d,
    0xeb, 0xf6, 0xd1, 0xcc, 0x9f, 0x82, 0xa5, 0xb8, 0x03, 0x1e, 0x39, 0x24,
    0x77, 0x6a, 0x4d, 0x50, 0xa1, 0xbc, 0x9b, 0x86, 0xd5, 0xc8, 0xef, 0xf2,
    0x49, 0x54, 0x73, 0x6e, 0x3d, 0x20, 0x07, 0x1a, 0x6c, 0x71, 0x56, 0x4b,
    0x18, 0x05, 0x22, 0x3f, 0x84, 0x99, 0xbe, 0xa3, 0xf0, 0xed, 0xca, 0xd7,
    0x35, 0x28, 0x0f, 0x12, 0x41, 0x5c, 0x7b, 0x66, 0xdd, 0xc0, 0xe7, 0xfa,
    0xa9, 0xb4, 0x93, 0x8e, 0xf8, 0xe5, 0xc2, 0xdf, 0x8c, 0x91, 0xb6, 0xab,
    0x10, 0x0d, 0x2a, 0x37, 0x64, 0x79, 0x5e, 0x43, 0xb2, 0xaf, 0x88, 0x95,
    0xc6, 0xdb, 0xfc, 0xe1, 0x5a, 0x47, 0x60, 0x7d, 0x2e, 0x33, 0x14, 0x09,
    0x7f, 0x62, 0x45, 0x58, 0x0b, 0x16, 0x31, 0x2c, 0x97, 0x8a, 0xad, 0xb0,
    0xe3, 0xfe, 0xd9, 0xc4
};

static uint8_t
crc8(uint8_t crc, uint8_t const *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        uint8_t data = buf[i] ^ crc;

        crc = crc8_lut[data] ^ (crc << 8);
    }

    return crc ^ 0x70;
}

static void
read_ack(void)
{
    printf("reading ack\n");
    while(1) {
        twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK) {
            //printf("Message received\n");
        } else {
            printf("Failed to receive message\n");
            continue;
        }
        if (message.identifier < 0x700 && message.identifier >= 0x600) {
            printf("great!\n");
            printf("done reading ack\n");
            return;
        }
    }
}

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
    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
        printf("Message queued for transmission\n");
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
    xSemaphoreTake(ctrl_task_sem, portMAX_DELAY);

    tx_task_action_t tx_action;

    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }

    uint8_t counter = 0;

    send_light(0);
    counter++;
    //send_reset();

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
            drive = 1;
            tx_action = BACKLIGHT;
            xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
        }
        if (message.data_length_code == 4 && message.data[3] == 0xd5) {
            drive = 0;
            tx_action = RESET;
            xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
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

        if (action == BACKLIGHT) {
            send_light(0);
            /*
            if (drive) {
                counter++;
                if ((counter & 0xF) == 0xF)
                    counter++;
                drive = 0;
            }
            */
        }

        if (action == RESET) {
            send_reset();
        }
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

    xTaskCreatePinnedToCore(ctrl_task, "TWAI_ctrl", 4096, NULL, CTRL_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xSemaphoreGive(ctrl_task_sem);
    vTaskDelay(pdMS_TO_TICKS(100));
    xSemaphoreTake(ctrl_task_sem, portMAX_DELAY);   //Wait for completion
}
