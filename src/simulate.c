#include "include/map.h"
#include "include/wire.h"
#include "include/box.h"

int simulate_step(int index, struct map *m)
{
    // Get the current connector type
    int type = m->connectors[index].connector_type;

    // If type is source we want to return its state
    if (type == SOURCE)
    {
        return m->connectors[index].state;
    }

    // Recursively call each of the current nodes children and save their values to the inputs array
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

    // Check current type
    if (type != BOX_CONNECTOR)
    {
        // When the type isn't a box_connector we want the output to be one if any of the inputs is one
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
                // There is a box at the current node

                if (m->boxes[i].type == OR)
                {
                    // If the type of the box is OR we want the output to be 1 if any of the inputs is 1
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
                    // If the type of the box is AND we want the output to be 0 if any of the inputs is 0, else 1
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
                    // If any of the inputs is 1 the output is 0, else 1
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
                    // Same as AND but the output is reversed
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
                    // Same as OR but the output is reversed
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
            // Door found, assign to door_index
            door_index = i;
        }
        else if (m->connectors[i].connector_type == SOURCE)
        {
            // Source found, set the state of the source to the corresponding input_state offset
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