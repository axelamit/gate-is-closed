#include "include/map.h"
#include "include/wire.h"
#include "include/box.h"

int simulate_step(int index, struct map *m)
{
    int type = m->connectors[index].connector_type;

    if (type == SOURCE)
    {
        return m->connectors[index].state;
    }

    int inputs[4];

    for (int i = 0; i < 4; i++)
    {
        int next_node = m->connectors[index].backwards_connections[i];
        if (next_node != -1)
        {
            inputs[i] = simulate_step(next_node, m);
        }
        else
        {
            inputs[i] = -1;
        }
    }

    if (type != BOX_CONNECTOR)
    {
        for (int i = 0; i < 4; i++)
        {
            // If one of the parent nodes is lit, so should the current node also be
            if (inputs[i] == 1)
            {
                return m->connectors[index].state = 1;
            }
        }
    }
    else
    {
        // Loop through each box and check if the box is positioned at the current node
        int curr_x = m->connectors[index].x;
        int curr_y = m->connectors[index].y;

        for (int i = 0; i < m->num_boxes; i++)
        {
            if (m->boxes[i].x / m->grid_size == curr_x && m->boxes[i].y / m->grid_size == curr_y)
            {

                if (m->boxes[i].type == OR)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        if (inputs[j] == 1)
                        {
                            return m->connectors[index].state = 1;
                        }
                    }
                    return m->connectors[index].state = 0;
                }
                else if (m->boxes[i].type == AND)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        if (inputs[j] == 0)
                        {
                            return m->connectors[index].state = 0;
                        }
                    }
                    return m->connectors[index].state = 1;
                }
                else if (m->boxes[i].type == NOT)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        if (inputs[j] == 1)
                        {
                            return m->connectors[index].state = 0;
                        }
                    }
                    return m->connectors[index].state = 1;
                }
                else if (m->boxes[i].type == NAND)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        if (inputs[j] == 0)
                        {
                            return m->connectors[index].state = 1;
                        }
                    }
                    return m->connectors[index].state = 0;
                }
                else if (m->boxes[i].type == NOR)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        if (inputs[j] == 1)
                        {
                            return m->connectors[index].state = 0;
                        }
                    }
                    return m->connectors[index].state = 1;
                }
            }
        }
    }
    return m->connectors[index].state = 0;
}

int simulate(struct map *m, int input_state)
{

    // Find door
    int door_index = 0;

    int sources = 0;
    for (int i = 0; i < m->num_connectors; i++)
    {
        if (m->connectors[i].connector_type == DOOR_CONNECTOR)
        {
            door_index = i;
        }
        else if (m->connectors[i].connector_type == SOURCE)
        {
            m->connectors[i].state = m->input_states[sources * m->num_input_states + input_state];
            sources++;
        }
    }

    // Simulate n steps
    for (int i = 0; i < m->num_connectors; i++)
    {
        simulate_step(door_index, m);
    }

    // Return value of door
    return simulate_step(door_index, m);
}