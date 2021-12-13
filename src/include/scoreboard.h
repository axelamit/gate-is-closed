#ifndef __SCOREBOARD_H_
#define __SCOREBOARD_H_

#include "game.h"

#define SCOREBOARD_ENTRY_LENGTH 7 // 3 for name and 4 for score (assuming time isn't more than 3h)
#define SCOREBOARD_ENTRIES 3

void get_scoreboard(struct game *g);
int get_scoreboard_pos(int time, int level);
void add_scoreboard_entry(char name[], int time, int level);

#endif