#include "include/i2c.h"
#include "include/game.h"
#include "include/scoreboard.h"

// Get the scoreboard entries of the current level
void get_scoreboard(struct game *g)
{
    int loc = EEPROM_MEM_ADDR + (g->level - 1) * (SCOREBOARD_ENTRY_LENGTH * SCOREBOARD_ENTRIES);

    // Iterate through the entries and save them to the game struct
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            // Save letter in name
            g->scoreboard_names[i * 3 + j] = i2c_recv_char(loc);
            loc++;
        }
        // Save score
        g->scoreboard_scores[i] = i2c_recv_int(loc);
        loc += 4;
    }
}

// Get the index that the current time will place on the scoreboard of the current level (if time is too slow it returns -1)
int get_scoreboard_pos(int time, int level)
{
    int max_time = 0;
    int max_index = 0;

    // Loop through each of the current entries and get the max_time as well as the index of that one
    for (int i = SCOREBOARD_ENTRY_LENGTH - 4; i < SCOREBOARD_ENTRY_LENGTH * SCOREBOARD_ENTRIES; i += SCOREBOARD_ENTRY_LENGTH)
    {
        int entry_time = i2c_recv_int(EEPROM_MEM_ADDR + (level - 1) * (SCOREBOARD_ENTRY_LENGTH * SCOREBOARD_ENTRIES) + i);

        // If entry time is 0 the score doesn't exists on this position yet (its free for assignment)
        if (entry_time == 0)
        {
            // Max time is almost max size of int since we always want to swap at this index instead of some other index where we an actual score is present
            max_time = 2047483647;
            max_index = i - SCOREBOARD_ENTRY_LENGTH + 4;
        }
        else if (entry_time > max_time)
        {
            // Entry time is the greatest that has been seen, update the max_time and max_index
            max_time = entry_time;
            max_index = i - SCOREBOARD_ENTRY_LENGTH + 4;
        }
    }

    // If the current time is less than the max_time we can overwrite that position with the new time
    if (time < max_time)
    {
        return max_index;
    }
    return -1;
}

// Add scoreboard entry to scoreboard if low enough
void add_scoreboard_entry(char name[], int time, int level)
{
    // Get the position where we want to insert the entry
    int pos = get_scoreboard_pos(time, level);

    // Check that score is low enough to be placed on the scoreboard
    if (pos != -1)
    {
        // The score is low enough, update the eeprom data of the scoreboard
        int mem_location = EEPROM_MEM_ADDR + (level - 1) * (SCOREBOARD_ENTRY_LENGTH * SCOREBOARD_ENTRIES) + pos;

        i2c_send_char(mem_location, name[0]);
        i2c_send_char(mem_location + 1, name[1]);
        i2c_send_char(mem_location + 2, name[2]);
        i2c_send_int(mem_location + 3, time);
    }
}