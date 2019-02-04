
#pragma once

#include "adxl345.h"

enum directions {
	UNDEFINED = 0,
	X_UP,
	X_DOWN,
	Y_UP,
	Y_DOWN,
	Z_UP,
	Z_DOWN
};

char *direction_to_string(uint8_t dir);
bool decode_direction_cipher_next(uint8_t dir, uint8_t *p_guess);
uint8_t check_direction(adxl345_acc_t acc);
