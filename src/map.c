#include "include/map.h"
#include "include/rotation.h"
#include "include/texture.h"
#include "include/box.h"
#include "include/wire.h"

// For all levels
char map[MAX_GROUND_SIZE * MAX_GROUND_SIZE];
int box_pos[2 * MAX_GROUND_SIZE];
char box_labels[MAX_GROUND_SIZE];
int connector_pos[2 * MAX_GROUND_SIZE];
char connector_labels[MAX_GROUND_SIZE];
int connections[2 * MAX_GROUND_SIZE];

// Level 1
int height1 = 9;
int width1 = 10;
int door_x1 = 5;
int door_y1 = 0;
int num_boxes1 = 2;
int num_connectors1 = 5;
int num_connections1 = 4;
int box_pos1[] = {4, 2, 4, 6};
char box_labels1[] = {'a', 'o'};
int connector_pos1[] = {2, 4, 4, 4, 7, 4, 7, 7, 4, 7};
char connector_labels1[] = {'s', 'n', 'c', 'c', 'd'};
int connections1[] = {0, 1, 1, 2, 2, 3, 3, 4};

// Level 2
int height2 = 12;
int width2 = 12;
int door_x2 = 9;
int door_y2 = 0;
int num_boxes2 = 3;
int num_connectors2 = 5;
int num_connections2 = 4;
int box_pos2[] = {6, 3, 5, 6, 2, 2};
char box_labels2[] = {'a', 'a', 'o'};
int connector_pos2[] = {2, 4, 4, 4, 7, 4, 7, 7, 4, 7};
char connector_labels2[] = {'s', 'n', 'c', 'c', 'd'};
int connections2[] = {0, 1, 1, 2, 2, 3, 3, 4};

void fill_map(struct map *m)
{
    for (int i = 0; i < m->height; i++)
    {
        for (int j = 0; j < m->width; j++)
        {
            if (i == 0 || j == 0 || i == m->height - 1 || j == m->width - 1)
            {
                map[i * m->width + j] = 'x';
            }
            else
            {
                map[i * m->width + j] = 'o';
            }
        }
    }
}

void create_level(struct map *m, int height, int width, int door_x, int door_y, int num_boxes, int num_connectors, int num_connections, int *curr_box_pos, char *curr_box_labels, int *curr_connector_pos, char *curr_connector_labels, int *curr_connections)
{
    m->height = height;
    m->width = width;

    fill_map(m);
    map[door_y * m->width + door_x] = 'd';

    m->num_boxes = num_boxes;
    m->num_connectors = num_connectors;
    m->num_connections = num_connections;

    for (int i = 0; i < 2 * m->num_boxes; i++)
    {
        box_pos[i] = curr_box_pos[i];
    }

    for (int i = 0; i < m->num_boxes; i++)
    {
        box_labels[i] = curr_box_labels[i];
    }

    for (int i = 0; i < 2 * m->num_connectors; i++)
    {
        connector_pos[i] = curr_connector_pos[i];
    }

    for (int i = 0; i < m->num_connectors; i++)
    {
        connector_labels[i] = curr_connector_labels[i];
    }

    for (int i = 0; i < 2 * num_connections; i++)
    {
        connections[i] = curr_connections[i];
    }
}

void create_map(struct map *m, int level)
{
    m->grid_size = 100;

    if (level == 1)
    {
        create_level(m, height1, width1, door_x1, door_y1, num_boxes1, num_connectors1, num_connections1, box_pos1, box_labels1, connector_pos1, connector_labels1, connections1);
    }
    else if (level == 2)
    {
        create_level(m, height2, width2, door_x2, door_y2, num_boxes2, num_connectors2, num_connections2, box_pos2, box_labels2, connector_pos2, connector_labels2, connections2);
    }

    char c;
    for (int i = 0; i < m->height; i++)
    {
        for (int j = 0; j < m->width; j++)
        {
            c = map[i * m->width + j];
            if (c == 'x')
            {
                m->ground[i][j].texture = WALL;
            }
            else if (c == 'o')
            {
                m->ground[i][j].texture = GROUND;
            }
            else if (c == 'd')
            {
                m->ground[i][j].texture = DOOR_CLOSED;
            }
        }
    }

    for (int i = 0; i < m->num_boxes; i++)
    {
        m->boxes[i].rotation = 0;

        char s = box_labels[i];
        m->boxes[i].x = box_pos[i * 2];
        m->boxes[i].y = box_pos[i * 2 + 1];

        m->boxes[i].x = m->boxes[i].x * m->grid_size;
        m->boxes[i].y = m->boxes[i].y * m->grid_size;

        if (s == 'a')
        {
            m->boxes[i].type = AND;
        }
        else if (s == 'o')
        {
            m->boxes[i].type = OR;
        }
        else if (s == 'n')
        {
            m->boxes[i].type = NOT;
        }
    }

    for (int i = 0; i < m->num_connectors; i++)
    {
        m->connectors[i].state = 0;

        // Clear connections
        for (int j = 0; j < 4; j++)
        {
            m->connectors[i].forward_connections[j] = -1;
            m->connectors[i].backwards_connections[j] = -1;
        }

        // Clear states
        for (int j = 0; j < 4; j++)
        {
            m->connectors[i].forward_states[j] = 0;
            // wire.backwards_states[j] = 0;
        }

        char s = connector_labels[i];
        m->connectors[i].x = connector_pos[i * 2];
        m->connectors[i].y = connector_pos[i * 2 + 1];

        if (s == 'n')
        {
            m->connectors[i].connector_type = SIMPLE;
        }
        else if (s == 's')
        {
            m->connectors[i].connector_type = SOURCE;
        }
        else if (s == 'c')
        {
            m->connectors[i].connector_type = BOX_CONNECTOR;
        }
        else if (s == 'd')
        {
            m->connectors[i].connector_type = DOOR_CONNECTOR;
        }
    }

    for (int i = 0; i < m->num_connections; i++)
    {
        int frm = connections[i * 2];
        int to = connections[i * 2 + 1];

        // Check for connection that has not been assigned yet
        for (int j = 0; j < sizeof(m->connectors[frm].forward_connections); j++)
        {
            if (m->connectors[frm].forward_connections[j] == -1)
            {
                m->connectors[frm].forward_connections[j] = to;
                break;
            }
        }

        for (int j = 0; j < sizeof(m->connectors[to].backwards_connections); j++)
        {
            if (m->connectors[to].backwards_connections[j] == -1)
            {
                m->connectors[to].backwards_connections[j] = frm;
                break;
            }
        }
    }
}