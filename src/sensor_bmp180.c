/*
 * The BMP180 driver code was downloaded from
 * https://github.com/lexruee/bmp180.
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Alexander Rüedlinger <a.rueedlinger@gmail.com>
 * Copyright (C) 2016 Brian Masney <masneyb@onstation.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <wiringPiI2C.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include "yadl.h"

// pressure oversampling modes
#define BMP180_PRESSURE_OSS0 0 // ultra low power
#define BMP180_PRESSURE_OSS1 1 // standard
#define BMP180_PRESSURE_OSS2 2 // high resolution
#define BMP180_PRESSURE_OSS3 3 // ultra high resoultion

// Registers
#define BMP180_REGISTER_AC1_H 0xAA
#define BMP180_REGISTER_AC2_H 0xAC
#define BMP180_REGISTER_AC3_H 0xAE
#define BMP180_REGISTER_AC4_H 0xB0
#define BMP180_REGISTER_AC5_H 0xB2
#define BMP180_REGISTER_AC6_H 0xB4
#define BMP180_REGISTER_B1_H 0xB6
#define BMP180_REGISTER_B2_H 0xB8
#define BMP180_REGISTER_MB_H 0xBA
#define BMP180_REGISTER_MC_H 0xBC
#define BMP180_REGISTER_MD_H 0xBE
#define BMP180_REGISTER_TEMPERATURE 0xF6
#define BMP180_REGISTER_PRESSURE 0xF6

#define BMP180_CTRL 0xF4

#define BMP180_TEMPERATURE_READ_CMD 0x2E
#define BMP180_TEMPERATURE_READ_WAIT_US 5000

// Pressure oversampling modes
#define BMP180_PRESSURE_OSS0 0 // ultra low power
#define BMP180_PRESSURE_OSS1 1 // standard
#define BMP180_PRESSURE_OSS2 2 // high resolution
#define BMP180_PRESSURE_OSS3 3 // ultra high resoultion

// Pressure read commands
#define BMP180_PRESSURE_OSS0_CMD 0x34
#define BMP180_PRESSURE_OSS1_CMD 0x74
#define BMP180_PRESSURE_OSS2_CMD 0xB4
#define BMP180_PRESSURE_OSS3_CMD 0xF4

// Waiting times in us for reading pressure values
#define BMP180_PRESSURE_OSS0_WAIT_US 5000
#define BMP180_PRESSURE_OSS1_WAIT_US 8000
#define BMP180_PRESSURE_OSS2_WAIT_US 14000
#define BMP180_PRESSURE_OSS3_WAIT_US 26000

#define BMP180_AVG_PRESSURE_AT_SEA_LEVEL_IN_HPA 1013.25

// Basic structure for the bmp180 sensor
typedef struct {
	int fd;

	/* BMP180 oversampling mode */
	int oss;
	
	/* Eprom values */
	int32_t ac1;
	int32_t ac2;
	int32_t ac3;
	int32_t ac4;
	int32_t ac5;
	int32_t ac6;
	int32_t b1;
	int32_t b2;
	int32_t mb;
	int32_t mc;
	int32_t md;

	/* Last errno */
	int error;
} bmp180_t;

// Lookup table for BMP180 register addresses
static int32_t bmp180_register_table[11][2] = {
		{BMP180_REGISTER_AC1_H, 1},
		{BMP180_REGISTER_AC2_H, 1},
		{BMP180_REGISTER_AC3_H, 1},
		{BMP180_REGISTER_AC4_H, 0},
		{BMP180_REGISTER_AC5_H, 0},
		{BMP180_REGISTER_AC6_H, 0},
		{BMP180_REGISTER_B1_H, 1},
		{BMP180_REGISTER_B2_H, 1},
		{BMP180_REGISTER_MB_H, 1},
		{BMP180_REGISTER_MC_H, 1},
		{BMP180_REGISTER_MD_H, 1}
};


// Reads the eprom of this BMP180 sensor.
static void bmp180_read_eprom(bmp180_t *bmp)
{
	int32_t *bmp180_register_addr[11] = {
		&bmp->ac1, &bmp->ac2, &bmp->ac3, &bmp->ac4, &bmp->ac5, &bmp->ac6,
		&bmp->b1, &bmp->b2, &bmp->mb, &bmp->mc, &bmp->md
	};
	
	for(int i = 0; i < 11; i++) {
		uint8_t reg = (uint8_t) bmp180_register_table[i][0];
		uint8_t sign = (uint8_t) bmp180_register_table[i][1];

		int32_t data = wiringPiI2CReadReg16(bmp->fd, reg) & 0xFFFF;
		*(bmp180_register_addr[i]) = ((data << 8) & 0xFF00) + (data >> 8);
		if (sign && (*(bmp180_register_addr[i]) > 32767)) {
			*(bmp180_register_addr[i]) -= 65536;
		}
	}
}

// Returns the raw measured temperature value of this BMP180 sensor.
static int32_t bmp180_read_raw_temperature(bmp180_t *bmp)
{
	wiringPiI2CWriteReg8(bmp->fd, BMP180_CTRL, BMP180_TEMPERATURE_READ_CMD);

	usleep(BMP180_TEMPERATURE_READ_WAIT_US);

	int32_t data = wiringPiI2CReadReg16(bmp->fd, BMP180_REGISTER_TEMPERATURE) & 0xFFFF;
	data = ((data << 8) & 0xFF00) + (data >> 8);
	return data;
}


// Returns the raw measured pressure value of this BMP180 sensor.
static int32_t bmp180_read_raw_pressure(bmp180_t *bmp, uint8_t oss)
{
	uint16_t wait;
	uint8_t cmd;
	
	switch(oss) {
		case BMP180_PRESSURE_OSS1:
			wait = BMP180_PRESSURE_OSS1_WAIT_US; cmd = BMP180_PRESSURE_OSS1_CMD;
			break;
		case BMP180_PRESSURE_OSS2:
			wait = BMP180_PRESSURE_OSS2_WAIT_US; cmd = BMP180_PRESSURE_OSS2_CMD;
			break;
		case BMP180_PRESSURE_OSS3:
			wait = BMP180_PRESSURE_OSS3_WAIT_US; cmd = BMP180_PRESSURE_OSS3_CMD;
			break;
		case BMP180_PRESSURE_OSS0:
		default:
			wait = BMP180_PRESSURE_OSS0_WAIT_US; cmd = BMP180_PRESSURE_OSS0_CMD;
			break;
	}
	
	wiringPiI2CWriteReg8(bmp->fd, BMP180_CTRL, cmd);

	usleep(wait);
	
	int32_t msb, lsb, xlsb;
	msb = wiringPiI2CReadReg8(bmp->fd, BMP180_REGISTER_PRESSURE) & 0xFF;
	lsb = wiringPiI2CReadReg8(bmp->fd, BMP180_REGISTER_PRESSURE+1) & 0xFF;
	xlsb = wiringPiI2CReadReg8(bmp->fd, BMP180_REGISTER_PRESSURE+2) & 0xFF;
	
	return ((msb << 16)  + (lsb << 8)  + xlsb) >> (8 - bmp->oss);
}

// Returns the measured temperature in celsius.
static float bmp180_temperature(bmp180_t *bmp)
{
	long UT, X1, X2, B5;
	float T;
	
	UT = bmp180_read_raw_temperature(bmp);
	if (bmp->error != 0) {
		return -FLT_MAX; // error
	}
	
	X1 = ((UT - bmp->ac6) * bmp->ac5) >> 15;
	X2 = (bmp->mc << 11) / (X1 + bmp->md);
	B5 = X1 + X2;
	T = ((B5 + 8) >> 4) / 10.0;
	
	return T;
}


// Returns the measured pressure in pascal.
static long bmp180_pressure(bmp180_t *bmp)
{
	long UT, UP, B6, B5, X1, X2, X3, B3, p;
	unsigned long B4, B7;
	
	UT = bmp180_read_raw_temperature(bmp);
	if (bmp->error != 0) {
		return LONG_MIN; // error
	}

	UP = bmp180_read_raw_pressure(bmp, bmp->oss);
	if (bmp->error != 0) {
		return LONG_MIN; // error
	}
	
	X1 = ((UT - bmp->ac6) * bmp->ac5) >> 15;
	X2 = (bmp->mc << 11) / (X1 + bmp->md);
	
	B5 = X1 + X2;
	
	B6 = B5 - 4000;
	
	X1 = (bmp->b2 * (B6 * B6) >> 12) >> 11;
	X2 = (bmp->ac2 * B6) >> 11;
	X3 = X1 + X2;
	
	B3 = ((((bmp->ac1 * 4) + X3) << bmp->oss) + 2) / 4;
	X1 = (bmp->ac3 * B6) >> 13;
	X2 = (bmp->b1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	
	
	B4 = bmp->ac4 * (unsigned long)(X3 + 32768) >> 15;
	B7 = ((unsigned long) UP - B3) * (50000 >> bmp->oss);
	
	if(B7 < 0x80000000) {
		p = (B7 * 2) / B4;
	} else {
		p = (B7 / B4) * 2;
	}
	
	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;
	p = p + ((X1 + X2 + 3791) >> 4);
	
	return p;
}

// Returns altitude in meters based on the measured pressure 
// and temperature of this sensor.
static float bmp180_altitude(bmp180_t *bmp)
{
	float p, alt;

	p = bmp180_pressure(bmp);
	if (bmp->error != 0) {
		return -FLT_MAX; // error
	}
	alt = 44330 * (1 - pow(( (p/100) / BMP180_AVG_PRESSURE_AT_SEA_LEVEL_IN_HPA),1/5.255));
	
	return alt;
}

static void _bmp180_init(yadl_config *config)
{
	if (config->temperature_converter == NULL) {
		fprintf(stderr, "You must specify the --temperature_unit argument\n");
		usage();
	}
	else if (config->i2c_address == -1) {
		fprintf(stderr, "You must specify the --i2c_address argument\n");
		usage();
	}

	config->fd = wiringPiI2CSetup(config->i2c_address);
	if(config->fd < 0) {
		fprintf(stderr, "i2c device not found at address %x\n", config->i2c_address);
		usage();
	}

}

static yadl_result *_bmp180_read_data(yadl_config *config)
{
        bmp180_t bmp;

	memset(&bmp, 0, sizeof(bmp));
        bmp.fd = config->fd;
        bmp.oss = BMP180_PRESSURE_OSS3;
        bmp.error = 0;
        bmp180_read_eprom(&bmp);

	float temperature = bmp180_temperature(&bmp);
	if (bmp.error != 0) {
		return NULL;
	}

	float pressure = bmp180_pressure(&bmp) / 100.0;
	if (bmp.error != 0) {
		return NULL;
	}

	float altitude = bmp180_altitude(&bmp);
	if (bmp.error != 0) {
		return NULL;
	}

	yadl_result *result = malloc(sizeof(*result));

	result->value = malloc(sizeof(float) * 4);
	result->value[0] = config->temperature_converter(temperature);
	result->value[1] = pressure;
	result->value[2] = pressure * 0.0295301;
	result->value[3] = altitude;

	result->unit = malloc(sizeof(char *) * 1);
	result->unit[0] = config->temperature_unit;

	return result;
}

static char * _bmp180_value_header_names[] = { "temperature", "pressure_millibars", "pressure_in", "altitude", NULL };

static char ** _bmp180_get_value_header_names(__attribute__((__unused__)) yadl_config *config)
{
	return _bmp180_value_header_names;
}

static char * _bmp180_unit_header_names[] = { "temperature_unit", NULL };

static char ** _bmp180_get_unit_header_names(__attribute__((__unused__)) yadl_config *config)
{
	return _bmp180_unit_header_names;
}

sensor bmp180_sensor_funcs = {
	.init = &_bmp180_init,
	.get_value_header_names = &_bmp180_get_value_header_names,
	.get_unit_header_names = &_bmp180_get_unit_header_names,
	.read = _bmp180_read_data


};

