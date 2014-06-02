/*
* blinky.h
*
* Created: 12/29/2011 3:41:14 PM
*  Author: Matthew
*/ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/power.h>

#include "pins.h"


#ifndef __BLINKY__H_
#define __BLINKY__H_


//------------------------------------------------------------------------------------------
// Main clock-div value
#define TIMER_PRESCALE			8

// Yields an 8-bit number that when combined with clock prescalers will give a 5KHz interrupt clock
// 9.6MHz/8/5000 = 240
#define TIMER_CLK_DIV			240
//F_CPU/TIMER_PRESCALE/5000

// This is the timer delay needed to convert the PWM carrier frequency to a 1ms clock tick
// 9.6MHz/8/5000/1000 = 5
#define TIMER_MS_DIV			5
//F_CPU/TIMER_PRESCALE/TIMER_CLK_DIV/1000

// This is the carrier frequency of the PWM output
// 9.6MHz/8/240 = 5KHz
//#define PWM_CARRIER_FREQUENCY	F_CPU/TIMER_PRESCALE/TIMER_CLK_DIV


//------------------------------------------------------------------------------------------
// blink modes
#define MODE_NONE				0
#define MODE_MAX				1
#define MODE_BK					2
#define MODE_TOTAL				2

//------------------------------------------------------------------------------------------
// button debounce delay (in milliseconds)
#define BUTTON_PRESS_TIMEOUT	25
#define BUTTON_RELEASE_TIMEOUT	200
#define BUTTON_RELEASED			1
#define BUTTON_PRESSED			0

//------------------------------------------------------------------------------------------
// button states
#define STATE_UNKNOWN			0
#define STATE_TRIGGERED			1
#define STATE_DEBOUNCE			2
#define STATE_DEBOUNCED			3


//------------------------------------------------------------------------------------------
// PWM timer values
// clk/8 = 9.6MHz/1 = 9.6MHz
// 1.2MHz / 256 = 4,687.5Hz
#define TIMER0_PRESCALE			((0<<CS02) | (0<<CS01) | (1<<CS00))
#define TIMER0_PWM_OUT			((1<<COM0A1) | (0<<COM0A0) | (1<<COM0B1) | (0<<COM0B0))


//------------------------------------------------------------------------------------------
// Methods
void init(void);
void poleButtonState(void);
void processMode(void);
void processTemp(void);
void processYellow(void);
void shutDown(void);
void setDriverLevel(uint8_t level);
//------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------
// each "delay" value is in units of 10ms
// So, if a delay is set for 12, then the total time is 12 * 10ms, or 120ms.
#define BLINK_DELAY	125
const static uint8_t BLINK_TABLE[] PROGMEM = {
	// brightness	delay
	0.00 * TIMER_CLK_DIV,	0,
	0.20 * TIMER_CLK_DIV,	0,
	0.00 * TIMER_CLK_DIV,	0,
	1.00 * TIMER_CLK_DIV,	0,
	0.00 * TIMER_CLK_DIV,	0,
	0.00 * TIMER_CLK_DIV,	0,
	0.20 * TIMER_CLK_DIV,	0,
	0.00 * TIMER_CLK_DIV,	0,
	0.00 * TIMER_CLK_DIV,	0,
	1.00 * TIMER_CLK_DIV,
	1.00 * TIMER_CLK_DIV,
};
const static uint8_t BLINK_TABLE_LEN = sizeof(BLINK_TABLE)/sizeof(uint8_t);


#endif /* __BIKELIGHT__H_ */