#include "include/game.h"
#include "include/simulate.h"
#include "include/texture.h"

#include "include/i2c.h"
#include "include/scoreboard.h"

// Add the initial values to the game
void init_game(struct game *g, int level)
{
    init_player(&g->player, 100, 100, 0); // starting pos should be dependent on the size of the grid (change this + player.c)
    create_map(&g->map, 1);
    g->level = level;
    g->time = 0;
    g->final_time = 0;
    g->state = GAME;
    g->curr_button = 0;
    g->input_state = 0;

    for (int i = 0; i < 3; i++)
    {
        g->curr_name[i] = 'A';
    }

    for (int i = 0; i < 3 * SCOREBOARD_ENTRIES; i++)
    {
        g->scoreboard_names[i] = 'A';
    }
}

// Adds all of the gamestate data into one array that can be sent to the computer via UART
void get_gamestate(int gamestate[], struct game *g)
{
    // Add player pos
    gamestate[0] = g->player.pos_x;
    gamestate[1] = g->player.pos_y;

    // Add map to gamestate
    gamestate[2] = g->map.width;
    gamestate[3] = g->map.height;

    for (int i = 0; i < g->map.height; i++)
    {
        for (int j = 0; j < g->map.width; j++)
        {
            if (g->map.ground[i][j].texture == DOOR_CLOSED)
            {
                gamestate[4] = 1;
                gamestate[5] = i;
                gamestate[6] = j;
            }
            else if (g->map.ground[i][j].texture == DOOR_OPEN)
            {
                gamestate[4] = 0;
                gamestate[5] = i;
                gamestate[6] = j;
            }
        }
    }

    int buffer_index = 7;

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

        // Loop through each of the current connectors children and add them to the gamestate if present
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

    // Add time to gamestate
    gamestate[buffer_index] = g->time;
    buffer_index++;

    // Add simulation input state to gamestate
    gamestate[buffer_index] = g->input_state + 1;
    buffer_index++;
    gamestate[buffer_index] = g->map.num_input_states;
    buffer_index++;

    // Add the name that is currently picked to the gamestate
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

// Get the next gamestate
void next_gamestate(int gamestate[], struct game *g)
{
    if (g->state == GAME)
    {
        // The game is in "play mode"

        // Move player while checking for collision
        move_player(&g->player, &g->map);

        // Simulate all input_states except the one selected, if the door is closed on any of the simulations the level is not complete

        int complete = 1;
        for (int i = 0; i < g->input_state; i++)
        {
            int comp = simulate(&g->map, i);
            if (comp == 0)
            {
                complete = 0;
            }
        }

        for (int i = g->input_state + 1; i < g->map.num_input_states; i++)
        {
            int comp = simulate(&g->map, i);
            if (comp == 0)
            {
                complete = 0;
            }
        }

        // Simulate the current input_state, this is done in order to for that state to be the one displayed on the screen
        int comp = simulate(&g->map, g->input_state);
        if (comp == 0)
        {
            complete = 0;
        }

        // Check if level is complete
        if (complete)
        {
            // Find the door and switch its state from closed to open
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

            // Check if player is at the same position as the door, if so the level is officially completed
            if (door_x == g->player.pos_x && door_y == g->player.pos_y)
            {
                // Save the final time
                g->final_time = g->time;

                // Check if time is fast enough to place on the scoreboard
                int pos = get_scoreboard_pos(g->final_time, g->level);

                if (pos == -1)
                {
                    // Time is too slow, goto next game
                    g->state = NEXT_GAME;
                }
                else
                {
                    // Time is fast enough, select a name that is placed on the scoreboard
                    g->state = SELECT_NAME;
                }
            }
        }
    }
    else if (g->state == SELECT_NAME)
    {
        if (g->curr_button == 1)
        {
            // Selection of name is done, read scoreboard
            g->state = READ_SCOREBOARD;
        }
        else
        {
            // Check if any button corresponding to any of the letters is pressed. If so we increase the value of that letter. If its greater than 90 (Z) we wrap back to A.
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
        // Add current run to the scoreboard
        add_scoreboard_entry(g->curr_name, g->final_time, g->level);

        // Load the scoreboard
        get_scoreboard(g);

        // Display scoreboard
        g->state = DISPLAY_SCOREBOARD;
    }
    else if (g->state == DISPLAY_SCOREBOARD)
    {
        // Display scoreboard until a button is pressed to start the next game
        if (g->curr_button == 1)
        {
            g->state = NEXT_GAME;
        }
    }
    else if (g->state == NEXT_GAME)
    {
        // Prepare for the next game

        // Reset player position
        g->player.pos_x = 100;
        g->player.pos_y = 100;

        // Increase the level
        g->level++;
        create_map(&g->map, g->level);

        // Reset time
        g->time = 0;

        // Start the level
        g->state = GAME;
    }

    // Load in the new gamestate
    get_gamestate(gamestate, g);
}