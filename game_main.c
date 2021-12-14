#include <stdint.h>
#include <pic32mx.h>

#include "game_main.h"
#include "utils.h"
#include "uart.h"

#include "src/include/game.h"
#include "src/include/i2c.h"

// Declare game struct and gamestate
struct game g;
const int N = 1 * (MAX_GROUND_SIZE * MAX_GROUND_SIZE);
int gamestate[1 * (MAX_GROUND_SIZE * MAX_GROUND_SIZE)];

// Timer globals
int ticks = 0;
int timeoutcount = 0;

// Switch globals
int switch_state = 1;

// Interrupt handler
void user_isr(void)
{
    if (check_timeout() && timeoutcount == 10)
    {
        // Increase game time when clock is ticking
        g.time++;
        timeoutcount = 0;
    }
}

// Handle interrupts (timer)
int check_timeout(void)
{
    if (IFS(0) & (1 << 8))
    {
        timeoutcount++;
        IFS(0) &= ~(1 << 8);
        return 1;
    }
    else if (IFS(0) & (1 << 15))
    {
        ticks++;
        IFS(0) &= ~(1 << 15);
        return 0;
    }

    return 0;
}

// Get state of button 2-4
int getbtns(void)
{
    return ((PORTD & (7 << 5)) >> 5);
}

// Get state of button 1
int getbtn1(void)
{
    return (PORTF >> 1) & 1;
}

// Get state of switches
int getsw(void)
{
    return ((PORTD & (0xf << 8)) >> 8);
}

// Check buttons and switches
void bn_sw_time(void)
{
    // Get status of buttons and switches
    int btn_status = getbtns();
    int btn1_status = getbtn1();
    int sw_status = getsw();

    // Check if any of the buttons is pressed
    if (btn_status || btn1_status)
    {
        if (btn_status & 4)
        {
            // Btn4 - Left is pressed

            // Change what button is currently pressed only if it was just pressed (can't hold in the button)
            if (g.curr_button != 4 && g.curr_button != -1)
            {
                g.curr_button = 4;
            }
            else
            {
                g.curr_button = -1;
            }

            // If player isn't moving we change the direction and move the player
            if (g.player.is_moving == 0)
            {
                g.player.direction = 0;
                g.player.is_moving = 1;
            }
        }
        else if (btn_status & 2)
        {
            // Btn3 - Right is pressed

            // Change what button is currently pressed only if it was just pressed (can't hold in the button)
            if (g.curr_button != 3 && g.curr_button != -1)
            {
                g.curr_button = 3;
            }
            else
            {
                g.curr_button = -1;
            }

            // If player isn't moving we change the direction and move the player
            if (g.player.is_moving == 0)
            {
                g.player.direction = 2;
                g.player.is_moving = 1;
            }
        }
        else if (btn_status & 1)
        {
            // Btn2 - Up is pressed

            // Change what button is currently pressed only if it was just pressed (can't hold in the button)
            if (g.curr_button != 2 && g.curr_button != -1)
            {
                g.curr_button = 2;
            }
            else
            {
                g.curr_button = -1;
            }

            // If player isn't moving we change the direction and move the player
            if (g.player.is_moving == 0)
            {
                g.player.direction = 1;
                g.player.is_moving = 1;
            }
        }
        else if (btn1_status)
        {
            // Btn1 - Down is pressed

            // Change what button is currently pressed only if it was just pressed (can't hold in the button)
            if (g.curr_button != 1 && g.curr_button != -1)
            {
                g.curr_button = 1;
            }
            else
            {
                g.curr_button = -1;
            }

            // If player isn't moving we change the direction and move the player
            if (g.player.is_moving == 0)
            {
                g.player.direction = 3;
                g.player.is_moving = 1;
            }
        }
        else
        {
            // Non mapped button was pressed
            g.curr_button = 0;
        }
    }
    else
    {
        // No button was pressed
        g.curr_button = 0;
    }

    // Check if any of the switches is on
    if (sw_status)
    {
        // Check if switch 4 is on, if so the player is grabbing
        if (sw_status & 8)
        {
            g.player.is_grabbing = 1;
        }

        // Check if switch 3 is on, if so we want to switch the input state of the game (we only want to do this once the time the switch is flipped)
        if (sw_status & 4)
        {
            if (switch_state)
            {
                // Increase input state, wraps to 0 if too large
                g.input_state++;
                if (g.input_state == g.map.num_input_states)
                {
                    g.input_state = 0;
                }
                switch_state = 0;
            }
        }

        // If switch 1 is flipped the eeprom data is reseted
        if (sw_status & 1)
        {
            for (int i = 0; i < 50; i++)
            {
                i2c_send_char(i, 0);
            }
        }
    }
    else
    {
        // Player is not grabbing
        g.player.is_grabbing = 0;

        // Switch is flipped to 0, increase input state
        if (switch_state == 0)
        {
            // Increase input state, wraps to 0 if too large
            g.input_state++;
            if (g.input_state == g.map.num_input_states)
            {
                g.input_state = 0;
            }
            switch_state = 1;
        }
    }
}

// Initialize i2c pins
void i2c_init()
{
    I2C1CON = 0x0; // control register
    /* I2C Baud rate should be less than 400 kHz, is generated by dividing
       the 40 MHz peripheral bus clock down */
    I2C1BRG = 0x0C2; // holds the Baud Rate Generator
    I2C1STAT = 0x0; // status register
    I2C1CONSET = 1 << 13; // SIDL = 1 (Discontinue module operation when the device enters Idle mode)
    I2C1CONSET = 1 << 15; // ON = 1
}

// Initialize uart pins
void uart_init()
{
    /* Configure UART1 for 115200 baud, no interrupts */
    U1BRG = calculate_baudrate_divider(80000000, 115200, 0);
    U1STA = 0;
    /* 8-bit data, no parity, 1 stop bit */
    U1MODE = 0x8000;
    /* Enable transmit and recieve */
    U1STASET = 0x1400;
}

void game_init(void)
{
    volatile int *tris_e = (volatile int *)0xbf886100; // initialize pointer to trise (sets output or input)
    *tris_e &= ~0xff;                                  // mask first 8 bits, sets them to output (0)
    TRISD |= (0x7f << 5);                              // sets bits 5-11 to 1 (input)

    PR2 = 31250; // 80000000/256/10 set time-out period
    TMR2 = 0;    // clock should start 0

    T2CON = 0;                       // reset T2CON
    T2CON |= ((1 << 15) | (7 << 4)); // (1<<15) starts the timer, (7<<4) sets prescaling

    IPC(3) |= (0x1f << 24);
    IEC(0) |= (1 << 15);

    IPC(2) |= 0x1f;     // sets priority and subpriority to max
    IEC(0) |= (1 << 8); // enable interrupts for timer2
    enable_interrupt(); // enable global interrupts via MIPS

    // Init uart pins
    uart_init();

    // Init i2c pins
    i2c_init();

    // Initialize game, to level 1
    init_game(&g, 1);

    return;
}

// Send gamestate to computer via uart
void send_gamestate(int gamestate[])
{
    // Send start signal
    send_char('s');

    // Send length of gamestate
    send_int(N);

    // Send each digit in gamestate
    for (int i = 0; i < N; i++)
    {
        send_int(gamestate[i]);
    }
}

// Loop for game
void game_loop()
{
    // Check buttons
    bn_sw_time();

    // Get next gamestate
    next_gamestate(gamestate, &g);

    // Send gamestate to computer via UART
    send_gamestate(gamestate);
}