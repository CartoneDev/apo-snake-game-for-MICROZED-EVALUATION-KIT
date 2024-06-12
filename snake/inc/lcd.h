#ifndef LCD_H
#define LCD_H

#include "mzapo_regs.h"

#define INIT_LCD uint8_t  *lcd = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0); \
                 parlcd_hx8357_init(lcd);                                                    \
                 parlcd_write_cmd(lcd, 0x2c)                                                 \

#endif // LCD_H