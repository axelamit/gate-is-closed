#include <stdio.h>

#include "include/game.h"
#include "include/simulate.h"
#include "include/texture.h"

void init_game(struct game *g, int level)
{
    init_player(&g->player, 100, 100, 0); // starting pos should be dependent on the size of the grid (change this + player.c)
    create_map(&g->map, 1);
    g->level = level;
}

void get_gamestate(int gamestate[], struct game *g)
{
    gamestate[0] = g->player.pos_x;
    gamestate[1] = g->player.pos_y;

    // Add map to gamestate

    gamestate[2] = g->map.width;
    gamestate[3] = g->map.height;

    int buffer_index = 4;
    for (int i = 0; i < g->map.height; i++)
    {
        int j;
        for (j = 0; j < g->map.width; j++)
        {
            gamestate[4 + (i * g->map.width + j)] = g->map.ground[i][j].texture;
            buffer_index++;
        }
    }

    // Add connectors to gamestate

    gamestate[buffer_index] = g->map.num_connectors;
    buffer_index++;

    for (int i = 0; i < g->map.num_connectors; i++)
    {

        int num_connections = 0;
        int old_buffer_index = buffer_index;
        buffer_index++;

        gamestate[buffer_index] = g->map.connectors[i].connector_type;
        gamestate[buffer_index + 1] = g->map.connectors[i].x;
        gamestate[buffer_index + 2] = g->map.connectors[i].y;

        buffer_index += 3;

        for (int j = 0; j < 4; j++)
        {
            int next_index = g->map.connectors[i].forward_connections[j];
            if (next_index != -1)
            {
                int wire_state = g->map.connectors[i].state;
                gamestate[buffer_index] = g->map.connectors[next_index].connector_type;
                gamestate[buffer_index + 1] = g->map.connectors[next_index].x;
                gamestate[buffer_index + 2] = g->map.connectors[next_index].y;
                gamestate[buffer_index + 3] = wire_state;
                num_connections++;
                buffer_index += 4;
            }
        }
        gamestate[old_buffer_index] = num_connections;
    }

    // Add boxes to gamestate

    gamestate[buffer_index] = g->map.num_boxes;
    buffer_index++;

    for (int i = 0; i < g->map.num_boxes; i++)
    {
        gamestate[buffer_index] = g->map.boxes[i].type;
        gamestate[buffer_index + 1] = g->map.boxes[i].x;
        gamestate[buffer_index + 2] = g->map.boxes[i].y;
        buffer_index += 3;
    }
}

void next_gamestate(int gamestate[], struct game *g)
{
    move_player(&g->player, &g->map);
    int complete = simulate(&g->map);
    if (complete)
    {
        int door_x = -1;
        int door_y = -1;
        for (int i = 0; i < g->map.height; i++)
        {
            for (int j = 0; j < g->map.width; j++)
            {
                if (g->map.ground[i][j].texture == DOOR_CLOSED || g->map.ground[i][j].texture == DOOR_OPEN)
                {
                    g->map.ground[i][j].texture = DOOR_OPEN;
                    door_x = j * g->map.grid_size;
                    door_y = i * g->map.grid_size;
                }
            }
        }

        if (door_x == g->player.pos_x && door_y == g->player.pos_y)
        {
            g->level += 1;
            // g->map = create_map(g->level);
            g->player.pos_x = 100;
            g->player.pos_y = 100;
        }
        // if (g->player.pos_x / g->map.grid_size == )
    }
    get_gamestate(gamestate, g);
}