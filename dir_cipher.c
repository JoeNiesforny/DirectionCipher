#include <stdio.h>
#include <stdbool.h>

#include "dir_cipher.h"

#define BOUNDARY(a)	(a >= -5 && a <= 5)
#define UP_LIMIT	10
#define DOWN_LIMIT	(-10)

char *direction_to_string(uint8_t dir)
{
	switch (dir) {
	case X_UP:
		return "X_UP";
	case X_DOWN:
		return "X_DOWN";
	case Y_UP:
		return "Y_UP";
	case Y_DOWN:
		return "Y_DOWN";
	case Z_UP:
		return "Z_UP";
	case Z_DOWN:
		return "Z_DOWN";
	default:
		return "UNDEFINED";
	}
}

static uint8_t get_direction(adxl345_acc_t acc)
{
	if (BOUNDARY(acc.z)) {
		if (BOUNDARY(acc.y)) {
			if (acc.x > UP_LIMIT)
				return X_UP;
			if (acc.x < DOWN_LIMIT)
				return X_DOWN;
		}
		if (BOUNDARY(acc.x)) {
			if (acc.y > UP_LIMIT)
				return Y_UP;
			if (acc.y < DOWN_LIMIT)
				return Y_DOWN;
		}
	} else {
		if (BOUNDARY(acc.x) && BOUNDARY(acc.y)) {
			if (acc.z > UP_LIMIT)
				return Z_UP;
			if (acc.z < DOWN_LIMIT)
				return Z_DOWN;
		}
	}
	return UNDEFINED;
}

uint8_t check_direction(adxl345_acc_t acc)
{
	static uint8_t old_dir = 0;
	static uint16_t counter = 0;
	uint8_t dir = get_direction(acc);

	if (dir == UNDEFINED)
		return UNDEFINED;

	if (old_dir != dir) {
		if (counter > 256) {
			old_dir = dir;
			counter = 0;

			return old_dir;
		}
		counter++;
	}

	return UNDEFINED;
}

/* Private key, not for public usage */
uint8_t g_direction_cipher[] = {X_UP, Y_UP, Z_DOWN, X_DOWN, Y_DOWN, Z_UP};

bool decode_direction_cipher_next(uint8_t dir, uint8_t *p_guess)
{
	static uint8_t guess = 0;

	if (guess >= sizeof(g_direction_cipher)) {
    guess = 0;
		return true;
  }

	if (g_direction_cipher[guess] == dir)
		guess++;
	else
		guess = 0;

	if (p_guess)
		*p_guess = guess;

	if (guess >= sizeof(g_direction_cipher)) {
    guess = 0;
		return true;
  }

	return false;
}
