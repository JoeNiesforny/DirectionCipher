#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "spi.h"
#include "adxl345.h"

#define DEVID           0 
#define THRESH_TAP      29
#define OFSX            30
#define OFSY            31
#define OFSZ            32
#define DUR             33
#define LATENT          34
#define WINDOW          35
#define THRESH_ACT      36
#define THRESH_INACT    37
#define TIME_INACT      38
#define ACT_INACT_CTL   39
#define THRESH_FF       40
#define TIME_FF         41
#define TAP_AXES        42
#define ACT_TAP_STATUS  43
#define BW_RATE         44
#define POWER_CTL       45
#define INT_ENABLE      46
#define INT_MAP         47
#define INT_SOURCE      48
#define DATA_FORMAT     49
#define DATAX0          50
#define DATAX1          51
#define DATAY0          52
#define DATAY1          53
#define DATAZ0          54
#define DATAZ1          55
#define FIFO_CTL        56
#define FIFO_STATUS     57

#define DATA_FORMAT_FULL_RES	(1 << 3) /* 13-bit resolution */
#define DATA_FORMAT_2G		(0)
#define DATA_FORMAT_4G		(1 << 0)
#define DATA_FORMAT_8G		(1 << 1)
#define DATA_FORMAT_16G		((1 << 1) | (1 << 0))

#define BW_RATE_100kHz		0xa
#define BW_RATE_200kHz		0xb
#define BW_RATE_400kHz		0xc
#define BW_RATE_800kHz		0xd

#define POWER_CTL_MEASURE_BIT	(1 << 3)

#define ADXL345_DEVICE_ID	0xe5

#define ADD_ACC(a, b) a.x += b.x; a.y += b.y; a.z += b.z;
#define SUB_ACC(a, b) a.x -= b.x; a.y -= b.y; a.z -= b.z;

#define DIV_ACC_BY_CONS(a, b) {	\
		a.x /= b;	\
		a.y /= b;	\
		a.z /= b;	\
	}

#define MUL_ACC_BY_FRAC(a, b, c) { \
		a.x = b * a.x / c; \
		a.y = b * a.y / c; \
		a.z = b * a.z / c; \
	}

#define SHIFT_MUL_ACC(b, c, a) { \
		a.x = (a.x >> b) * c; \
		a.y = (a.y >> b) * c; \
		a.z = (a.z >> b) * c; \
	}

#define SAMPLE_NUMBER	4

static adxl345_acc_t low_pass;
static adxl345_acc_t samples[SAMPLE_NUMBER];
static adxl345_acc_t samples_sum;
static uint8_t sample_counter;

static char read_reg(char reg)
{
	char val;
	spi_start_transmission();
	spi_master_write(reg | 0x80);
	val = spi_master_read();
	spi_end_transmission();
	return val;
}

static void read_mb_reg(char reg, char *data, int count)
{
	spi_start_transmission();
	spi_master_write(reg | 0xc0);
	spi_master_read();
	for (int i = 0; i < count; i++) {
		spi_master_write(0x0);
		data[i] = spi_master_read();
	}
	spi_end_transmission();
}

static void write_reg(char reg, char val)
{
	spi_start_transmission();
	spi_master_write(reg);
	spi_master_write(val);
	spi_end_transmission();
}

int adxl345_enable()
{
	adxl345_acc_t acc;

	if ((char)read_reg(DEVID) != (char)ADXL345_DEVICE_ID)
		return -1;

	write_reg(BW_RATE, BW_RATE_100kHz);
	write_reg(DATA_FORMAT, DATA_FORMAT_16G);
	write_reg(POWER_CTL, POWER_CTL_MEASURE_BIT);

	for (int i = 0; i < SAMPLE_NUMBER; i++) {
		_delay_ms(1);
		adxl345_read(&acc);
	}

	return 0;
}

void adxl345_disable()
{
	write_reg(POWER_CTL, 0);
}

char adxl345_get(char reg)
{
	return read_reg(reg);
}

static void low_pass_filter(adxl345_acc_t *acc)
{
	/* acc = 5 * acc / 16 */
	SHIFT_MUL_ACC(4, 0x4, acc[0]);
	/* low = 11 * low / 16 */
	SHIFT_MUL_ACC(4, 0xc, low_pass);
	ADD_ACC(low_pass, acc[0]);

	*acc = low_pass;
}

static void running_avg(adxl345_acc_t *acc)
{
	SUB_ACC(samples_sum, samples[sample_counter]);
	ADD_ACC(samples_sum, acc[0]);
	samples[sample_counter] = *acc;

	sample_counter++;
	if (sample_counter == SAMPLE_NUMBER)
		sample_counter = 0;

	*acc = samples_sum;
	DIV_ACC_BY_CONS(acc[0], SAMPLE_NUMBER);
}

adxl345_acc_t acc_offset = {
	.x = 0,
	.y = 256,
	.z = 0
};

void adxl345_read(adxl345_acc_t *acc)
{
	char data[6];

	read_mb_reg(DATAX0, data, sizeof(data));
	acc->x = data[0] | (data[1] << 8);
	acc->y = data[2] | (data[3] << 8);
	acc->z = data[4] | (data[5] << 8);

#ifndef DEBUG_ADXL345
	//low_pass_filter(acc);
	running_avg(acc);
	//ADD_ACC(acc[0], acc_offset);
	//DIV_ACC_BY_CONS(acc[0], 4096);
#else
	if (false) {
	acc->x &= -1985;
	acc->y &= -1985;
	acc->z &= -1985;
	}
#endif
}
