#include <shifter.h>

uint8_t message_counter = 0;

static void
send_state(uint8_t state)
{
    twai_message_t message;
    message.identifier = 0x3FD;
    message.extd = 0;
    message.data_length_code = 4;
    message.data[1] = message_counter;
    message.data[2] = state;
    message.data[3] = 0x00;
    message.data[4] = 0x00;
    message.data[0] = crc8(0, &message.data[1], 4);

    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) != ESP_OK) {
        printf("Failed to queue message for transmission\n");
    }
    message_counter++;
    if ((message_counter & 0xF) == 0xF)
        message_counter++;
}

void
shifter_send_park(void)
{
    send_state(0x20);
}

void
shifter_send_neutral(void)
{
    send_state(0x60);
}

void
shifter_send_reverse(void)
{
    send_state(0x40);
}

void
shifter_send_drive(bool moveable)
{
    if (moveable) {
        send_state(0x81);
    }
    else {
        send_state(0x80);
    }
}

void
shifter_send_reset(void)
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
    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) != ESP_OK) {
        printf("Failed to queue message for transmission\n");
    }

    return;
}

void
shifter_send_light(uint8_t counter, uint8_t brightness)
{
    uint8_t buf[] = { 0, 0x00, 0x00, 0x00 };
    buf[0] = brightness;
    //buf[1] = brightness;

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

    return;
}

