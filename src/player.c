#include "include/player.h"
#include "include/directions.h"
#include "include/texture.h"

#include <stdio.h>

// Add the initial values to player
void init_player(struct player *p, int x, int y, int direction)
{
    p->pos_x = x;
    p->pos_y = y;
    p->direction = direction;

    p->speed = 25;
    p->is_moving = 0;

    p->move_counter = 0;
    p->is_grabbing = 0;
}

// Check if two squares intersect
int squares_intersecting(int x1, int y1, int size1, int x2, int y2, int size2)
{
    // Check if collision left or right
    int collision_lr = ((x1 < x2 + size2 && x1 > x2) || (x1 + size1 < x2 + size2 && x1 + size1 > x2));

    // Check if collision top or bottom
    int collision_tb = ((y1 > y2 && y1 < y2 + size2) || (y1 + size1 > y2 && y1 + size1 < y2 + size2));

    // Check if collision, the objects need to be positioned at either the same x or y coordinate
    if ((collision_lr && y1 == y2) || (collision_tb && x1 == x2))
    {
        return 1;
    }
    return 0;
}

// Recursively check for collision of an object (can be both a box or a player) and moves the objects
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
            // If player is moving to the left, we check if its colliding with the left wall or with the closed door
            if (m->ground[grid_y][grid_x - 1].texture == WALL || m->ground[grid_y][grid_x - 1].texture == DOOR_CLOSED)
            {
                return 1;
            }
        }
        else if (direction == RIGHT)
        {
            // If player is moving to the right, we check if its colliding with the right wall or with the closed door
            if (m->ground[grid_y][grid_x + 1].texture == WALL || m->ground[grid_y][grid_x + 1].texture == DOOR_CLOSED)
            {
                return 1;
            }
        }
        else if (direction == UP)
        {
            // If player is moving up, we check if its colliding with the top wall or with the closed door
            if (m->ground[grid_y - 1][grid_x].texture == WALL || m->ground[grid_y - 1][grid_x].texture == DOOR_CLOSED)
            {
                return 1;
            }
        }
        else if (direction == DOWN)
        {
            // If player is moving down, we check if its colliding with the bottom wall or with the closed door
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
        // We dont want an object to check for collision with itself
        if (i == curr_box_index)
        {
            continue;
        }

        // Get the position in the next iteration of the current object

        int next_x = x;
        int next_y = y;

        int back_x = x;
        int back_y = y;

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

        // Check if the next position collides with the current box, if so we recursively call this collision function with that box
        if (squares_intersecting(next_x, next_y, m->grid_size, m->boxes[i].x, m->boxes[i].y, m->grid_size))
        {
            if (check_collision(&m->boxes[i].x, &m->boxes[i].y, speed, m, direction, i, 0))
            {
                return 1;
            }
        }

        // Check if there is a box in the direction opposite to the one the player is moving, if so its possible that the player wants to drag that box
        if (squares_intersecting(back_x, back_y, m->grid_size, m->boxes[i].x, m->boxes[i].y, m->grid_size))
        {
            pulled_box_index = i;
        }
    }

    // If there was no collision move the object
    if (direction == LEFT)
    {
        // Move the object
        *object_x -= speed;

        // Check if any box is pullable and the player is grabbing, if so pull that box
        if (pulled_box_index != -1 && is_grabbing)
        {
            m->boxes[pulled_box_index].x -= speed;
        }
    }
    else if (direction == RIGHT)
    {
        // Move the object
        *object_x += speed;

        // Check if any box is pullable and the player is grabbing, if so pull that box
        if (pulled_box_index != -1 && is_grabbing)
        {
            m->boxes[pulled_box_index].x += speed;
        }
    }
    else if (direction == DOWN)
    {
        // Move the object
        *object_y += speed;

        // Check if any box is pullable and the player is grabbing, if so pull that box
        if (pulled_box_index != -1 && is_grabbing)
        {
            m->boxes[pulled_box_index].y += speed;
        }
    }
    else if (direction == UP)
    {
        // Move the object
        *object_y -= speed;

        // Check if any box is pullable and the player is grabbing, if so pull that box
        if (pulled_box_index != -1 && is_grabbing)
        {
            m->boxes[pulled_box_index].y -= speed;
        }
    }

    return 0;
}

void move_player(struct player *p, struct map *m)
{
    // If player isn't moving we dont want to check for collision nor move it
    if (p->is_moving == 0)
        return;

    // Check collision and move player
    check_collision(&p->pos_x, &p->pos_y, p->speed, m, p->direction, -1, p->is_grabbing);

    // Player has moved so we need to update the move_counter and check if the player has moved to the next grid position
    p->move_counter += p->speed;

    if (p->move_counter == m->grid_size)
    {
        // If player has moved to the next grid position we dont want it to move any more
        p->move_counter = 0;
        p->is_moving = 0;
    }
}