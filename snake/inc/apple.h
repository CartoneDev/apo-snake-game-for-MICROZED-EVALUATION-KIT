#ifndef APPLE_H
#define APPLE_H

typedef struct {
  int16_t x, y;
} apple_t;

// sets the apple position to a random position
#define SPAWN_APPLE(apple)                                                                                      \
    apple.x = (rand() % (WIDTH / CHAIN_RAD)) * CHAIN_RAD;                                                       \
    apple.y = (rand() % (FIELD_HEIGHT / CHAIN_RAD)) * CHAIN_RAD                                                 \
 
#define IS_APPLE_COLLIDING(apple_id, snake, collides_with)                                                      \
DO(                                                                                                             \
    if (pow(snake.tail[0].x - apple[apple_id].x, 2) + pow(snake.tail[0].y - apple[apple_id].y, 2) < CHAIN_RAD * CHAIN_RAD * 4){\
        collides_with = apple_id;                                                                               \
        SOUND(450000);                                                                                          \
        SPAWN_APPLE(apple[apple_id]);                                                                           \
    }                                                                                                           \
)                                                                                                               \

#endif // APPLE_H