#include "player.h"

struct player create_player(int x, int y)
{
    struct player p;
    p.x = 10;
    p.y = 10;
    return p;
}

void move_player(struct player *p)
{
    p->x++;
}