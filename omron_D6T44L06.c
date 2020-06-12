/*
 * MIT License
 * Copyright (c) 2019, 2018 - present OMRON Corporation
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* includes */
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

#define N_ROW 4
#define N_PIXEL (4 * 4)

#define N_READ ((N_PIXEL + 1) * 2 + 1)
uint8_t rbuf[N_READ];

#define RASPBERRY_PI_I2C    "/dev/i2c-1"
#define I2CDEV              RASPBERRY_PI_I2C


/* I2C functions */
/** <!-- i2c_read_reg8 {{{1 --> I2C read function for bytes transfer.
 */
uint32_t i2c_read_reg8(uint8_t devAddr, uint8_t regAddr,
                       uint8_t *data, int length
) {
    int fd = open(I2CDEV, O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "Failed to open device: %s\n", strerror(errno));
        return 21;
    }
    int err = 0;
    do {
        if (ioctl(fd, I2C_SLAVE, devAddr) < 0) {
            fprintf(stderr, "Failed to select device: %s\n", strerror(errno));
            err = 22; break;
        }
        if (write(fd, &regAddr, 1) != 1) {
            err = 23; break;
        }
        int count = read(fd, data, length);
        if (count < 0) {
            err = 24; break;
        } else if (count != length) {
            fprintf(stderr, "Short read  from device, expected %d, got %d\n",
                    length, count);
            err = 25; break;
        }
    } while (false);
    close(fd);
    return err;
}


uint8_t calc_crc(uint8_t data) {
    int index;
    uint8_t temp;
    for (index = 0; index < 8; index++) {
        temp = data;
        data <<= 1;
        if (temp & 0x80) {data ^= 0x07;}
    }
    return data;
}

/** <!-- D6T_checkPEC {{{ 1--> D6T PEC(Packet Error Check) calculation.
 * calculate the data sequence,
 * from an I2C Read client address (8bit) to thermal data end.
 */
bool D6T_checkPEC(uint8_t buf[], int n) {
    int i;
    uint8_t crc = calc_crc((D6T_ADDR << 1) | 1);  // I2C Read address (8bit)
    for (i = 0; i < n; i++) {
        crc = calc_crc(buf[i] ^ crc);
    }
    bool ret = crc != buf[n];
    if (ret) {
        fprintf(stderr,
                "PEC check failed: %02X(cal)-%02X(get)\n", crc, buf[n]);
    }
    return ret;
}


/** <!-- conv8us_s16_le {{{1 --> convert a 16bit data from the byte stream.
 */
int16_t conv8us_s16_le(uint8_t* buf, int n) {
    int ret;
    ret = buf[n];
    ret += buf[n + 1] << 8;
    return (int16_t)ret;   // and convert negative.
}


void delay(int msec) {
    struct timespec ts = {.tv_sec = msec / 1000,
                          .tv_nsec = (msec % 1000) * 1000000};
    nanosleep(&ts, NULL);
}


int16_t D6T44L06_Read(int16_t *ret_array) {
    uint8_t i, j;
    uint8_t rbuf[N_READ];

    memset(rbuf, 0, N_READ);
    for (i = 0; i < 10; i++) {
        uint32_t ret = i2c_read_reg8(D6T_ADDR, D6T_CMD, rbuf, N_READ);
        if (ret == 0) {
            break;
        } else if (ret == 23) {  // write error
            delay(60);
        } else if (ret == 24) {  // read error
            delay(3000);
        }
    }
    if (i >= 10) {
        fprintf(stderr, "Failed to read/write: %s\n", strerror(errno));
        return 0;
    }
    if (D6T_checkPEC(rbuf, N_READ - 1)) {
        return 0;
    }
    
    int16_t itemp;
    int16_t ptat;
    int16_t MAX_Sampling_temp;
    
    //PTAT measurement 
    ptat = conv8us_s16_le(rbuf, 0);
    ret_array[1] = ptat;
    //printf("PTAT: %6.1f[degC]\n", itemp / 10.0);



    //Read first Pixel
    itemp = conv8us_s16_le(rbuf, 2);
    //printf("%4.1f,", itemp / 10.0); 
    MAX_Sampling_temp = itemp;

    //Read remaining pixel and find max value
    for (i = 1, j = 4; i < N_PIXEL; i++, j += 2) {
        itemp = conv8us_s16_le(rbuf, j); 
        //printf("%4.1f", itemp / 10.0);  // print PTAT & Temperature
        if ((i % N_ROW) == N_ROW - 1) {
            //printf(" [degC]\n");  // wrap text at ROW end.
        } else {
            //printf(",");   // print delimiter
        }
        if(itemp > MAX_Sampling_temp){
             MAX_Sampling_temp = itemp;
        }
    }
    //printf("MAX_Sampling_temp : %4.1f \n", MAX_Sampling_temp / 10.0);
    ret_array[0] = MAX_Sampling_temp;
    return 1;
}

float D6T44L06_GetTemp(int Sampling){
    int16_t Max_temp = 0;
    int16_t face_temp[2];

    for (int i=0;i<=Sampling;i++){
        //printf("Sampling : %d \n", i); 
        if(D6T44L06_Read(face_temp)){
            if (face_temp[0] >  Max_temp){
                  Max_temp = face_temp[0];
            }
        }
        else{
              //printf("\n\nError : %d \n\n\n",  face_temp);
              return 0;
        }
    }
    float Offset = 8.35881937 + -0.020313201*face_temp[1];
    /*Offset face to body temp*/
    return((float)(Max_temp/10.0)+Offset); 


}


/** <!-- main - Thermal sensor {{{1 -->
 * 1. read sensor.
 * 2. output results, format is: [degC]
 */
int main() {
   
    int Sampling = 3;
    float temp;

 		
 	temp = D6T44L06_GetTemp(Sampling); 
 	printf("\n\nBody Temp : %4.1f \n\n\n", temp); 



    // memset(rbuf, 0, N_READ);
    // for (i = 0; i < 10; i++) {
    //     uint32_t ret = i2c_read_reg8(D6T_ADDR, D6T_CMD, rbuf, N_READ);
    //     if (ret == 0) {
    //         break;
    //     } else if (ret == 23) {  // write error
    //         delay(60);
    //     } else if (ret == 24) {  // read error
    //         delay(3000);
    //     }
    // }
    // if (i >= 10) {
    //     fprintf(stderr, "Failed to read/write: %s\n", strerror(errno));
    //     return 1;
    // }

    // if (D6T_checkPEC(rbuf, N_READ - 1)) {
    //     return 2;
    // }

    // // 1st data is PTAT measurement (: Proportional To Absolute Temperature)
    // int16_t itemp = conv8us_s16_le(rbuf, 0);
    // printf("PTAT: %6.1f[degC]\n", itemp / 10.0);

    // // loop temperature pixels of each thrmopiles measurements
    // for (i = 0, j = 2; i < N_PIXEL; i++, j += 2) {
    //     itemp = conv8us_s16_le(rbuf, j);
    //     printf("%4.1f", itemp / 10.0);  // print PTAT & Temperature
    //     if ((i % N_ROW) == N_ROW - 1) {
    //         printf(" [degC]\n");  // wrap text at ROW end.
    //     } else {
    //         printf(",");   // print delimiter
    //     }
    // }
    return 0;
}
