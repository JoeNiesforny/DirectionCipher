#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "gpio.h"
#include "uart.h"
#include "spi.h"
#include "adxl345.h"
#include "dir_cipher.h"
#include "motor_control.h"

int main(void)
{
	adxl345_acc_t acc;
	char buffer[256];
	uint8_t retries = 3;
	uint8_t dir = 0;
	uint8_t result = 0;

  /* set gpio C5/A5 to detect shelf state */
  gpio_dir_in(DDRC, DDC5);
  gpio_dir_in(DDRC, DDC4);
  gpio_set_high(PORTC, PORTC5);
  gpio_set_high(PORTC, PORTC4);

	uart_enable();
	close_door();

retry:
	spi_master_init();

	if (adxl345_enable()) {
		uart_send("Failed to enable ADXL345\r\n");
		if (retries) {
			SPCR = 0;
			retries--;
			goto retry;
		}
		while (true) {};
	}

	uart_send("Success to enable ADXL345\r\n");

	while (true) {
		adxl345_read(&acc);

#ifdef DEBUG_ADXL345
		sprintf(buffer, "x:%hi, y:%hi, z:%hi\r\n",
			acc.x, acc.y, acc.z);
		uart_send(buffer);
		_delay_ms(1000);
#else
		dir = check_direction(acc);
		if (dir != UNDEFINED) {
			sprintf(buffer, "x:%hi, y:%hi, z:%hi - %s\r\n",
				acc.x, acc.y, acc.z, direction_to_string(dir));
			uart_send(buffer);

			if (decode_direction_cipher_next(dir, &result)) {
				uart_send("Puzzle solved\r\n");
				open_door();

        while (gpio_get(PINC, PINC5));

        close_door();
			}

			sprintf(buffer, "guessed %u\r\n", result);
			uart_send(buffer);
		}
#endif

		if (uart_recv_byte_non_block(NULL)) {
			adxl345_disable();
			_delay_ms(10);
			SPCR = 0;
			uart_send("disabled SPI and adxl345\r\n");
			while (true) {};
		}

		_delay_ms(1);
	};

	return 0;
}
