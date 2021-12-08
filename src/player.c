#include "include/player.h"
#include "include/directions.h"
#include "include/texture.h"

#include <stdio.h>

void init_player(struct player *p, int x, int y, int direction)
{
    p->pos_x = x;
    p->pos_y = y;
    p->direction = direction;

    p->speed = 10;
    p->is_moving = 0;
    p->acceleration = 1; // not in use

    p->move_counter = 0;
    p->is_grabbing = 0;
}

int squares_intersecting(int x1, int y1, int size1, int x2, int y2, int size2)
{
    int collision_lr = ((x1 < x2 + size2 && x1 > x2) || (x1 + size1 < x2 + size2 && x1 + size1 > x2));
    int collision_tb = ((y1 > y2 && y1 < y2 + size2) || (y1 + size1 > y2 && y1 + size1 < y2 + size2));

    if ((collision_lr && y1 == y2) || (collision_tb && x1 == x2))
    {
        return 1;
    }
    return 0;
}

int check_collision(int *object_x, int *object_y, int speed, struct map *m, int direction, int curr_box_index, int is_grabbing)
{
    int x = *object_x;
    int y = *object_y;

    int grid_x = x / m->grid_size;
    int grid_y = y / m->grid_size;

    // Wall collision
    if (x % m->grid_size == 0 && y % m->grid_size == 0)
    {
        if (direction == LEFT)
        {
            if (m->ground[grid_y][grid_x - 1].texture == WALL || m->ground[grid_y][grid_x - 1].texture == DOOR_CLOSED)
            {
                return 1;
            }
        }
        else if (direction == RIGHT)
        {
            if (m->ground[grid_y][grid_x + 1].texture == WALL || m->ground[grid_y][grid_x + 1].texture == DOOR_CLOSED)
            {
                return 1;
            }
        }
        else if (direction == UP)
        {
            if (m->ground[grid_y - 1][grid_x].texture == WALL || m->ground[grid_y - 1][grid_x].texture == DOOR_CLOSED)
            {
                return 1;
            }
        }
        else if (direction == DOWN)
        {
            if (m->ground[grid_y + 1][grid_x].texture == WALL || m->ground[grid_y + 1][grid_x].texture == DOOR_CLOSED)
            {
                return 1;
            }
        }
    }

    // Box collision

    /*
    Iterate through all boxes check if one is at the next position
    Call this function recursivly
    */
    int pulled_box_index = -1;

    for (int i = 0; i < m->num_boxes; i++)
    {
        if (i == curr_box_index)
        {
            continue;
        }

        int next_x = x;
        int next_y = y;

        int back_x = x;
        int back_y = y;

        // This can be moved out from the for-loop
        if (direction == LEFT)
        {
            next_x -= speed;
            back_x += speed;
        }
        else if (direction == RIGHT)
        {
            next_x += speed;
            back_x -= speed;
        }
        else if (direction == UP)
        {
            next_y -= speed;
            back_y += speed;
        }
        else if (direction == DOWN)
        {
            next_y += speed;
            back_y -= speed;
        }

        if (squares_intersecting(next_x, next_y, m->grid_size, m->boxes[i].x, m->boxes[i].y, m->grid_size))
        {
            if (check_collision(&m->boxes[i].x, &m->boxes[i].y, speed, m, direction, i, 0))
            {
                return 1;
            }
        }

        if (squares_intersecting(back_x, back_y, m->grid_size, m->boxes[i].x, m->boxes[i].y, m->grid_size))
        {
            pulled_box_index = i;
        }
    }

    // If there was no collision move the object

    if (direction == LEFT)
    {
        *object_x -= speed;
        if (pulled_box_index != -1 && is_grabbing)
        {
            m->boxes[pulled_box_index].x -= speed;
        }
    }
    else if (direction == RIGHT)
    {
        *object_x += speed;
        if (pulled_box_index != -1 && is_grabbing)
        {
            m->boxes[pulled_box_index].x += speed;
        }
    }
    else if (direction == DOWN)
    {
        *object_y += speed;
        if (pulled_box_index != -1 && is_grabbing)
        {
            m->boxes[pulled_box_index].y += speed;
        }
    }
    else if (direction == UP)
    {
        *object_y -= speed;
        if (pulled_box_index != -1 && is_grabbing)
        {
            m->boxes[pulled_box_index].y -= speed;
        }
    }

    return 0;
}

void move_player(struct player *p, struct map *m)
{
    if (p->is_moving == 0)
        return;

    // we can change this so that the movement is always aligned with the move_steps

    check_collision(&p->pos_x, &p->pos_y, p->speed, m, p->direction, -1, p->is_grabbing);

    // Player has moved so we need to update the move_counter and check if the player has moved to the next grid position

    p->move_counter += p->speed;

    if (p->move_counter == m->grid_size)
    {
        p->move_counter = 0;
        p->is_moving = 0;
    }
}