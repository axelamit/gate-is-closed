#ifndef BOX_H__
#define BOX_H__

enum type
{
    AND = 0,
    OR = 1,
    NOT = 2
};

struct box
{
    int x;
    int y;
    int rotation;
    int type;
};

#endif