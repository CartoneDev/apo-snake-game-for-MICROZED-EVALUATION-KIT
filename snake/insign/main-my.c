/*******************************************************************
  Snake game for MicroZed based MZ_APO board
  designed by Zeka

  main.c      - main file

  include your name there and license for distribution.

 *******************************************************************/

#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#define NANO_PER_FRAME 33333333
#define WIDTH 480
#define HEIGHT 320
#define SCORE_HEIGHT 30
#define FIELD_HEIGHT (HEIGHT - SCORE_HEIGHT)
#define PI 3.14159265358979323846
#define KNOB_FULL_TURN 96
#define APPLES_COUNT 7
#define APPLE_ADDS 4
#define MAX_ENERGY 100
#define START_LENGTH 100
#define CHAIN_RAD 5
#define SNAKE_SPEED 3
#define DOUBLE_SPEED_COST 3

#define DO(...) do{__VA_ARGS__}while(0)

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define SOUND(freq) (*(volatile uint32_t*)(audio + AUDIOPWM_REG_PWMPER_o)) = freq;

#define PIXEL(x, y) frame_buffer[x * WIDTH + y]

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F

#define RED24   0xff0000
#define BLUE24  0x0000ff

#define FLUSH                                                                                               \
DO(                                                                                                         \
  for (int rewrite_i = 0; rewrite_i < HEIGHT * WIDTH; rewrite_i++){                                         \
    parlcd_write_data(lcd, frame_buffer[rewrite_i]);                                                        \
    frame_buffer[rewrite_i] = BLACK;                                                                        \
  }                                                                                                         \
)                                                                                                           \

#define CLEAR(color)                                                                                        \
for (int clear_i = 0; clear_i < HEIGHT * WIDTH; clear_i++)                                                  \
  frame_buffer[clear_i] = color                                                                             \

#define RECTANGLE(x, y, w, h, color)                                                                        \
  for (int rec_i = MAX(0, y); rec_i < MIN(HEIGHT, y + h); rec_i++)                                          \
    for (int rec_j = MAX(0, x); rec_j < MIN(WIDTH, x + w); rec_j++)                                         \
      PIXEL(rec_i, rec_j) = color                                                                           \

#define CIRCLE(x, y, r, color)                                                                              \
  for (int cir_i = MAX(0, y - r); cir_i < MIN(FIELD_HEIGHT, y + r); cir_i++)                                \
    for (int cir_j = MAX(0, x - r); cir_j < MIN(WIDTH, x + r); cir_j++)                                     \
      if (abs(cir_i - y) * abs(cir_i - y) + abs(cir_j - x) * abs(cir_j - x) < r * r)                        \
        PIXEL(cir_i, cir_j) = color                                                                         \

#define APPLE(apple) CIRCLE(apple.x, apple.y, CHAIN_RAD, GREEN);                                            \
                     CIRCLE(apple.x, apple.y, CHAIN_RAD / 2, BLACK)                                         \

#define SPAWN_APPLE(apple)                                                                                  \
  apple.x = (rand() % (WIDTH / CHAIN_RAD)) * CHAIN_RAD;                                                     \
  apple.y = (rand() % (FIELD_HEIGHT / CHAIN_RAD)) * CHAIN_RAD                                               \
 
#define IS_APPLE_COLLIDING(apple_id, snake, collides_with)                                                  \
DO(                                                                                                         \
  if (pow(snake.tail[0].x - apple[apple_id].x, 2) + pow(snake.tail[0].y - apple[apple_id].y, 2) < CHAIN_RAD * CHAIN_RAD * 4){\
      collides_with = apple_id;                                                                             \
      SOUND(450000);                                                                                        \
      SPAWN_APPLE(apple[apple_id]);                                                                         \
  }                                                                                                         \
)                                                                                                           \

struct {
  struct {
    int16_t x, y;
    float angle;
    bool spawned, moving;
  } tail[MAX_ENERGY];
  uint8_t length, stunned, energy_spended;
} snake1, snake2;

#define SNAKE_INIT_POS(snake)                                                                               \
  snake.tail[0].x = WIDTH / 2;                                                                              \
  snake.tail[0].angle = 3 * PI / 2;                                                                         \
  snake.tail[0].moving = true;                                                                              \
  snake.tail[0].spawned = true;                                                                             \
  snake.length = 1;                                                                                         \
  snake.stunned = 0;                                                                                        \
  snake.energy_spended = 0                                                                                  \

#define SNAKE(snake, color)                                                                                 \
  uint16_t chains_color = color;                                                                            \
  if (snake.stunned % 2 != 0){                                                                              \
    chains_color = WHITE;                                                                                   \
  }                                                                                                         \
  for (int snake_i = 0; snake_i < snake.length; snake_i++)                                                  \
    CIRCLE(snake.tail[snake_i].x, snake.tail[snake_i].y, CHAIN_RAD, chains_color)                           \

#define RECALC_SPEED(chain)                                                                                 \
  chain.x += sin(chain.angle) * chain.moving * SNAKE_SPEED;                                                 \
  chain.y += cos(chain.angle) * chain.moving * SNAKE_SPEED;                                                 \
  if (chain.x < 0) chain.x += WIDTH;                                                                        \
  else if (chain.x >= WIDTH) chain.x -= WIDTH;                                                              \
  if (chain.y < 0) chain.y += FIELD_HEIGHT;                                                                 \
  else if (chain.y >= FIELD_HEIGHT) chain.y -= FIELD_HEIGHT                                                 \

#define RECALC_SNAKE(snake)                                                                                 \
DO(                                                                                                         \
  RECALC_SPEED(snake.tail[0]);                                                                              \
  for (int recalc_i = snake.length - 1; recalc_i > 0; recalc_i--){                                          \
    RECALC_SPEED(snake.tail[recalc_i]);                                                                     \
    snake.tail[recalc_i].angle = snake.tail[recalc_i - 1].angle;                                            \
    snake.tail[recalc_i].moving = snake.tail[recalc_i - 1].moving;                                          \
    snake.tail[recalc_i].spawned = snake.tail[recalc_i - 1].spawned;                                        \
  }                                                                                                         \
)                                                                                                           \

#define ADD_CHAIN(snake)                                                                                    \
  snake.tail[snake.length].x = snake.tail[snake.length - 1].x;                                              \
  snake.tail[snake.length].y = snake.tail[snake.length - 1].y;                                              \
  snake.tail[snake.length].angle = 0;                                                                       \
  snake.tail[snake.length].moving = false;                                                                  \
  snake.tail[snake.length].spawned = false;                                                                 \
  snake.length++                                                                                            \

#define IS_COLLIDES_WITH_ITSELF(snake, some_bool)                                                           \
DO(                                                                                                         \
  for (int collides_i = 10; collides_i < snake.length; collides_i++){                                       \
    if (!snake.tail[collides_i].spawned)                                                                    \
      break;                                                                                                \
    if (pow(snake.tail[0].x - snake.tail[collides_i].x, 2) + pow(snake.tail[0].y - snake.tail[collides_i].y, 2) < CHAIN_RAD * CHAIN_RAD * 4){\
      some_bool = true;                                                                                     \
      break;                                                                                                \
    }                                                                                                       \
  }                                                                                                         \
)                                                                                                           \

#define IS_COLLIDES_WITH_OTHER(snake, other, some_bool)                                                     \
DO(                                                                                                         \
  for (int collides_i = 0; collides_i < other.length; collides_i++)                                         \
    if (pow(snake.tail[0].x - other.tail[collides_i].x, 2) + pow(snake.tail[0].y - other.tail[collides_i].y, 2) < CHAIN_RAD * CHAIN_RAD * 4){\
      some_bool = true;                                                                                     \
      break;                                                                                                \
    }                                                                                                       \
)                                                                                                           \

#define SCORE RECTANGLE(0, FIELD_HEIGHT, WIDTH, SCORE_HEIGHT, WHITE);                                       \
              RECTANGLE(WIDTH / 5, FIELD_HEIGHT + SCORE_HEIGHT / 4, (WIDTH * snake1.length) / (MAX_ENERGY * 5), SCORE_HEIGHT / 2, RED);                                                                   \
              RECTANGLE(WIDTH / 5 + (WIDTH * snake1.length) / (MAX_ENERGY * 5), FIELD_HEIGHT + SCORE_HEIGHT / 4, (WIDTH / 5) - (WIDTH * snake1.length) / (MAX_ENERGY * 5), SCORE_HEIGHT / 2, BLACK);      \
              RECTANGLE(3 * WIDTH / 5, FIELD_HEIGHT + SCORE_HEIGHT / 4, (WIDTH * snake2.length) / (MAX_ENERGY * 5), SCORE_HEIGHT / 2, BLUE);                                                              \
              RECTANGLE(3 * WIDTH / 5 + (WIDTH * snake2.length) / (MAX_ENERGY * 5), FIELD_HEIGHT + SCORE_HEIGHT / 4, (WIDTH / 5) - (WIDTH * snake2.length) / (MAX_ENERGY * 5), SCORE_HEIGHT / 2, BLACK)   \

#define TOTAL_SCORE RECTANGLE(WIDTH / 7, FIELD_HEIGHT / 5, 2 * WIDTH / 7, 3 * FIELD_HEIGHT / 5, WHITE);     \
                    RECTANGLE(4 * WIDTH / 7, FIELD_HEIGHT / 5, 2 * WIDTH / 7, 3 * FIELD_HEIGHT / 5, WHITE)  \

#define NSEC_DELAY(time)                                                                                    \
  clock_gettime(CLOCK_MONOTONIC, &clock);                                                                   \
  clock.tv_nsec += time;                                                                                    \
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &clock, NULL)                                             \

#define SEC_DELAY(time)                                                                                     \
  clock_gettime(CLOCK_MONOTONIC, &clock);                                                                   \
  clock.tv_sec += time;                                                                                     \
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &clock, NULL)                                             \


//TODO: total_score.snake2 print
//TODO: total_score.snake1 print
#define SNAKE_LOST(color1, color2, background, total_score)                                                 \
  CLEAR(background);                                                                                        \
  SNAKE(snake1, color1);                                                                                    \
  SNAKE(snake2, color2);                                                                                    \
  SCORE;                                                                                                    \
  FLUSH;                                                                                                    \
  SOUND(500000);                                                                                            \
  NSEC_DELAY(NANO_PER_FRAME);                                                                               \
  NSEC_DELAY(NANO_PER_FRAME);                                                                               \
  SOUND(0);                                                                                                 \
  break                                                                                                     \

#define SNAKE_DAMAGED(snake, lost)                                                                          \
DO(                                                                                                         \
  if (snake.length > MAX_ENERGY / 3){                                                                       \
    snake.length -= MAX_ENERGY / 3;                                                                         \
    if (snake.length <= 1){                                                                                 \
      lost = true;                                                                                          \
    }                                                                                                       \
    else{                                                                                                   \
      snake.tail[0].angle += PI;                                                                            \
      snake.stunned = snake.length;                                                                         \
      RECALC_SNAKE(snake);                                                                                  \
      SOUND(100000);                                                                                        \
    }                                                                                                       \
  }                                                                                                         \
  else{                                                                                                     \
    lost = true;                                                                                            \
  }                                                                                                         \
)                                                                                                           \

#define SNAKE_ATE_APPLE(snake, lost, apple, c1, c2, bg, total)                                              \
DO(                                                                                                         \
  uint16_t adding = 0;                                                                                      \
  if(apple != -1){                                                                                          \
    if (snake.length + APPLE_ADDS > MAX_ENERGY){                                                            \
      adding = MAX_ENERGY - snake.length;                                                                   \
    }                                                                                                       \
    else{                                                                                                   \
      adding = APPLE_ADDS;                                                                                  \
    }                                                                                                       \
    for(int count_i = 0; count_i < adding; count_i++){                                                      \
        ADD_CHAIN(snake);                                                                                   \
    }                                                                                                       \
  }                                                                                                         \
)                                                                                                           \

#define EXTRA_HP_LED(snake, its_led, color)                                                                 \
DO(                                                                                                         \
  if(snake.length > MAX_ENERGY / 3){                                                                        \
    its_led = color;                                                                                        \
  }                                                                                                         \
  else{                                                                                                     \
    its_led = BLACK;                                                                                        \
  }                                                                                                         \
)                                                                                                           \

#define BLUE_KNOB_ROT  (uint8_t)( *(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o))
#define GREEN_KNOB_ROT (uint8_t)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 8) & 0xff)
#define RED_KNOB_ROT   (uint8_t)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 16) & 0xff)

#define BLUE_KNOB_BTN  (bool)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 24) & 0x01)
#define GREEN_KNOB_BTN (bool)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 25) & 0x01)
#define RED_KNOB_BTN   (bool)((*(volatile uint32_t*)(led + SPILED_REG_KNOBS_8BIT_o) >> 26) & 0x01)

#define LEFT_LED  (*(volatile uint32_t*)(led + SPILED_REG_LED_RGB1_o))
#define RIGHT_LED (*(volatile uint32_t*)(led + SPILED_REG_LED_RGB2_o))

int main(int argc, char *argv[])
{
  uint8_t  *lcd          = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  uint8_t  *led          = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  uint8_t  *audio        = map_phys_address(AUDIOPWM_REG_BASE_PHYS, AUDIOPWM_REG_SIZE, 0);
  uint16_t *frame_buffer = (unsigned short *)malloc(HEIGHT * WIDTH * sizeof(unsigned short));

  (*(volatile uint32_t*)(audio + AUDIOPWM_REG_CR_o)) = 0x01;
  (*(volatile uint32_t*)(audio + AUDIOPWM_REG_PWM_o)) = 50000;

  parlcd_hx8357_init(lcd);
  parlcd_write_cmd(lcd, 0x2c);
  srand(time(NULL));

  struct {
      uint16_t red, blue;
  } total_score = {0, 0};

  do{

    SOUND(0);
    bool pause = false;

    CLEAR(BLACK);

    SNAKE_INIT_POS(snake1);
    SNAKE_INIT_POS(snake2);
    snake1.tail[0].y = FIELD_HEIGHT / 3;
    snake2.tail[0].y = 2 * FIELD_HEIGHT / 3;

    for(int i = 1; i < START_LENGTH; i++){
      ADD_CHAIN(snake1);
      ADD_CHAIN(snake2);
    }

    SNAKE(snake1, RED);
    SNAKE(snake2, BLUE);

    struct timespec clock;

    uint8_t prev_red_knob = RED_KNOB_ROT;
    uint8_t prev_blue_knob = BLUE_KNOB_ROT;

    struct {
      int16_t x, y;
    } apple[APPLES_COUNT];
    
    for (int i = 0; i < APPLES_COUNT; i++){
      SPAWN_APPLE(apple[i]);
    }

    do{
      SOUND(0);
      if (GREEN_KNOB_BTN){
        SEC_DELAY(1);
        pause = !pause;
      }
      clock_gettime(CLOCK_MONOTONIC, &clock);
      clock.tv_nsec += NANO_PER_FRAME;

      FLUSH;

      EXTRA_HP_LED(snake1, LEFT_LED, RED24);
      EXTRA_HP_LED(snake2, RIGHT_LED, BLUE24);

      if(!pause){
        if(snake1.stunned == 0){
          bool red_alter = abs(RED_KNOB_ROT - prev_red_knob) > KNOB_FULL_TURN ? 0 : 1;
          snake1.tail[0].angle += (RED_KNOB_ROT - prev_red_knob) / (float)KNOB_FULL_TURN * PI * red_alter;
        }
        if(snake2.stunned == 0){
          bool blue_alter = abs(BLUE_KNOB_ROT - prev_blue_knob) > KNOB_FULL_TURN ? 0 : 1;
          snake2.tail[0].angle += (BLUE_KNOB_ROT - prev_blue_knob) / (float)KNOB_FULL_TURN * PI * blue_alter;
        }

        prev_red_knob = RED_KNOB_ROT;
        prev_blue_knob = BLUE_KNOB_ROT;

        RECALC_SNAKE(snake1);
        if ((RED_KNOB_BTN || snake1.stunned > 0) && snake1.length > 2){
          RECALC_SNAKE(snake1);
          if(snake1.energy_spended == 0){
            snake1.length--;
            snake1.energy_spended = DOUBLE_SPEED_COST;
          }
          else{
            snake1.energy_spended--;
          }
        }
        RECALC_SNAKE(snake2);
        if ((BLUE_KNOB_BTN || snake2.stunned > 0) && snake2.length > 2){
          RECALC_SNAKE(snake2);
          if(snake2.energy_spended == 0){
            snake2.length--;
            snake2.energy_spended = DOUBLE_SPEED_COST;
          }
          else{
            snake2.energy_spended--;
          }
        }

        bool s1_damaged = false, s2_damaged = false;
        if(snake1.stunned > 0){
          snake1.stunned--;
        }
        else{
          IS_COLLIDES_WITH_ITSELF(snake1, s1_damaged);
          IS_COLLIDES_WITH_OTHER(snake1, snake2, s1_damaged);
        }
        if(snake2.stunned > 0){
          snake2.stunned--;
        }
        else{
          IS_COLLIDES_WITH_ITSELF(snake2, s2_damaged);
          IS_COLLIDES_WITH_OTHER(snake2, snake1, s2_damaged);
        }

        
        bool s1_lost = false, s2_lost = false;
        if(s1_damaged){
          SNAKE_DAMAGED(snake1, s1_lost);
        }
        if(s2_damaged){
          SNAKE_DAMAGED(snake2, s2_lost);
        }
        
        if(s1_lost || s2_lost){
          if(s1_lost && !s2_lost){
            total_score.blue++;
            SNAKE_LOST(BLACK, WHITE, BLUE, total_score)
          }
          else if(s2_lost && !s1_lost){
            total_score.red++;
            SNAKE_LOST(WHITE, BLACK, RED, total_score)
          }
          else{
            SNAKE_LOST(BLACK, BLACK, WHITE, total_score);
          }
        }

        int16_t s1_apple = -1, s2_apple = -1;

        for(int i = 0; i < APPLES_COUNT; i++){
          IS_APPLE_COLLDING(i, snake1, s1_apple);
          IS_APPLE_COLLIDING(i, snake2, s2_apple);
        }

        if(s1_apple != s2_apple){
          SNAKE_ATE_APPLE(snake1, snake2, s1_apple, WHITE, BLACK, RED, total_loses);
          SNAKE_ATE_APPLE(snake2, snake1, s2_apple, BLACK, WHITE, BLUE, total_loses);
        }
      }

      SNAKE(snake1, RED);
      SNAKE(snake2, BLUE);
      for(int i = 0; i < APPLES_COUNT; i++){
        APPLE(apple[i]);
      }
      SCORE;
      
      clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &clock, NULL);
    }while(true);
    
    while(!GREEN_KNOB_BTN){
      NSEC_DELAY(NANO_PER_FRAME);
    }
    SEC_DELAY(1);
  }while(true);
  return 0;
}