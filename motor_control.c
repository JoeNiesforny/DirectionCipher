#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>

#include <util/atomic.h>
#include <util/delay.h>

#include "gpio.h"

/* Code is prepared for Pololu DRV8838 module */

/*
 * To get phase correct PWM output on PD6 pin
 * set 1 to COM0A1 and 0 to COM0A0.
 */
#define PWM_OUTPUT_A0	PD6
/* PWM on PD5 pin set COM0B1 and COM0B0 */
#define PWM_OUTPUT_B0	PD5

/* Phase is used to set direction of motor */
#define PHASE_OUT	PD7

static void motor_control_enable()
{
	/* Set Phase output */
	gpio_dir_out(DDRD, PHASE_OUT);

	/* Set PWM output */
	gpio_dir_out(DDRD, PWM_OUTPUT_A0);

	/*
	 * Set Timer0 to phase correct PWM mode 
	 * and PWM output at PD6 pin 
	 */
	TCCR0A |= (1 << COM0A1) | (1 << WGM00) | (1 << WGM01);
	/* Set prescaler to 8 and kick off PWM */
	TCCR0B |= (1 << CS01);
}

static void motor_control_duty(uint8_t duty)
{
	/*
	 * Duty is limited to MAX_UCHAR 256
	 * To get 20% duty -> 256 * 20% = 256 * (2 / 10) = 51
	 */
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		OCR0A = duty;
	}
}

static void motor_control_direction(bool phase)
{
	if (phase)
		gpio_set_high(PORTD, PHASE_OUT);
	else
		gpio_set_low(PORTD, PHASE_OUT);
}

static void motor_control_disable()
{
	/* Disable Timer0 */
	TCCR0A = 0;
	TCCR0B = 0;
}

#define DOOR_DUTY	128

#define DOOR_OPEN_TIMEOUT_MS	100
#define DOOR_OPEN_MAX_TIMEOUT_MS  3000

void open_door()
{
  int timeout = 0;

	motor_control_direction(true);
	motor_control_duty(DOOR_DUTY);
	motor_control_enable();

  while (true) {
	  _delay_ms(DOOR_OPEN_TIMEOUT_MS);
    timeout += DOOR_OPEN_TIMEOUT_MS;

    if (!gpio_get(PINC, PIN4))
      break;
    if (timeout > DOOR_OPEN_MAX_TIMEOUT_MS)
      break;
  }

	motor_control_disable();
}

#define DOOR_CLOSE_SAMPLE_MS 10
#define DOOR_CLOSE_PERIOD_MS 3000

void close_door()
{
  int timeout = 0;

	motor_control_direction(false);
	motor_control_duty(DOOR_DUTY);
	motor_control_enable();

  while (true) {
    _delay_ms(DOOR_CLOSE_SAMPLE_MS);
    timeout += DOOR_CLOSE_SAMPLE_MS;

    if (!gpio_get(PINC, PIN5))
      break;

    if (timeout > DOOR_CLOSE_PERIOD_MS)
      break;
  }

	motor_control_disable();
}
