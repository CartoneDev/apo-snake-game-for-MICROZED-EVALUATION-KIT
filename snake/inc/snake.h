#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

#include "draw.h"
#include "sound.h"
#include "nano_clock.h"
#include "menu.h"
#include "const.h"
#include "etc.h"

typedef struct {
    struct {
        int16_t x, y;
        float angle;
        // moving is true if the chain link is moving
        // spawned is true if the chain link just spawned to prevent collision with itself
        bool spawned, moving;
    } tail[MAX_ENERGY];
    uint8_t length, stunned, energy_spended;
} snake_t;

#define SNAKE_INIT_POS(snake)                                                                                   \
    snake.tail[0].x = WIDTH / 2;                                                                                \
    snake.tail[0].angle = 3 * PI / 2;                                                                           \
    snake.tail[0].moving = true;                                                                                \
    snake.tail[0].spawned = true;                                                                               \
    snake.length = 1;                                                                                           \
    snake.stunned = 0;                                                                                          \
    snake.energy_spended = 0                                                                                    \

// if chain exceeds the field, it appears on the opposite side
#define RECALC_CHAIN_LINK(chain)                                                                                \
    chain.x += sin(chain.angle) * chain.moving * SNAKE_SPEED;                                                   \
    chain.y += cos(chain.angle) * chain.moving * SNAKE_SPEED;                                                   \
    if (chain.x < 0) chain.x += WIDTH;                                                                          \
    else if (chain.x >= WIDTH) chain.x -= WIDTH;                                                                \
    if (chain.y < 0) chain.y += FIELD_HEIGHT;                                                                   \
    else if (chain.y >= FIELD_HEIGHT) chain.y -= FIELD_HEIGHT                                                   \

// recalculating position of each chain link depending on the angle and snake's speed
#define RECALC_SNAKE(snake)                                                                                     \
DO(                                                                                                             \
    RECALC_CHAIN_LINK(snake.tail[0]);                                                                           \
    for (int recalc_i = snake.length - 1; recalc_i > 0; recalc_i--){                                            \
        RECALC_CHAIN_LINK(snake.tail[recalc_i]);                                                                \
        snake.tail[recalc_i].angle = snake.tail[recalc_i - 1].angle;                                            \
        snake.tail[recalc_i].moving = snake.tail[recalc_i - 1].moving;                                          \
        snake.tail[recalc_i].spawned = snake.tail[recalc_i - 1].spawned;                                        \
    }                                                                                                           \
)                                                                                                               \

#define ADD_CHAIN_LINK(snake)                                                                                   \
    snake.tail[snake.length].x = snake.tail[snake.length - 1].x;                                                \
    snake.tail[snake.length].y = snake.tail[snake.length - 1].y;                                                \
    snake.tail[snake.length].angle = 0;                                                                         \
    snake.tail[snake.length].moving = false;                                                                    \
    snake.tail[snake.length].spawned = false;                                                                   \
    snake.length++                                                                                              \

#define IS_COLLIDING_WITH_ITSELF(snake, some_bool)                                                              \
DO(                                                                                                             \
    for (int collides_i = 10; collides_i < snake.length; collides_i++){                                         \
        if (!snake.tail[collides_i].spawned)                                                                    \
            break;                                                                                              \
        if (pow(snake.tail[0].x - snake.tail[collides_i].x, 2) + pow(snake.tail[0].y - snake.tail[collides_i].y, 2) < CHAIN_RAD * CHAIN_RAD * 4){\
            some_bool = true;                                                                                   \
            break;                                                                                              \
        }                                                                                                       \
    }                                                                                                           \
)                                                                                                               \

#define IS_COLLIDING_WITH_OTHER(snake, other, some_bool)                                                        \
DO(                                                                                                             \
    for (int collides_i = 0; collides_i < other.length; collides_i++)                                           \
        if (pow(snake.tail[0].x - other.tail[collides_i].x, 2) + pow(snake.tail[0].y - other.tail[collides_i].y, 2) < CHAIN_RAD * CHAIN_RAD * 4){\
            some_bool = true;                                                                                   \
            break;                                                                                              \
        }                                                                                                       \
)                                                                                                               \

// if snake ate an apple, it grows and wins if win score is reached in hunt mode
#define SNAKE_ATE_APPLE(snake, apple, mode, win, its_led)                                                       \
DO(                                                                                                             \
    uint16_t adding = APPLE_ADDS;                                                                               \
    if(apple != -1){                                                                                            \
        its_led = GREEN24;                                                                                      \
        if (snake.length + APPLE_ADDS > MAX_ENERGY){                                                            \
            if(mode == HUNT){                                                                                   \
                win = true;                                                                                     \
                break;                                                                                          \
            }                                                                                                   \
            adding = MAX_ENERGY - snake.length;                                                                 \
        }                                                                                                       \
        for(int count_i = 0; count_i < adding; count_i++){                                                      \
            ADD_CHAIN_LINK(snake);                                                                              \
        }                                                                                                       \
    }                                                                                                           \
)                                                                                                               \

// if snake can survive the damage turn its led to the color
#define EXTRA_HP_LED(snake, its_led, color, part)                                                               \
DO(                                                                                                             \
    if(snake.length > MAX_ENERGY / part){                                                                       \
        its_led = color;                                                                                        \
    }                                                                                                           \
    else{                                                                                                       \
        its_led = BLACK;                                                                                        \
    }                                                                                                           \
)                                                                                                               \

#define SNAKE_DAMAGED(snake, lost, part)                                                                        \
DO(                                                                                                             \
    if (snake.length > MAX_ENERGY / part){                                                                      \
        snake.length -= MAX_ENERGY / part;                                                                      \
        if (snake.length <= 1)                                                                                  \
            lost = true;                                                                                        \
        else{                                                                                                   \
            snake.tail[0].angle += PI;                                                                          \
            snake.stunned = snake.length / 2;                                                                   \
            RECALC_SNAKE(snake);                                                                                \
            SOUND(100000);                                                                                      \
        }                                                                                                       \
    }                                                                                                           \
    else                                                                                                        \
        lost = true;                                                                                            \
)                                                                                                               \

#define SNAKE_LOST(color1, color2, color3, total_score, btn_prev)                                               \
    LOST_SCREEN(color1, color2, color3, total_score);                                                           \
    SOUND(500000);                                                                                              \
    NSEC_DELAY(NANO_PER_FRAME);                                                                                 \
    NSEC_DELAY(NANO_PER_FRAME);                                                                                 \
    SOUND(0);                                                                                                   \
    while(!GREEN_KNOB_BTN(btn_prev)){                                                                           \
        btn_prev = PREV_BTN;                                                                                    \
        NSEC_DELAY(NANO_PER_FRAME);                                                                             \
    }                                                                                                           \
    btn_prev = PREV_BTN;                                                                                        \
    break                                                                                                       \

#endif // SNAKE_H