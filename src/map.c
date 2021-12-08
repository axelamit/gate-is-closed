#include "include/map.h"
#include "include/rotation.h"
#include "include/texture.h"
#include "include/box.h"
#include "include/wire.h"

char map1[] = {
    'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
    'x', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'x',
    'x', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'x',
    'x', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'x',
    'x', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'd',
    'x', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'x',
    'x', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'x',
    'x', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o', 'x',
    'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};

int box_pos1[] = {4, 2, 4, 6};
char box1_labels[] = {'a', 'o'};

int connector1_pos[] = {2, 4, 4, 4, 7, 4, 7, 7, 4, 7};
char connector1_labels[] = {'s', 'n', 'c', 'c', 'd'};

int connections1[] = {0, 1, 1, 2, 2, 3, 3, 4};

void create_map(struct map *m, int level)
{
    m->grid_size = 100;

    m->height = 9;
    m->width = 10;

    char c;
    for (int i = 0; i < m->height; i++)
    {
        for (int j = 0; j < m->width; j++)
        {
            c = map1[i * m->width + j];
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

    m->num_boxes = 2;

    for (int i = 0; i < m->num_boxes; i++)
    {
        m->boxes[i].rotation = 0;

        char s = box1_labels[i];
        m->boxes[i].x = box_pos1[i * 2];
        m->boxes[i].y = box_pos1[i * 2 + 1];

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

    m->num_connectors = 5;

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

        char s = connector1_labels[i];
        m->connectors[i].x = connector1_pos[i * 2];
        m->connectors[i].y = connector1_pos[i * 2 + 1];

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

    int num_connections = 4;

    for (int i = 0; i < num_connections; i++)
    {
        int frm = connections1[i * 2];
        int to = connections1[i * 2 + 1];

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