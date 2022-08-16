#ifndef SHIFTER_H
#define SHIFTER_H

#include <stdbool.h>
#include <driver/twai.h>
#include <bmw_crc.h>

static inline bool
SHIFTER_BACK_P(twai_message_t message)
{
    return message.identifier == 0x197 && message.data[2] == 0x3e;
}

static inline bool
SHIFTER_PARK_P(twai_message_t message)
{
    return message.identifier == 0x197 && message.data[3] == 0xd5;
}

static inline bool
SHIFTER_CENTER_P(twai_message_t message)
{
    return message.identifier == 0x197 && message.data[2] == 0x0e &&
      message.data[3] == 0xc0;
}

void shifter_send_park(void);
void shifter_send_drive(bool moveable);
void shifter_send_reset(void);
void shifter_send_light(uint8_t counter);

#endif
