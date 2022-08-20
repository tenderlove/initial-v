#ifndef SHIFTER_H
#define SHIFTER_H

#include <stdbool.h>
#include <driver/twai.h>
#include <bmw_crc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POSITION_PREDICATE(name, byte) \
static inline bool SHIFTER_##name##_P(twai_message_t message) \
{ \
    return message.identifier == 0x197 && message.data[2] == byte; \
}

POSITION_PREDICATE(CENTER, 0x0e);
POSITION_PREDICATE(UP, 0x1e);
POSITION_PREDICATE(UP_UP, 0x2e);
POSITION_PREDICATE(DOWN, 0x3e);
POSITION_PREDICATE(DOWN_DOWN, 0x4e);
POSITION_PREDICATE(SIDE_UP, 0x5e);
POSITION_PREDICATE(SIDE_DOWN, 0x6e);
POSITION_PREDICATE(SIDE, 0x7e);

static inline bool
SHIFTER_PARK_P(twai_message_t message)
{
    return message.identifier == 0x197 && message.data[3] == 0xd5;
}

void shifter_send_park(void);
void shifter_send_drive(bool moveable);
void shifter_send_reset(void);
void shifter_send_light(uint8_t counter);

#ifdef __cplusplus
}
#endif

#endif
