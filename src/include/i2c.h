#ifndef __I2C_H_
#define __I2C_H_

#include <pic32mx.h>
#include <stdint.h>

#define EEPROM_WRITE 0xA0
#define EEPROM_READ 0xA1
#define EEPROM_MEM_ADDR 0x0000

void i2c_idle();
int i2c_send(uint8_t data);
uint8_t i2c_recv();
void i2c_ack();
void i2c_nack();
void i2c_start();
void i2c_stop();
void i2c_restart();

void i2c_send_char(int location, char c);
char i2c_recv_char(int location);
void i2c_send_int(int location, int n);
int i2c_recv_int(int location);

#endif