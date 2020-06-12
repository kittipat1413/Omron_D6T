/* includes */
#include <omron_D6T8L09.h>

uint32_t i2c_read_reg8(uint8_t devAddr, uint8_t regAddr,uint8_t *data, int length) 
{
    int fd = open(I2CDEV, O_RDWR);

    if (fd < 0) {
        //fprintf(stderr, "Failed to open device: %s\n", strerror(errno));
        return -21;
    }
    int err = 0;
    do {
        if (ioctl(fd, I2C_SLAVE, devAddr) < 0) {
            //fprintf(stderr, "Failed to select device: %s\n", strerror(errno));
            err = -22; break;
        }
        if (write(fd, &regAddr, 1) != 1) {
            //fprintf(stderr, "Failed to write reg: %s\n", strerror(errno));
            err = -23; break;
        }
        int count = read(fd, data, length);
        if (count < 0) {
            //fprintf(stderr, "Failed to read device(%d): %s\n",count, strerror(errno));
            err = -24; break;
        } else if (count != length) {
            //fprintf(stderr, "Short read  from device, expected %d, got %d\n",length, count);
            err = -25; break;
        }
    } while (false);
    close(fd);
    return err;
}

uint32_t i2c_write_reg8(uint8_t devAddr,uint8_t *data, int length) 
{
    int fd = open(I2CDEV, O_RDWR);
    if (fd < 0) {
        //fprintf(stderr, "Failed to open device: %s\n", strerror(errno));
        return -21;
    }
    int err = 0;
    do {
        if (ioctl(fd, I2C_SLAVE, devAddr) < 0) {
            //fprintf(stderr, "Failed to select device: %s\n", strerror(errno));
            err = -22; break;
        }
        if (write(fd, data, length) != length) {
            //fprintf(stderr, "Failed to write reg: %s\n", strerror(errno));
            err = -23; break;
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
    //if (ret) {
        //fprintf(stderr,"PEC check failed: %02X(cal)-%02X(get)\n", crc, buf[n]);
    //}
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



int16_t D6T8L09_Read(int16_t *ret_array) {
    uint8_t i, j;
    uint8_t rbuf[N_READ];
    uint32_t ret;

    uint8_t dat1[] = {0x02, 0x00, 0x01, 0xee};
    ret = i2c_write_reg8(D6T_ADDR, dat1, sizeof(dat1));
    if (ret < 0) {
        return ret;
    }
    uint8_t dat2[] = {0x05, 0x90, 0x3a, 0xb8};
    ret = i2c_write_reg8(D6T_ADDR, dat2, sizeof(dat2));
    if (ret < 0) {
        return ret;
    }
    uint8_t dat3[] = {0x03, 0x00, 0x03, 0x8b};
    ret = i2c_write_reg8(D6T_ADDR, dat3, sizeof(dat3));
    if (ret < 0) {
        return ret;
    }
    uint8_t dat4[] = {0x03, 0x00, 0x07, 0x97};
    ret = i2c_write_reg8(D6T_ADDR, dat4, sizeof(dat4));
    if (ret < 0) {
        return ret;
    }
    uint8_t dat5[] = {0x02, 0x00, 0x00, 0xe9};
    ret = i2c_write_reg8(D6T_ADDR, dat5, sizeof(dat5));
    if (ret < 0) {
        return ret;
    }
    delay(500);

    memset(rbuf, 0, N_READ);
    ret = i2c_read_reg8(D6T_ADDR, D6T_CMD, rbuf, N_READ);
    if (ret < 0) {
        return ret;
    }

    if (D6T_checkPEC(rbuf, N_READ - 1)) {
        return -20;
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

float D6T8L09_GetTemp(int Sampling){
    int16_t Max_temp = 0;
    int16_t face_temp[2];

    for (int i=0;i<=Sampling;i++){
        //printf("Sampling : %d \n", i); 
        if(D6T8L09_Read(face_temp)){
            if (face_temp[0] >  Max_temp){
                  Max_temp = face_temp[0];
            }
        }
        else{
              //printf("\n\nError : %d \n\n\n",  face_temp);
              return 0;
        }
    }
    float Offset = 8.50444606 + -0.021150108*face_temp[1];
    /*Offset face to body temp*/
    return((float)(Max_temp/10.0)+Offset); 


}
