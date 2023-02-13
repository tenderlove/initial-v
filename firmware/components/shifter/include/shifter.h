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

#define PARK_BUTTON_BIT (1 << 4)
#define SHIFTER_POSITION_MASK (PARK_BUTTON_BIT - 1)
typedef enum {
    SHIFTER_CENTER,
    SHIFTER_UP,
    SHIFTER_UP_UP,
    SHIFTER_DOWN,
    SHIFTER_DOWN_DOWN,
    SHIFTER_SIDE_UP,
    SHIFTER_SIDE_DOWN,
    SHIFTER_SIDE,

    SHIFTER_CENTER_PARK = 16,
    SHIFTER_UP_PARK,
    SHIFTER_UP_UP_PARK,
    SHIFTER_DOWN_PARK,
    SHIFTER_DOWN_DOWN_PARK,
    SHIFTER_SIDE_UP_PARK,
    SHIFTER_SIDE_DOWN_PARK,
    SHIFTER_SIDE_PARK,

    SHIFTER_MAX,
} handle_position_t;

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

static inline bool
SHIFTER_POSITION(twai_message_t message, handle_position_t * pos)
{
    if (message.identifier != 0x197) {
        return false;
    }

    uint8_t position = (message.data[2] >> 4) & 0xF;

    // Resetting causes the position to go to 9
    if (position > 8) {
      position = SHIFTER_CENTER;
    }

    if (SHIFTER_PARK_P(message)) {
        position |= PARK_BUTTON_BIT;
    }

    *pos = (handle_position_t)position;

    return true;
}


void shifter_send_park(void);
void shifter_send_neutral(void);
void shifter_send_reverse(void);
void shifter_send_drive(bool moveable);
void shifter_send_reset(void);
void shifter_send_light(uint8_t counter, uint8_t brightness);

#ifdef __cplusplus
}
#endif

#endif
