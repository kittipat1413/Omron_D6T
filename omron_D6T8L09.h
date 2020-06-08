#ifndef __D6T8L09_H__
#define __D6T8L09_H__
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include <time.h>


/* defines */
#define D6T_ADDR 0x0A  // for I2C 7bit address
#define D6T_CMD 0x4C  // for D6T-44L-06/06H, D6T-8L-09/09H, for D6T-1A-01/02

#define N_ROW 8
#define N_PIXEL 8

#define N_READ ((N_PIXEL + 1) * 2 + 1) 

#define I2C1    "/dev/i2c-1"
#define I2CDEV  I2C1


uint32_t i2c_read_reg8(uint8_t devAddr, uint8_t regAddr,uint8_t *data, int length); 
uint32_t i2c_write_reg8(uint8_t devAddr,uint8_t *data, int length); 
int16_t conv8us_s16_le(uint8_t* buf, int n);

uint8_t calc_crc(uint8_t data);
bool D6T_checkPEC(uint8_t buf[], int n);
void delay(int msec);

//Return Max value from 8 pixel 
/*The output will be negative if some problem occurs.
-20 = "Crc Error"
-21 = "Failed to open device"
-22 = "Failed to select device"
-23 = "Failed to write reg"
-24 = "Failed to read device"
-25 = "Short read  from device"
*/ 
int16_t D6T8L09_Read(void);






#endif // __D6T8L09_H__