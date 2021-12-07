#include <stdint.h>
#include <pic32mx.h>

#include "game.h"

#include "src/player.h"

struct player p = create_player(10, 10);

void user_isr(void)
{
    //
}

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
    // enable_interrupt(); // enable global interrupts via MIPS

    /* Configure UART1 for 115200 baud, no interrupts */
    U1BRG = calculate_baudrate_divider(80000000, 115200, 0);
    U1STA = 0;
    /* 8-bit data, no parity, 1 stop bit */
    U1MODE = 0x8000;
    /* Enable transmit and recieve */
    U1STASET = 0x1400;

    return;
}

void game_loop()
{
    if (!(U1STA & (1 << 9)))
    {
        U1TXREG = p.x;
    }
}