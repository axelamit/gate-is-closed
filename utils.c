#include "utils.h"

#include <pic32mx.h>

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