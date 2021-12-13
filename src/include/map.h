#ifndef MAP_H__
#define MAP_H__

#include "block.h"
#include "box.h"
#include "wire.h"

#define MAX_GROUND_SIZE 15

struct map
{
    int grid_size;
    struct block ground[MAX_GROUND_SIZE][MAX_GROUND_SIZE];
    struct box boxes[MAX_GROUND_SIZE];
    struct wire_connection connectors[MAX_GROUND_SIZE * 3];
    int width;
    int height;
    int num_boxes;
    int num_connectors;
    int num_connections;
    int num_input_states;
    int input_states[MAX_GROUND_SIZE * 3];
};

void create_map(struct map *m, int level);

#endif