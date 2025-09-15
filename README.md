# Omron D6T Thermal Sensor Library for Raspberry Pi

A comprehensive C and Python library for interfacing with Omron D6T series thermal sensors on Raspberry Pi via I2C communication.


## 📁 Project Structure

```
├── omron_D6T8L09.h         # Header file for D6T-8L-09 sensor functions
├── omron_D6T8L09.c         # Main library implementation for D6T-8L-09
├── test_read.c             # Simple test application for D6T-8L-09
├── i2c.c                   # Standalone I2C example for D6T-8L-09
├── i2c.py                  # Python implementation for sensor communication
├── linear regression.ipynb # Data analysis and temperature calibration
├── DATA.xlsx               # Calibration data for temperature compensation
├── test_data.txt           # Test measurements and calibration notes
└── en_D6T_users_manual.pdf # Official Omron user manual
```

## 🚀 Quick Start

### Prerequisites
- Raspberry Pi with I2C enabled
- GCC compiler for C programs
- Python 3 with smbus2 library for Python examples

### Building C Programs

```bash
gcc -lm -Wall -I . -o test_read omron_D6T8L09.c test_read.c
./test_read
```

## 📊 Features

### Core Functionality
- **Raw Temperature Reading**: Read PTAT (Proportional To Absolute Temperature) and pixel temperatures
- **Body Temperature Estimation**: Automatic compensation from face temperature to body temperature
- **Error Handling**: Comprehensive error codes and PEC (Packet Error Check) validation
- **Multi-sampling**: Configurable sampling for improved accuracy

## 🔍 API Reference

### C Library Functions (D6T-8L-09)

#### `int16_t D6T8L09_Read(int16_t *ret_array)`
Reads temperature data from the 8-pixel sensor.
- **Parameters**: `ret_array[2]` - Array to store [max_temp, PTAT]
- **Returns**: 1 on success, negative error code on failure

#### `float D6T8L09_GetTemp(int Sampling)`
Gets compensated body temperature with multiple sampling.
- **Parameters**: `Sampling` - Number of samples to average
- **Returns**: Body temperature in Celsius, 0 on error

### Error Codes
- `-20`: CRC Error
- `-21`: Failed to open I2C device
- `-22`: Failed to select I2C device
- `-23`: Failed to write to register
- `-24`: Failed to read from device
- `-25`: Short read from device

## 📈 Calibration Data

The project includes extensive calibration data (`test_data.txt` and `DATA.xlsx`) showing:
- Real body temperature vs. sensor readings
- Ambient temperature effects
- Offset calculations for different environmental conditions

## 🔬 Data Analysis

The included Jupyter notebook ([`linear regression.ipynb`](./linear%20regression.ipynb)) provides:
- Statistical analysis of temperature correlations
- Linear regression modeling for temperature compensation
- 3D visualization of temperature relationships
- Pearson correlation analysis