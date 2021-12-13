#include "include/i2c.h"
#include "include/game.h"
#include "include/scoreboard.h"

void get_scoreboard(struct game *g)
{
    int loc = EEPROM_MEM_ADDR + (g->level - 1) * (SCOREBOARD_ENTRY_LENGTH * SCOREBOARD_ENTRIES);

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            g->scoreboard_names[i * 3 + j] = i2c_recv_char(loc);
            loc++;
        }
        g->scoreboard_scores[i] = i2c_recv_int(loc);
        loc += 4;
    }
}

int get_scoreboard_pos(int time, int level)
{
    int max_time = 0;
    int max_index = 0;

    for (int i = SCOREBOARD_ENTRY_LENGTH - 4; i < SCOREBOARD_ENTRY_LENGTH * SCOREBOARD_ENTRIES; i += SCOREBOARD_ENTRY_LENGTH)
    {
        int entry_time = i2c_recv_int(EEPROM_MEM_ADDR + (level - 1) * (SCOREBOARD_ENTRY_LENGTH * SCOREBOARD_ENTRIES) + i);

        if (entry_time == 0)
        {
            max_time = 2047483647;
            max_index = i - SCOREBOARD_ENTRY_LENGTH + 4;
        }
        else if (entry_time > max_time)
        {
            max_time = entry_time;
            max_index = i - SCOREBOARD_ENTRY_LENGTH + 4;
        }
    }

    if (time < max_time)
    {
        return max_index;
    }
    return -1;
}

void add_scoreboard_entry(char name[], int time, int level)
{
    int pos = get_scoreboard_pos(time, level);

    if (pos != -1)
    {
        int mem_location = EEPROM_MEM_ADDR + (level - 1) * (SCOREBOARD_ENTRY_LENGTH * SCOREBOARD_ENTRIES) + pos;

        i2c_send_char(mem_location, name[0]);
        i2c_send_char(mem_location + 1, name[1]);
        i2c_send_char(mem_location + 2, name[2]);
        i2c_send_int(mem_location + 3, time);
    }
}