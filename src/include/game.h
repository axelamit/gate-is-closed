#ifndef GAME_H__
#define GAME_H__

#include "player.h"
#include "map.h"

struct game
{
    struct player player;
    struct map map;
    int level;
};

void init_game(struct game *g, int level);
void get_gamestate(int gamestate[], struct game *g);
void next_gamestate(int gamestate[], struct game *g);

#endif