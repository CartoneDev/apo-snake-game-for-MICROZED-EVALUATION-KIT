/*******************************************************************
  Snake game for MicroZed based MZ_APO board
  designed by Lezhnev Evgenii and Jáchym Žák

  main.c - main file

  Lezhnev Evgenii and Jáchym Žák, MIT license, 2024

 *******************************************************************/

#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_types.h"

#include "led.h"
#include "knobs.h"
#include "lcd.h"
#include "sound.h"
#include "nano_clock.h"
#include "draw.h"
#include "snake.h"
#include "apple.h"
#include "const.h"
#include "text.h"
#include "menu.h"

int main(int argc, char *argv[]){
    INIT_LCD;
    INIT_LED;
    INIT_SOUND;
    INIT_FRAMEBUFFER;
    INIT_NANO_CLOCK;

    srand(time(NULL));

    do{ // main menu loop
        struct{
            uint16_t red, blue;
        } total_score = {0, 0};

        uint8_t prev_red_knob = RED_KNOB_ROT; // previous knob values
        uint8_t prev_blue_knob = BLUE_KNOB_ROT;
        bool btn_prev = PREV_BTN;

        uint8_t mode = 0, pause = 0;
        MENU(mode, btn_prev);
        SOUND(0);

        // if exit was selected
        if (mode == EXIT){
            CLEAR(BLACK);
            FLUSH;
            break;
        }

        do{  // choosed game mode loop
            SOUND(0); // stop sound if it was playing

            CLEAR(BLACK);

            snake_t snake1, snake2;
            SNAKE_INIT_POS(snake1);
            SNAKE_INIT_POS(snake2);
            // initial snake positions(by y axis)
            snake1.tail[0].y = FIELD_HEIGHT / 3;
            snake2.tail[0].y = 2 * FIELD_HEIGHT / 3;

            // sets the initial snake length
            for (int i = 1; i < MODE_START_LENGTH(mode); i++){
                ADD_CHAIN_LINK(snake1);
                ADD_CHAIN_LINK(snake2);
            }

            apple_t apple[APPLES_COUNT * 2]; // APPLES_COUNT * 2 is the maximum number of apples for both modes

            for (int i = 0; i < MODE_APPLES(mode); i++){
                SPAWN_APPLE(apple[i]);
            }

            do{  // frames loop (game loop)
                SOUND(0);
                clock_gettime(CLOCK_MONOTONIC, &clock); // get current time so we can wait to the next frame after all calculations
                clock.tv_nsec += NANO_PER_FRAME;

                PAUSE(pause, btn_prev); // pause menu logic(without own loop)
                if (pause == EXIT_GAME)
                    break;

                FLUSH;

                EXTRA_HP_LED(snake1, LEFT_LED, RED24, MODE_DMG(mode));
                EXTRA_HP_LED(snake2, RIGHT_LED, BLUE24, MODE_DMG(mode));

                if (!pause){
                    if (snake1.stunned == 0){
                        // if the knob was rotated more than a full turn, then ignore this alteration
                        // it's needed because knob has 0..255 values and we need at least to ignore 255 to 0 and vice versa alteration
                        bool red_alter = abs(RED_KNOB_ROT - prev_red_knob) > KNOB_FULL_TURN ? 0 : 1;
                        snake1.tail[0].angle += (RED_KNOB_ROT - prev_red_knob) / (float)KNOB_FULL_TURN * PI * red_alter;
                    }
                    if (snake2.stunned == 0){
                        bool blue_alter = abs(BLUE_KNOB_ROT - prev_blue_knob) > KNOB_FULL_TURN ? 0 : 1;
                        snake2.tail[0].angle += (BLUE_KNOB_ROT - prev_blue_knob) / (float)KNOB_FULL_TURN * PI * blue_alter;
                    }

                    prev_red_knob = RED_KNOB_ROT;
                    prev_blue_knob = BLUE_KNOB_ROT;

                    RECALC_SNAKE(snake1);
                    if ((RED_KNOB_BTN_PRESSED || snake1.stunned > 0) && snake1.length > 2){
                        // to achive double speed we recalculate snake position twice in a single frame
                        // snake moves twice faster when it's stunned too
                        // snake spend energy(length) to move faster
                        // snake loses one chain link every DOUBLE_SPEED_COST frames
                        RECALC_SNAKE(snake1);
                        if (snake1.energy_spended == 0){
                            snake1.length--;
                            snake1.energy_spended = DOUBLE_SPEED_COST;
                        }
                        else{
                            snake1.energy_spended--;
                        }
                    }
                    RECALC_SNAKE(snake2);
                    if ((BLUE_KNOB_BTN_PRESSED || snake2.stunned > 0) && snake2.length > 2){
                        RECALC_SNAKE(snake2);
                        if (snake2.energy_spended == 0){
                            snake2.length--;
                            snake2.energy_spended = DOUBLE_SPEED_COST;
                        }
                        else{
                            snake2.energy_spended--;
                        }
                    }

                    // check if snakes are colliding with themselves or with each other and if they are colliding with apples
                    bool s1_damaged = false, s2_damaged = false;
                    if (snake1.stunned > 0)
                        snake1.stunned--;
                    else{
                        IS_COLLIDING_WITH_ITSELF(snake1, s1_damaged);
                        IS_COLLIDING_WITH_OTHER(snake1, snake2, s1_damaged);
                    }
                    if (snake2.stunned > 0)
                        snake2.stunned--;
                    else{
                        IS_COLLIDING_WITH_ITSELF(snake2, s2_damaged);
                        IS_COLLIDING_WITH_OTHER(snake2, snake1, s2_damaged);
                    }

                    int16_t s1_apple = -1, s2_apple = -1;
                    for (int i = 0; i < MODE_APPLES(mode); i++){
                        IS_APPLE_COLLIDING(i, snake1, s1_apple);
                        IS_APPLE_COLLIDING(i, snake2, s2_apple);
                    }

                    // snake gets damage if it collides with itself or with another snake
                    // snake loses depending on the mode
                    bool s1_lost = false, s2_lost = false;
                    if (s1_damaged)
                        SNAKE_DAMAGED(snake1, s1_lost, MODE_DMG(mode));
                    if (s2_damaged)
                        SNAKE_DAMAGED(snake2, s2_lost, MODE_DMG(mode));

                    if (s1_apple != s2_apple){
                        SNAKE_ATE_APPLE(snake1, s1_apple, mode, s2_lost, LEFT_LED);
                        SNAKE_ATE_APPLE(snake2, s2_apple, mode, s1_lost, RIGHT_LED);
                    }

                    if (s1_lost || s2_lost){
                        if (s1_lost && !s2_lost){
                            total_score.blue++;
                            SNAKE_LOST(BLACK, WHITE, BLUE, total_score, btn_prev);
                        }
                        else if (s2_lost && !s1_lost){
                            total_score.red++;
                            SNAKE_LOST(WHITE, BLACK, RED, total_score, btn_prev);
                        }
                        else
                            SNAKE_LOST(BLACK, BLACK, WHITE, total_score, btn_prev);
                    }
                }

                // draw all the objects
                SNAKE(snake1, RED);
                SNAKE(snake2, BLUE);
                for (int i = 0; i < MODE_APPLES(mode); i++){
                    APPLE(apple[i]);
                }
                SCORE;

                // wait remaining time to the next frame
                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &clock, NULL);
            } while (true);
            if (pause == EXIT_GAME)
                break;
        } while (true);
    } while (true);
    return 0;
}