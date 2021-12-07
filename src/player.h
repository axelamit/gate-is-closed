#ifndef PLAYER_H__
#define PLAYER_H__

struct player
{
    int x;
    int y;
};

struct player create_player(int x, int y);
void move_player(struct player *p);

#endif