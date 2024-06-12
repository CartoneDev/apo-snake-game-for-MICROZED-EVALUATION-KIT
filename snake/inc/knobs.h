#ifndef KNOBS_H
#define KNOBS_H

#include "mzapo_regs.h"

#define BLUE_KNOB_ROT  (uint8_t)( *(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o))
#define GREEN_KNOB_ROT (uint8_t)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 8) & 0xff)
#define RED_KNOB_ROT   (uint8_t)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 16) & 0xff)

// if a knob button was pressed
#define BLUE_KNOB_BTN_PRESSED  (bool)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 24) & 0x01)
#define GREEN_KNOB_BTN_PRESSED (bool)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 25) & 0x01)
#define RED_KNOB_BTN_PRESSED   (bool)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 26) & 0x01)

// if any knob button was pressed
#define PREV_BTN (BLUE_KNOB_BTN_PRESSED || GREEN_KNOB_BTN_PRESSED || RED_KNOB_BTN_PRESSED)

// if a knob button was pressed and any of the knob buttons was not pressed before
#define BLUE_KNOB_BTN(prev)  (!prev && BLUE_KNOB_BTN_PRESSED)
#define GREEN_KNOB_BTN(prev) (!prev && GREEN_KNOB_BTN_PRESSED)
#define RED_KNOB_BTN(prev)   (!prev && RED_KNOB_BTN_PRESSED)

#endif // KNOBS_H