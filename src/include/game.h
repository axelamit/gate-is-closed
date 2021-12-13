#ifndef GAME_H__
#define GAME_H__

#include "player.h"
#include "map.h"

enum GAMESTATE
{
    GAME = 0,
    SELECT_NAME = 1,
    READ_SCOREBOARD = 2,
    DISPLAY_SCOREBOARD = 3
};

struct game
{
    struct player player;
    struct map map;
    int level;
    int time;
    int final_time;
    int state;

    int curr_button;
    char curr_name[3];

    int scoreboard_scores[3];
    char scoreboard_names[9];
};

void init_game(struct game *g, int level);
void get_gamestate(int gamestate[], struct game *g);
void next_gamestate(int gamestate[], struct game *g);

#endif