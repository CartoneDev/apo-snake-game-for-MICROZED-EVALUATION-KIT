#ifndef MENU_H
#define MENU_H

#include "text.h"
#include "draw.h"
#include "knobs.h"
#include "sound.h"
#include "nano_clock.h"
#include "const.h"
#include "etc.h"

#define SELECTED_COLOR GREEN
#define UNSELECTED_COLOR WHITE

#define SKIRMISH 0
#define HUNT 1
#define EXIT 2

#define IN_GAME 0
#define CONTINUE 1
#define MAIN_MENU 2
#define EXIT_GAME 3

#define MODE_DMG(mode)          (mode == SKIRMISH ? SKIRMISH_DMG_PART : 1)
#define MODE_APPLES(mode)       (mode == SKIRMISH ? APPLES_COUNT : APPLES_COUNT * 2)
#define MODE_START_LENGTH(mode) (mode == SKIRMISH ? SKIRMISH_START_LENGTH : HUNT_START_LENGTH)

#define ENTRYS_COLOR(selected, cur) (selected == cur ? SELECTED_COLOR : UNSELECTED_COLOR)

// writes main menu in a loop until an entry is selected
#define MENU(mode, btn_prev)                                                                                            \
DO(                                                                                                                     \
    uint8_t selected = SKIRMISH;                                                                                        \
    do{                                                                                                                 \
        TEXT(WIDTH / 2, (HEIGHT / 60 * DEF_CHAR_HEIGHT), WIDTH / 80, HEIGHT / 60, "Skirmish", ENTRYS_COLOR(selected, SKIRMISH));  \
        TEXT(WIDTH / 2, 2 * (HEIGHT / 60 * DEF_CHAR_HEIGHT), WIDTH / 80, HEIGHT / 60, "Hunt", ENTRYS_COLOR(selected, HUNT));      \
        TEXT(WIDTH / 2, 3 * (HEIGHT / 60 * DEF_CHAR_HEIGHT), WIDTH / 80, HEIGHT / 60, "Exit", ENTRYS_COLOR(selected, EXIT));      \
        FLUSH;                                                                                                          \
        if(GREEN_KNOB_BTN(btn_prev)){                                                                                   \
            mode = selected;                                                                                            \
            SOUND(100000);                                                                                              \
            btn_prev = PREV_BTN;                                                                                        \
            break;                                                                                                      \
        }                                                                                                               \
        else if(RED_KNOB_BTN(btn_prev)){                                                                                \
            SOUND(500000);                                                                                              \
            selected = (selected + 2) % 3;                                                                              \
        }                                                                                                               \
        else if(BLUE_KNOB_BTN(btn_prev)){                                                                               \
            SOUND(500000);                                                                                              \
            selected = (selected + 1) % 3;                                                                              \
        }                                                                                                               \
        btn_prev = PREV_BTN;                                                                                            \
        NSEC_DELAY(NANO_PER_FRAME);                                                                                     \
        SOUND(0);                                                                                                       \
    }while(1);                                                                                                          \
)                                                                                                                       \

// writes pause menu (it doesn't have its own loop)
#define PAUSE(selected, btn_prev)                                                                                           \
DO(                                                                                                                         \
    if(selected != IN_GAME){                                                                                                \
        TEXT(11 * WIDTH / 20, (HEIGHT / 50 * DEF_CHAR_HEIGHT), WIDTH / 100, HEIGHT / 80, "Continue", ENTRYS_COLOR(selected, CONTINUE));      \
        TEXT(11 * WIDTH / 20, 2 * (HEIGHT / 50 * DEF_CHAR_HEIGHT), WIDTH / 100, HEIGHT / 80, "Main Menu", ENTRYS_COLOR(selected, MAIN_MENU));\
        if(GREEN_KNOB_BTN(btn_prev)){                                                                                       \
            SOUND(100000);                                                                                                  \
            if(selected == CONTINUE)                                                                                        \
                selected = IN_GAME;                                                                                         \
            else if(selected == MAIN_MENU)                                                                                  \
                selected = EXIT_GAME;                                                                                       \
        }                                                                                                                   \
        else if(RED_KNOB_BTN(btn_prev) || BLUE_KNOB_BTN(btn_prev)){                                                         \
            SOUND(500000);                                                                                                  \
            selected = selected % 2 + 1;                                                                                    \
        }                                                                                                                   \
    }                                                                                                                       \
    else if(GREEN_KNOB_BTN(btn_prev)){                                                                                      \
        SOUND(100000);                                                                                                      \
        selected = CONTINUE;                                                                                                \
    }                                                                                                                       \
    btn_prev = PREV_BTN;                                                                                                    \
)                                                                                                                           \

#endif // MENU_H