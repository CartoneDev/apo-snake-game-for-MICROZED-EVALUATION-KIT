#ifndef DRAW_H
#define DRAW_H

#include <time.h>

#include "mzapo_regs.h"
#include "const.h"
#include "text.h"
#include "etc.h"

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F

#define RED24   0xff0000
#define GREEN24 0x00ff00
#define BLUE24  0x0000ff

#define INIT_FRAMEBUFFER uint16_t *frame_buffer = (unsigned short *)malloc(HEIGHT * WIDTH * sizeof(unsigned short));

#define PIXEL(x, y) frame_buffer[x * WIDTH + y]

// write buffer to lcd and clear buffer
#define FLUSH                                                                                                   \
DO(                                                                                                             \
    for (int rewrite_i = 0; rewrite_i < HEIGHT * WIDTH; rewrite_i++){                                           \
        parlcd_write_data(lcd, frame_buffer[rewrite_i]);                                                        \
        frame_buffer[rewrite_i] = BLACK;                                                                        \
    }                                                                                                           \
)                                                                                                               \

// fill buffer with a color
#define CLEAR(color)                                                                                            \
    for (int clear_i = 0; clear_i < HEIGHT * WIDTH; clear_i++)                                                  \
        frame_buffer[clear_i] = color                                                                           \

#define RECTANGLE(x, y, w, h, color)                                                                            \
    for (int rec_i = MAX(0, y); rec_i < MIN(HEIGHT, y + h); rec_i++)                                            \
        for (int rec_j = MAX(0, x); rec_j < MIN(WIDTH, x + w); rec_j++)                                         \
        PIXEL(rec_i, rec_j) = color                                                                             \

// check each pixel in a rectangle to be in a circle
#define CIRCLE(x, y, r, color)                                                                                  \
    for (int cir_i = MAX(0, y - r); cir_i < MIN(FIELD_HEIGHT, y + r); cir_i++)                                  \
        for (int cir_j = MAX(0, x - r); cir_j < MIN(WIDTH, x + r); cir_j++)                                     \
        if (abs(cir_i - y) * abs(cir_i - y) + abs(cir_j - x) * abs(cir_j - x) < r * r)                          \
            PIXEL(cir_i, cir_j) = color                                                                         \

#define APPLE(apple) CIRCLE(apple.x, apple.y, CHAIN_RAD, GREEN);                                                \
                     CIRCLE(apple.x, apple.y, CHAIN_RAD / 2, BLACK)                                             \


#define SNAKE(snake, color)                                                                                     \
DO(                                                                                                             \
    uint16_t chains_color = color;                                                                              \
    if (snake.stunned % 2 != 0){                                                                                \
        chains_color = WHITE;                                                                                   \
    }                                                                                                           \
    for (int snake_i = 0; snake_i < snake.length; snake_i++)                                                    \
        CIRCLE(snake.tail[snake_i].x, snake.tail[snake_i].y, CHAIN_RAD, chains_color);                          \
)                                                                                                               \

// draws energy bar for each snake
#define SCORE RECTANGLE(0, FIELD_HEIGHT, WIDTH, SCORE_HEIGHT, WHITE);                                           \
              RECTANGLE(WIDTH / 5, FIELD_HEIGHT + SCORE_HEIGHT / 4, (WIDTH * snake1.length) / (MAX_ENERGY * 5), SCORE_HEIGHT / 2, RED);                                                                   \
              RECTANGLE(WIDTH / 5 + (WIDTH * snake1.length) / (MAX_ENERGY * 5), FIELD_HEIGHT + SCORE_HEIGHT / 4, (WIDTH / 5) - (WIDTH * snake1.length) / (MAX_ENERGY * 5), SCORE_HEIGHT / 2, BLACK);      \
              RECTANGLE(3 * WIDTH / 5, FIELD_HEIGHT + SCORE_HEIGHT / 4, (WIDTH * snake2.length) / (MAX_ENERGY * 5), SCORE_HEIGHT / 2, BLUE);                                                              \
              RECTANGLE(3 * WIDTH / 5 + (WIDTH * snake2.length) / (MAX_ENERGY * 5), FIELD_HEIGHT + SCORE_HEIGHT / 4, (WIDTH / 5) - (WIDTH * snake2.length) / (MAX_ENERGY * 5), SCORE_HEIGHT / 2, BLACK)   \

// draws rectangles for total score
#define TOTAL_SCORE RECTANGLE(WIDTH / 13, FIELD_HEIGHT / 8, 3 * WIDTH / 13, FIELD_HEIGHT / 2, WHITE);           \
                    RECTANGLE(9 * WIDTH / 13, FIELD_HEIGHT / 8, 3 * WIDTH / 13, FIELD_HEIGHT / 2, WHITE)        \

#define LOST_SCREEN(color1, color2, background, total_score)                                                    \
    CLEAR(background);                                                                                          \
    SNAKE(snake1, color1);                                                                                      \
    SNAKE(snake2, color2);                                                                                      \
    SCORE;                                                                                                      \
    TOTAL_SCORE;                                                                                                \
    char text[10];                                                                                              \
    sprintf(text, "%d", total_score.red);                                                                       \
    TEXT(21 * WIDTH / 104, 13 * FIELD_HEIGHT / 32, WIDTH / 40, FIELD_HEIGHT / 30, text, RED);                   \
    sprintf(text, "%d", total_score.blue);                                                                      \
    TEXT(85 * WIDTH / 104, 13 * FIELD_HEIGHT / 32, WIDTH / 40, FIELD_HEIGHT / 30, text, BLUE);                  \
    FLUSH;                                                                                                      \
  

#endif // DRAW_H