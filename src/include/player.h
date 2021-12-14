#ifndef PLAYER_H__
#define PLAYER_H__

#include "map.h"

struct player
{
    int pos_x;
    int pos_y;
    int width;
    int height;

    int direction;
    int is_moving;
    float speed;

    int move_steps;
    int move_counter;
    int is_grabbing;
};

void init_player(struct player *p, int x, int y, int direction);
void move_player(struct player *p, struct map *m);
int check_collision(int *object_x, int *object_y, int speed, struct map *m, int direction, int curr_box_index, int is_grabbing);
int squares_intersecting(int x1, int y1, int size1, int x2, int y2, int size2);

#endif