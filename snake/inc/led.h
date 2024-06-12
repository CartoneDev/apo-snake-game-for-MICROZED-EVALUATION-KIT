#ifndef LED_H
#define LED_H

#include "mzapo_regs.h"

#define INIT_LED uint8_t *led = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);

#define LEFT_LED  (*(volatile uint32_t*)(led + SPILED_REG_LED_RGB1_o))
#define RIGHT_LED (*(volatile uint32_t*)(led + SPILED_REG_LED_RGB2_o))

#endif // LED_H