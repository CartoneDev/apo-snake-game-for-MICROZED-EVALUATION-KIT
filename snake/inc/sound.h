#ifndef SOUND_H
#define SOUND_H

#include "mzapo_regs.h"

#define INIT_SOUND  uint8_t  *audio = map_phys_address(AUDIOPWM_REG_BASE_PHYS, AUDIOPWM_REG_SIZE, 0);        \
                    (*(volatile uint32_t*)(audio + AUDIOPWM_REG_CR_o)) = 0x01;                               \
                    (*(volatile uint32_t*)(audio + AUDIOPWM_REG_PWM_o)) = 50000                              \

// sets the sound frequency
#define SOUND(freq) (*(volatile uint32_t*)(audio + AUDIOPWM_REG_PWMPER_o)) = freq

#endif // SOUND_H