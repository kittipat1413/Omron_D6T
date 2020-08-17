import smbus2
import time
import numpy
from smbus2 import i2c_msg
# for RPI version 1, use "bus = smbus.SMBus(0)"
bus = smbus2.SMBus(1)


#SET REGISTER
bus.write_i2c_block_data(0x0a,0x02,[0x00,0x01,0xEE])
bus.write_i2c_block_data(0x0a,0x05,[0x90,0x3A,0xB8])
bus.write_i2c_block_data(0x0a,0x03,[0x00,0x03,0x8B])
bus.write_i2c_block_data(0x0a,0x03,[0x00,0x07,0x97])
bus.write_i2c_block_data(0x0a,0x02,[0x00,0x00,0xE9])



write = i2c_msg.write(0x0a,[0x02])
read  = i2c_msg.read(0x0a, 2)

print("Expected value of 2 byte read is 0x00 and 0x00")
bus.i2c_rdwr(write, read)
for i in range(read.len):
	print(hex(ord(read.buf[i])))

write = i2c_msg.write(0x0a,[0x05])
read  = i2c_msg.read(0x0a, 2)

print("Expected value of 2 byte read is 0x90 and 0x3A")
bus.i2c_rdwr(write, read)
for i in range(read.len):
        print(hex(ord(read.buf[i])))


write = i2c_msg.write(0x0a,[0x03])
read  = i2c_msg.read(0x0a, 2)

print("Expected value of 2 byte read is 0x00 and 0x07")
bus.i2c_rdwr(write, read)
for i in range(read.len):
        print(hex(ord(read.buf[i])))

print("......................................")
print("")

#AT less 750msec
time.sleep(1)

#Read Temperature
write = i2c_msg.write(0x0a,[0x4c])
read  = i2c_msg.read(0x0a, 19)

bus.i2c_rdwr(write, read)

data = [];
for i in range(read.len):
        # print(hex(ord(read.buf[i])))
	data.append(read.buf[i])

PTAT = ((256*int(ord(data[1])))+int(ord(data[0])))/10.0
print("PTAT = {}".format(PTAT))
P0 = ((256*int(ord(data[3])))+int(ord(data[2])))/10.0
print("P0 = {}".format(P0))
P1 = ((256*int(ord(data[5])))+int(ord(data[4])))/10.0
print("P1 = {}".format(P1))
P2 = ((256*int(ord(data[7])))+int(ord(data[6])))/10.0
print("P2 = {}".format(P2))
P3 = ((256*int(ord(data[9])))+int(ord(data[8])))/10.0
print("P3 = {}".format(P3))
P4 = ((256*int(ord(data[11])))+int(ord(data[10])))/10.0
print("P4 = {}".format(P4))
P5 = ((256*int(ord(data[13])))+int(ord(data[12])))/10.0
print("P5 = {}".format(P5))
P6 = ((256*int(ord(data[15])))+int(ord(data[14])))/10.0
print("P6 = {}".format(P6))
P7 = ((256*int(ord(data[17])))+int(ord(data[16])))/10.0
print("P7 = {}".format(P7))

print("PEC = {}".format(hex(ord(data[18]))))



def D6T_checkPEC(data):
        crc = calc_crc(0x14)
        crc = calc_crc(0x4c^crc)
        crc = calc_crc(0x15^crc)
        for i in range(18):
                crc = calc_crc(int(ord(data[i]))^crc)
        print("CRC : {} ".format(hex(crc)))
        return

def calc_crc(data):
        uint8_data = numpy.uint8(data)
        temp = 0
        for i in range(8):
                temp = uint8_data
                uint8_data = numpy.left_shift(uint8_data,1)
                uint8_data = numpy.uint8(uint8_data)
                # uint8_data <<= 1
                if(temp & 0x80):
                        uint8_data ^= 0x07
        return uint8_data

D6T_checkPEC(data)

#End of the Script

