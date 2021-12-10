#include <stdint.h>
#include <pic32mx.h>

#include "game_main.h"

#include "src/include/game.h"

struct game g;
const int N = 2 * (MAX_GROUND_SIZE * MAX_GROUND_SIZE);
int gamestate[2 * (MAX_GROUND_SIZE * MAX_GROUND_SIZE)];

int ticks = 0;
int timeoutcount = 0;

void user_isr(void)
{
    if (check_timeout() && timeoutcount == 10)
    {
        g.time++;
        timeoutcount = 0;
    }
}

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

int getbtns(void)
{
    return ((PORTD & (7 << 5)) >> 5);
}

int getbtn1(void)
{
    return (PORTF >> 1) & 1;
}

int getsw(void)
{
    return ((PORTD & (0xf << 8)) >> 8);
}

void bn_sw_time(void)
{
    int btn_status = getbtns();
    int btn1_status = getbtn1();
    int sw_status = getsw();
    if (btn_status || btn1_status)
    {
        if (btn_status & 4)
        { // btn4
            // Left
            if (g.curr_button != 4 && g.curr_button != -1)
            {
                g.curr_button = 4;
            }
            else
            {
                g.curr_button = -1;
            }
            if (g.player.is_moving == 0)
            {
                g.player.direction = 0;
                g.player.is_moving = 1;
            }
        }
        else if (btn_status & 2)
        { // btn3
            // Right
            if (g.curr_button != 3 && g.curr_button != -1)
            {
                g.curr_button = 3;
            }
            else
            {
                g.curr_button = -1;
            }
            if (g.player.is_moving == 0)
            {
                g.player.direction = 2;
                g.player.is_moving = 1;
            }
        }
        else if (btn_status & 1)
        { // btn2
            // Up
            if (g.curr_button != 2 && g.curr_button != -1)
            {
                g.curr_button = 2;
            }
            else
            {
                g.curr_button = -1;
            }
            if (g.player.is_moving == 0)
            {
                g.player.direction = 1;
                g.player.is_moving = 1;
            }
        }
        else if (btn1_status)
        {
            // Down
            if (g.curr_button != 1 && g.curr_button != -1)
            {
                g.curr_button = 1;
            }
            else
            {
                g.curr_button = -1;
            }

            if (g.player.is_moving == 0)
            {
                g.player.direction = 3;
                g.player.is_moving = 1;
            }
        }
        else
        {
            g.curr_button = 0;
        }

        if (g.player.is_moving == 0)
        {
            if (sw_status << 12)
            {

                g.player.is_grabbing = 1;
            }
            else
            {
                g.player.is_grabbing = 0;
            }
        }
    }
    else
    {
        g.curr_button = 0;
    }
}

// From mcb32 toolchain github
int calculate_baudrate_divider(int sysclk, int baudrate, int highspeed)
{
    int pbclk, uxbrg, divmult;
    unsigned int pbdiv;

    divmult = (highspeed) ? 4 : 16;
    /* Periphial Bus Clock is divided by PBDIV in OSCCON */
    pbdiv = (OSCCON & 0x180000) >> 19;
    pbclk = sysclk >> pbdiv;

    /* Multiply by two, this way we can round the divider up if needed */
    uxbrg = ((pbclk * 2) / (divmult * baudrate)) - 2;
    /* We'll get closer if we round up */
    if (uxbrg & 1)
        uxbrg >>= 1, uxbrg++;
    else
        uxbrg >>= 1;
    return uxbrg;
}

void i2c_init()
{
    I2C1CON = 0x0;
    /* I2C Baud rate should be less than 400 kHz, is generated by dividing
       the 40 MHz peripheral bus clock down */
    I2C1BRG = 0x0C2;
    I2C1STAT = 0x0;
    I2C1CONSET = 1 << 13; // SIDL = 1
    I2C1CONSET = 1 << 15; // ON = 1
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
    IEC(0) |= (1 << 8); // enable intrrupts for timer2
    enable_interrupt(); // enable global interrupts via MIPS

    /* Configure UART1 for 115200 baud, no interrupts */
    U1BRG = calculate_baudrate_divider(80000000, 115200, 0);
    U1STA = 0;
    /* 8-bit data, no parity, 1 stop bit */
    U1MODE = 0x8000;
    /* Enable transmit and recieve */
    U1STASET = 0x1400;

    i2c_init();

    init_game(&g, 1);

    return;
}

#define ITOA_BUFSIZ (24)
char *itoaconv(int num)
{
    register int i, sign;
    static char itoa_buffer[ITOA_BUFSIZ];
    static const char maxneg[] = "-2147483648";

    itoa_buffer[ITOA_BUFSIZ - 1] = 0; /* Insert the end-of-string marker. */
    sign = num;                       /* Save sign. */
    if (num < 0 && num - 1 > 0)       /* Check for most negative integer */
    {
        for (i = 0; i < sizeof(maxneg); i += 1)
            itoa_buffer[i + 1] = maxneg[i];
        i = 0;
    }
    else
    {
        if (num < 0)
            num = -num;      /* Make number positive. */
        i = ITOA_BUFSIZ - 2; /* Location for first ASCII digit. */
        do
        {
            itoa_buffer[i] = num % 10 + '0'; /* Insert next digit. */
            num = num / 10;                  /* Remove digit from number. */
            i -= 1;                          /* Move index to next empty position. */
        } while (num > 0);
        if (sign < 0)
        {
            itoa_buffer[i] = '-';
            i -= 1;
        }
    }
    /* Since the loop always sets the index i to the next empty position,
     * we must add 1 in order to return a pointer to the first occupied position. */
    return (&itoa_buffer[i + 1]);
}

void send_int(int a)
{
    for (int i = 0; i < 4; i++)
    {
        while (1)
        {
            if (!(U1STA & (1 << 9)))
            {
                U1TXREG = *(itoaconv(a) + i);
                break;
            }
        }
    }
}

void send_char(char a)
{
    while (1)
    {
        if (!(U1STA & (1 << 9)))
        {
            U1TXREG = a;
            break;
        }
    }
}

void send_gamestate(int gamestate[])
{
    send_char('s');
    send_int(N);

    for (int i = 0; i < N; i++)
    {
        send_int(gamestate[i]);
    }
}

// int iteration = 0;
void game_loop()
{
    bn_sw_time();
    next_gamestate(gamestate, &g);
    send_gamestate(gamestate);
    // if (iteration % 1000 == 0)
    // {
    //     next_gamestate(gamestate, &g);
    //     send_gamestate(gamestate);
    //     iteration = 0;
    // }
    // iteration++;
}