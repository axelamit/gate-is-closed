#ifndef WIRE_H__
#define WIRE_H__

enum connector_type
{
    SIMPLE = 0,
    SOURCE = 1,
    BOX_CONNECTOR = 2,
    DOOR_CONNECTOR = 3
};

struct wire_connection
{
    int x;
    int y;
    int connector_type;
    int forward_connections[4];
    int forward_states[4];
    int backwards_connections[4];
    // int backwards_states[4];

    int state;
};

#endif