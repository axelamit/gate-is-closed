#include "include/map.h"
#include "include/rotation.h"
#include "include/texture.h"
#include "include/box.h"
#include "include/wire.h"

// For all levels
int map[MAX_GROUND_SIZE * MAX_GROUND_SIZE];
int box_pos[2 * MAX_GROUND_SIZE];
int box_labels[MAX_GROUND_SIZE];
int connector_pos[2 * MAX_GROUND_SIZE];
int connector_labels[MAX_GROUND_SIZE];
int connections[2 * MAX_GROUND_SIZE];

// Level 1
int height1 = 9;
int width1 = 10;
int door_x1 = 6;
int door_y1 = 0;
int num_boxes1 = 2;
int num_connectors1 = 7;
int num_connections1 = 6;
int box_pos1[] = {4, 2, 4, 6};
int box_labels1[] = {AND, OR};
int connector_pos1[] = {0, 4, 0, 6, 4, 4, 4, 6, 4, 5, 6, 5, 6, 0};
int connector_labels1[] = {SOURCE, SOURCE, SIMPLE, SIMPLE, BOX_CONNECTOR, SIMPLE, DOOR_CONNECTOR};
int connections1[] = {0, 2, 1, 3, 2, 4, 3, 4, 4, 5, 5, 6};

int num_input_states1 = 2;
int input_states1[] = {0, 1, 1, 0};

// Level 2
int height2 = 10;
int width2 = 10;
int door_x2 = 6;
int door_y2 = 0;
int num_boxes2 = 5;
int num_connectors2 = 15;
int num_connections2 = 16;
int box_pos2[] = {4, 2, 4, 2, 8, 6, 7, 7, 1, 3};
int box_labels2[] = {AND, AND, NAND, OR, NOR};
int connector_pos2[] = {0, 4, 1, 4, 3, 4, 3, 6, 5, 6, 5, 7, 6, 7, 6, 0, 5, 8, 3, 8, 3, 7, 2, 8, 1, 7, 2, 6, 0, 6};
int connector_labels2[] = {SOURCE, SIMPLE, SIMPLE, BOX_CONNECTOR, SIMPLE, BOX_CONNECTOR, SIMPLE, DOOR_CONNECTOR, SIMPLE, BOX_CONNECTOR, SIMPLE, SIMPLE, SIMPLE, SIMPLE, SOURCE};
int connections2[] = {0, 1, 1, 2, 1, 12, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 5, 9, 8, 10, 9, 11, 9, 12, 10, 13, 3, 13, 11, 14, 13};

int num_input_states2 = 4;
int input_states2[] = {0, 0, 1, 1, 0, 1, 0, 1};

// Fill the map with walls on the sides and ground in the middle
void fill_map(struct map *m)
{
    for (int i = 0; i < m->height; i++)
    {
        for (int j = 0; j < m->width; j++)
        {
            // Check if the current position is on any of the sides
            if (i == 0 || j == 0 || i == m->height - 1 || j == m->width - 1)
            {
                map[i * m->width + j] = WALL;
            }
            else
            {
                map[i * m->width + j] = GROUND;
            }
        }
    }
}

// Writes the values of the current level to the global level arrays
void create_level(struct map *m, int height, int width, int door_x, int door_y, int num_boxes, int num_connectors, int num_connections, int *curr_box_pos, int *curr_box_labels, int *curr_connector_pos, int *curr_connector_labels, int *curr_connections, int num_input_states, int *curr_input_states)
{
    // Copy the values to the global arrays

    m->height = height;
    m->width = width;

    // Fill map and add a door
    fill_map(m);
    map[door_y * m->width + door_x] = DOOR_CLOSED;

    m->num_boxes = num_boxes;
    m->num_connectors = num_connectors;
    m->num_connections = num_connections;
    m->num_input_states = num_input_states;

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

    int num_inputs = 0;
    for (int i = 0; i < m->num_connectors; i++)
    {
        if (curr_connector_labels[i] == SOURCE)
        {
            // Count the number of input sources (used in the  next for-loop)
            num_inputs++;
        }

        connector_labels[i] = curr_connector_labels[i];
    }

    for (int i = 0; i < 2 * num_connections; i++)
    {
        connections[i] = curr_connections[i];
    }

    for (int i = 0; i < num_input_states * num_inputs; i++)
    {
        m->input_states[i] = curr_input_states[i];
    }
}

// Create the map based on the current level
void create_map(struct map *m, int level)
{
    // Set grid size of map
    m->grid_size = 100;

    // Create the level
    if (level == 1)
    {
        create_level(m, height1, width1, door_x1, door_y1, num_boxes1, num_connectors1, num_connections1, box_pos1, box_labels1, connector_pos1, connector_labels1, connections1, num_input_states1, input_states1);
    }
    else if (level == 2)
    {
        create_level(m, height2, width2, door_x2, door_y2, num_boxes2, num_connectors2, num_connections2, box_pos2, box_labels2, connector_pos2, connector_labels2, connections2, num_input_states2, input_states2);
    }

    // Save map values
    for (int i = 0; i < m->height; i++)
    {
        for (int j = 0; j < m->width; j++)
        {
            m->ground[i][j].texture = map[i * m->width + j];
        }
    }

    // Save box values
    for (int i = 0; i < m->num_boxes; i++)
    {
        m->boxes[i].rotation = 0;

        m->boxes[i].type = box_labels[i];
        m->boxes[i].x = box_pos[i * 2];
        m->boxes[i].y = box_pos[i * 2 + 1];

        m->boxes[i].x = m->boxes[i].x * m->grid_size;
        m->boxes[i].y = m->boxes[i].y * m->grid_size;
    }

    // Save connector values
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
        }

        m->connectors[i].connector_type = connector_labels[i];
        m->connectors[i].x = connector_pos[i * 2];
        m->connectors[i].y = connector_pos[i * 2 + 1];
    }

    // Sva connections
    for (int i = 0; i < m->num_connections; i++)
    {
        int frm = connections[i * 2];
        int to = connections[i * 2 + 1];

        // Check for forward_connection that has not been assigned yet
        for (int j = 0; j < sizeof(m->connectors[frm].forward_connections); j++)
        {
            if (m->connectors[frm].forward_connections[j] == -1)
            {
                m->connectors[frm].forward_connections[j] = to;
                break;
            }
        }

        // Check for backwards_connection that has not been assigned yet
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