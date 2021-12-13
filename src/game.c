#include "include/game.h"
#include "include/simulate.h"
#include "include/texture.h"

#include "include/i2c.h"
#include "include/scoreboard.h"

void init_game(struct game *g, int level)
{
    init_player(&g->player, 100, 100, 0); // starting pos should be dependent on the size of the grid (change this + player.c)
    create_map(&g->map, 1);
    g->level = level;
    g->time = 0;
    g->final_time = 0;
    g->state = GAME;
    g->curr_button = 0;

    for (int i = 0; i < 3; i++)
    {
        g->curr_name[i] = 'A';
    }

    for (int i = 0; i < 3 * SCOREBOARD_ENTRIES; i++)
    {
        g->scoreboard_names[i] = 'A';
    }
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

    gamestate[buffer_index] = g->time;
    buffer_index++;

    // Pick name
    gamestate[buffer_index] = 0;
    if (g->state == SELECT_NAME || g->state == READ_SCOREBOARD)
    {
        gamestate[buffer_index] = 1;
        for (int i = 0; i < 3; i++)
        {
            gamestate[buffer_index + i + 1] = g->curr_name[i];
        }
        buffer_index += 3;
    }
    buffer_index++;

    // Add scoreboard to gamestate
    gamestate[buffer_index] = 0;
    buffer_index++;
    if (g->state == DISPLAY_SCOREBOARD)
    {
        gamestate[buffer_index - 1] = SCOREBOARD_ENTRIES + 3 * SCOREBOARD_ENTRIES;

        // Sort entries by score (also swap names at the same time)
        int temp;
        for (int i = 0; i < SCOREBOARD_ENTRIES; i++)
        {
            for (int j = i + 1; j < SCOREBOARD_ENTRIES; j++)
            {
                if (g->scoreboard_scores[i] > g->scoreboard_scores[j])
                {
                    temp = g->scoreboard_scores[i];
                    g->scoreboard_scores[i] = g->scoreboard_scores[j];
                    g->scoreboard_scores[j] = temp;

                    for (int k = 0; k < 3; k++)
                    {
                        temp = g->scoreboard_names[i * 3 + k];
                        g->scoreboard_names[i * 3 + k] = g->scoreboard_names[j * 3 + k];
                        g->scoreboard_names[j * 3 + k] = temp;
                    }
                }
            }
        }

        // Add scoreboard scores to gamestate
        for (int i = 0; i < SCOREBOARD_ENTRIES; i++)
        {
            gamestate[buffer_index] = g->scoreboard_scores[i];
            buffer_index++;
        }

        // Add scoreboard names to gamestate
        for (int i = 0; i < 3 * SCOREBOARD_ENTRIES; i++)
        {
            gamestate[buffer_index] = g->scoreboard_names[i];
            buffer_index++;
        }
    }
}

void next_gamestate(int gamestate[], struct game *g)
{
    if (g->state == GAME)
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
                // g->level += 1;
                // g->time = 0;
                g->final_time = g->time;
                g->state = SELECT_NAME;
                // g->map = create_map(g->level);
            }
        }
    }
    else if (g->state == SELECT_NAME)
    {
        if (g->curr_button == 1)
        {
            g->state = READ_SCOREBOARD;
        }
        else
        {
            for (int i = 2; i <= 4; i++)
            {
                if (g->curr_button == i)
                {
                    g->curr_name[4 - i]++;
                    if (g->curr_name[4 - i] > 90)
                    {
                        g->curr_name[4 - i] = 'A';
                    }
                }
            }
        }
    }
    else if (g->state == READ_SCOREBOARD)
    {
        // for (int i = 0; i < 50; i++)
        // {
        //     i2c_send_char(i, 0);
        // }

        add_scoreboard_entry(g->curr_name, g->final_time, g->level);
        get_scoreboard(g);
        g->state = DISPLAY_SCOREBOARD;
    }
    else if (g->state == DISPLAY_SCOREBOARD)
    {
        if (g->curr_button == 1)
        {
            g->player.pos_x = 100;
            g->player.pos_y = 100;
            g->time = 0;
            g->state = GAME;
        }
    }

    get_gamestate(gamestate, g);
}