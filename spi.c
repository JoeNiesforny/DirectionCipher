#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "gpio.h"

#define DDR_SPI	DDRB

#define SS	PB2 // D10 -> CS
#define MOSI	PB3 // D11 -> SDA
#define MISO	PB4 // D12 -> SDO
#define SCK	PB5 // D13 -> SCL

#define set_CS()	gpio_set_high(PORTB, SS)
#define clear_CS()	gpio_set_low(PORTB, SS)

void spi_master_init(void)
{
	/* Set MISO as input */
	DDR_SPI &= ~(1 << MISO);
	/* Set MOSI, SS and SCK as output */
	DDR_SPI |= (1 << MOSI) | (1 << SCK) | (1 << SS);
	/* Set pull-up resistors */
	gpio_set_high(PORTB, SS);
	gpio_set_high(PORTB, MISO);
	/* Enable SPI, Master, set clock rate fck/2 */
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA);
	/* SPI2X */
	SPSR |= 1;
}

void spi_start_transmission()
{
	clear_CS();
}

void spi_end_transmission()
{
	set_CS();
}

void spi_master_write(char data)
{
	/* Start transmission */
	SPDR = data;
	/* Empty cycle */
	asm volatile("nop");
	/* Wait for transmission complete */
	while(!(SPSR & (1 << SPIF)));
}

char spi_master_read()
{
	/* Wait for transmission complete */
	while(!(SPSR & (1 << SPIF)));
	/* Return receive byte */
	return SPDR;
}
