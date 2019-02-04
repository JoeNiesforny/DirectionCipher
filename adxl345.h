
#pragma once

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} __attribute__ ((packed)) adxl345_acc_t;

char adxl345_get(char reg);
char adxl345_get_id();
int adxl345_enable();
void adxl345_disable();
void adxl345_read(adxl345_acc_t *acc);
