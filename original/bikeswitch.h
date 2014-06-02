/*
 * bikeswitch.h
 *
 * Created: 12/29/2011 3:41:14 PM
 *  Author: Matthew
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <math.h>

#include "pins.h"


#ifndef BIKESWITCH_H_
#define BIKESWITCH_H_

//------------------------------------------------------------------------------------------
// Main clock-div value
#define CLK_DIV 240


//------------------------------------------------------------------------------------------
// blink modes
#define MODE_OFF		0
#define MODE_DIM		1
#define MODE_HIGH		2
#define MODE_BK			3
#define MODE_SP			4
#define MODE_FD			5
#define MODE_MAX		MODE_FD

// button debounce delay
#define BUTTON_DEBOUNCE		32

// button states
#define STATE_PROCESSED	0
#define STATE_DEBOUNCE	1

// effects state
#define EFFECTS_WAIT	1

#define TIMER0_PRESCALE	((1<<CS02) | (0<<CS01) | (0<<CS00))
#define TIMER0_PWM_OUT	((1<<COM0A1) | (0<<COM0A0))

//------------------------------------------------------------------------------------------
// Methods
void init(void);
void processMode(uint8_t mode);
void shutDown(void);
void SetDriverLevel(uint8_t level);
//------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------
// each "delay" value is in units of 10ms
// So, if a delay is set for 12, then the total time is 12 * 10ms, or 120ms.

#define BLINK_DELAY 5	// 3 fast blinks and 1 slow blink
const static uint8_t BLINK_TABLE[] PROGMEM = {
	// brightness	delay
	0.10 * CLK_DIV, BLINK_DELAY,
	0,				BLINK_DELAY,
	0.10 * CLK_DIV, BLINK_DELAY,
	0,				BLINK_DELAY,
	0.10 * CLK_DIV, BLINK_DELAY,
	0,				BLINK_DELAY,
	1.00 * CLK_DIV, BLINK_DELAY,
	0,				BLINK_DELAY
};
const static uint8_t BLINK_TABLE_LEN PROGMEM = sizeof(BLINK_TABLE)/sizeof(uint8_t);

#define SLEEP_TABLE_DELAY 2 // sleepy eye effect
const static uint8_t SLEEP_TABLE[] PROGMEM = {
	0.01 * CLK_DIV,
	0.02 * CLK_DIV,
	0.04 * CLK_DIV,
	0.07 * CLK_DIV,
	0.10 * CLK_DIV,
	0.18 * CLK_DIV,
	0.25 * CLK_DIV,
	0.33 * CLK_DIV,
	0.50 * CLK_DIV,
	0.70 * CLK_DIV,
	1.00 * CLK_DIV,
	0.70 * CLK_DIV,
	0.50 * CLK_DIV,
	0.33 * CLK_DIV,
	0.25 * CLK_DIV,
	0.18 * CLK_DIV,
	0.10 * CLK_DIV,
	0.07 * CLK_DIV,
	0.04 * CLK_DIV,
	0.02 * CLK_DIV,
	0.01 * CLK_DIV,
	0.01 * CLK_DIV
};
const static uint8_t SLEEP_TABLE_LEN PROGMEM = sizeof(SLEEP_TABLE)/sizeof(uint8_t);

#define FADE_DELAY 2
const static uint8_t FADE_DN_TABLE[] PROGMEM = {
	1.00 * CLK_DIV,
	0.30 * CLK_DIV,
	0.15 * CLK_DIV,
	0.10 * CLK_DIV,
	0.07 * CLK_DIV,
	0.02 * CLK_DIV,
	0.01 * CLK_DIV,
	1,
	1,
	1
};
const static uint8_t FADE_DN_TABLE_LEN PROGMEM = sizeof(FADE_DN_TABLE)/sizeof(uint8_t);


#endif /* BIKESWITCH_H_ */