#include "uart.h"
#include "utils.h"

#include <pic32mx.h>

// Send integer via uart (one digit at a time)
void send_int(int a)
{
    for (int i = 0; i < 4; i++)
    {
        while (1)
        {
            // Wait until clear to write
            if (!(U1STA & (1 << 9)))
            {
                U1TXREG = *(itoaconv(a) + i);
                break;
            }
        }
    }
}

// Send char via UART
void send_char(char a)
{
    while (1)
    {
        // Wait until clear to write
        if (!(U1STA & (1 << 9)))
        {
            U1TXREG = a;
            break;
        }
    }
}