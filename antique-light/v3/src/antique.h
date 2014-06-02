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

#define BLINK_TABLE_FACTOR	240
#include <libs/effects/blink-tables.h>

#include "pins.h"

#ifndef __ANTIQUE_H__
#define __ANTIQUE_H__


//------------------------------------------------------------------------------------------
// Main clock-div value
// F_CPU = 9,600,000
#define TIMER_PRESCALE			1

// Yields an 8-bit number that when combined with clock prescalers will give a 5KHz interrupt clock
// 9.6MHz/8/5000 = 240
#define TIMER_CLK_DIV			240

// This is the carrier frequency of the PWM output
#define PWM_FREQUENCY			F_CPU/TIMER_PRESCALE/TIMER_CLK_DIV

// This is the timer delay needed to convert the PWM carrier frequency to a 1ms clock tick
#define TIMER_MS_DIV			PWM_FREQUENCY/1000


//------------------------------------------------------------------------------------------
// blink modes
#define MODE_NONE				0
#define MODE_DIM				1
#define MODE_MAX				2
#define MODE_CYCLE				3
#define MODE_TOTAL				3

#define CYCLE_WAIT				1950

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
class AntiqueBikeLight
{
public:
	void init(void);
	void shutDown(void);
	void onTimerTick(void);
	void onButtonClick(void);
private:
	void preparePWM(void);
	void preparePowerSwitch(void);
	void poleButtonState(void);
	void processMode(void);
	void cycleEffects(void);
	void setDriverLevel(uint8_t level);

	uint8_t		_mode;				// lighting mode
	uint8_t		_tick;				// 1ms interval flag
	uint8_t		_state;				// button debounce state
	uint8_t		_cycle;				// lighting effects cycle value
	uint16_t	_cycleWait;			// Delay before changing cycles
};


#endif
