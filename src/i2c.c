#include "include/i2c.h"
#include <stdint.h>

/* Wait for I2C bus to become idle */
void i2c_idle()
{
    while (I2C1CON & 0x1F || I2C1STAT & (1 << 14))
        ; // TRSTAT
}

/* Send one byte on I2C bus, return ack/nack status of transaction */
int i2c_send(uint8_t data)
{
    i2c_idle();
    I2C1TRN = data;
    i2c_idle();
    return !(I2C1STAT & (1 << 15)); // ACKSTAT
}

/* Receive one byte from I2C bus */
uint8_t i2c_recv()
{
    i2c_idle();
    I2C1CONSET = 1 << 3; // RCEN = 1
    i2c_idle();
    I2C1STATCLR = 1 << 6; // I2COV = 0
    return I2C1RCV;
}

/* Send acknowledge conditon on the bus */
void i2c_ack()
{
    i2c_idle();
    I2C1CONCLR = 1 << 5; // ACKDT = 0
    I2C1CONSET = 1 << 4; // ACKEN = 1
}

/* Send not-acknowledge conditon on the bus */
void i2c_nack()
{
    i2c_idle();
    I2C1CONSET = 1 << 5; // ACKDT = 1
    I2C1CONSET = 1 << 4; // ACKEN = 1
}

/* Send start conditon on the bus */
void i2c_start()
{
    i2c_idle();
    I2C1CONSET = 1 << 0; // SEN
    i2c_idle();
}

/* Send restart conditon on the bus */
void i2c_restart()
{
    i2c_idle();
    I2C1CONSET = 1 << 1; // RSEN
    i2c_idle();
}

/* Send stop conditon on the bus */
void i2c_stop()
{
    i2c_idle();
    I2C1CONSET = 1 << 2; // PEN
    i2c_idle();
}

// Send char to eeprom at location
void i2c_send_char(int location, char c)
{
    do
    {
        i2c_start();
    } while (!i2c_send(EEPROM_WRITE));

    i2c_send(EEPROM_MEM_ADDR >> 8);
    i2c_send(EEPROM_MEM_ADDR + location);
    i2c_send(c);
    i2c_stop();
}

// Receive char from eeprom
char i2c_recv_char(int location)
{
    // Set location to read from
    do
    {
        i2c_start();
    } while (!i2c_send(EEPROM_WRITE));

    i2c_send(EEPROM_MEM_ADDR >> 8);
    i2c_send(EEPROM_MEM_ADDR + location);

    // Recieve char
    i2c_start();
    i2c_send(EEPROM_READ);

    char out = i2c_recv();
    i2c_nack();
    i2c_stop();

    return out;
}

// Sends an int (not an actual int, only 4 digits) to eeprom at specified location
void i2c_send_int(int location, int n)
{
    // We only want the first 4 digits
    int d = 1000;
    n = n % (d * 10);

    // Loop through one time for each digit in n
    for (int i = 0; i < 4; i++)
    {
        // Send digit at index i to eeprom
        i2c_send_char(location + i, n / d);
        n = n % d;
        d /= 10;
    }
}

// Reads an int (not an actual int, only 4 digits) from eeprom at specified location
int i2c_recv_int(int location)
{
    int d = 1000;
    int out = 0;

    // Loop through 4 times to get the digit, also multiply it by its a power of 10 that matches its position
    for (int i = 0; i < 4; i++)
    {
        out += i2c_recv_char(location + i) * d;
        d /= 10;
    }

    return out;
}