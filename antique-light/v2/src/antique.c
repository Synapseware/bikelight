/*
 * blinky.c
 *
 * Created: 12/10/2011 1:02:19 PM
 *  Author: Matthew
 
ATtiny85 - uC powered bicycle LED bicycle headlight.
- Uses SMPS theory to regulate current flow for high power LED.
- Feedback system is an analog comparator, who's value is used to
adjust the duty cycle used by the SMPS.
- A temperature sensor monitors the heat of the high power LED
board and attempts to compensate the lighting intensity or effects
used to keep temperature within design limits (100 C max)
- 

5/13/13
	- Use OC0B for PWM output
	- Change from INT0 to PCINTx, whichever is most appropriate
	- Update timer to use F_CPU/8/
 */ 


#include "antique.h"



//------------------------------------------------------------------------------------------
// volatiles used by interrupt handlers
volatile uint8_t	_mode			= MODE_NONE;			// lighting mode
volatile uint8_t	_tick			= 0;					// 1ms interval flag
volatile uint8_t	_state			= 0;					// button debounce state

volatile uint8_t	_cycle			= 0;					// lighting effects cycle value


//------------------------------------------------------------------------------------------
// Prepare timer0 for PWM output on OC0A
void preparePWM(void)
{
	// enable PWM output
	pwmLedEnable();

	// setup timer0 for fast PWM
	TCCR0A	=	TIMER0_PWM_OUT	|
				(1<<WGM01)		|		// Fast PWM
				(1<<WGM00);
	TCCR0B	=	(1<<WGM02)		|		// Fast PWM
				TIMER0_PRESCALE;		// Prescale macro

	TIMSK0	=	(1<<TOIE0);				// Enable timer0 overflow interrupt (to count 'ticks')
										// Interrupt frequency is 9.6MHz/1/200 = 48kHz

	OCR0A	=	TIMER_CLK_DIV-1;		// OCR0A sets the final clock div value
	OCR0B	=	0;						// This is the PWM value
}


//------------------------------------------------------------------------------------------
// Configure the power switch input
void preparePowerSwitch(void)
{
	// setup PWR_SWITCH pin as input with pull up resistor enabled
	MCUCR	&=	~(1<<PUD);				// make sure pull-ups are enabled
	DDRB	&=	~(1<<PWR_SW);			// set power switch as input pin
	PORTB	|=	(1<<PWR_SW);			// enable pull-up resistor on switch sensor

	// enable interrupt triggering
	GIMSK	|=	(1<<PCIE);
	PCMSK	|=	(1<<PWR_SW_INT);
}


//------------------------------------------------------------------------------------------
// System Setup
void init(void)
{
	ADCSRA			= 0;	// disable a/d

	preparePWM();

	preparePowerSwitch();

	// init startup values
	_mode		= MODE_NONE;
	_state		= STATE_UNKNOWN;

	pwrLedEnable();

	sei();
}


//------------------------------------------------------------------------------------------
// processes button presses
void poleButtonState(void)
{
	static uint16_t debounce = 0;
	static uint8_t position = BUTTON_RELEASED;
	static uint16_t samplesRemaining = 0;

	switch (_state)
	{
		// nothing should be done for the unknown state (default start state)
		case STATE_UNKNOWN:
			// no-op
			break;

		// button interrupt handler sets state to STATE_TRIGGERED
		case STATE_TRIGGERED:

			// set the new state
			_state = STATE_DEBOUNCE;

			// setup debounce values
			switch (position)
			{
				case BUTTON_RELEASED:
					position = BUTTON_PRESSED;
					debounce = BUTTON_PRESS_TIMEOUT;
					samplesRemaining = BUTTON_PRESS_TIMEOUT * 0.50;
					break;
				case BUTTON_PRESSED:
					position = BUTTON_RELEASED;
					debounce = BUTTON_RELEASE_TIMEOUT;
					samplesRemaining = BUTTON_RELEASE_TIMEOUT * 0.50;
					break;
			}

			break;

		// uses an over-sampling approach to determine if a click event is legit
		case STATE_DEBOUNCE:
			if (0 != debounce)
			{
				debounce--;
				if (0 != samplesRemaining)
				{
					uint8_t val = PINB & (1<<PWR_SW);

					if (position == BUTTON_RELEASED && val != 0)
						samplesRemaining--;
					else if (position == BUTTON_PRESSED && val == 0)
						samplesRemaining--;
				}
			}
			else
			{
				// if counter is not zero, then we didn't get enough good
				// samples during the debounce run
				if (0 != samplesRemaining)
				{
					_state = STATE_UNKNOWN;
					samplesRemaining = 0;
					debounce = 0;
				}
				else
					_state = STATE_DEBOUNCED;
			}
			break;

		// debounce routine completed, we have a good button state change
		case STATE_DEBOUNCED:
			_state = STATE_UNKNOWN;
			if (BUTTON_PRESSED == position)
			{
				// change the mode
				if (++_mode > MODE_TOTAL)
				{
					_mode = MODE_NONE;
					position = BUTTON_RELEASED;
				}
			}
			break;
	}
}


//------------------------------------------------------------------------------------------
// processMode
// Blinks the power LED circuit to simulate certain lighting effects
// Called every 1ms
void processMode()
{
	static uint8_t	delay			= 0;
	static uint8_t 	effectsIdx		= 0;
	static uint8_t 	effectsMax		= 0;
	static uint8_t 	brightness		= 0;
	static uint8_t	lastCycle		= 0;

	// skip until delay is 0
	if (delay > 0)
	{
		delay--;
		return;
	}

	// steady modes
	switch(_mode)
	{
		case MODE_NONE:
			shutDown();
			return;
		case MODE_DIM:
			brightness		= TIMER_CLK_DIV * 0.40;
			delay			= 0;
			effectsIdx		= 0;
			break;
		case MODE_MAX:
			brightness		= TIMER_CLK_DIV * 0.95;
			delay			= 0;
			effectsIdx		= 0;
			break;
		case MODE_CYCLE:

			// reset the effects table index if we cycle to a new effect
			if (lastCycle != _cycle)
			{
				lastCycle = _cycle;
				effectsIdx = 0;
			}

			// process 1 of 3 special blinking effects
			switch(_cycle)
			{
				case 0:
					effectsMax		= BLINK_TABLE_LEN;
					delay			= BLINK_TABLE_DELAY;
					brightness		= pgm_read_byte(&BLINK_TABLE[effectsIdx++]);
					break;
				case 1:
					effectsMax		= SLEEP_TABLE_LEN;
					delay			= SLEEP_TABLE_DELAY;
					brightness		= pgm_read_byte(&SLEEP_TABLE[effectsIdx++]);
					break;
				case 2:
					effectsMax		= FADE_TABLE_LEN;
					delay			= FADE_TABLE_DELAY;
					brightness		= pgm_read_byte(&FADE_TABLE[effectsIdx++]);
					break;
			}
			break;
	}

	// cycle the effects
	if (effectsIdx >= effectsMax)
		effectsIdx = 0;

	setDriverLevel(brightness);
}


//------------------------------------------------------------------------------------------
// Cycles through the modes if it is any of the patterned modes
void cycleEffects(void)
{
	static uint16_t _cycleWait = CYCLE_WAIT;
	if (_cycleWait > 0)
	{
		_cycleWait--;
		return;
	}

	_cycle++;
	if (_cycle > CYCLE_MAX)
		_cycle = 0;

	switch (_cycle)
	{
		case 0:
			_cycleWait = BLINK_TABLE_DELAY * BLINK_TABLE_LEN * 2;
			break;
		case 1:
			_cycleWait = SLEEP_TABLE_DELAY * SLEEP_TABLE_LEN * 2;
			break;
		case 2:
			_cycleWait = FADE_TABLE_DELAY * FADE_TABLE_LEN * 1;
			break;
	}
}


//------------------------------------------------------------------------------------------
// sets the PWM output level
void setDriverLevel(uint8_t level)
{
	// set the PWM brightness level
	PWM_REGISTER = level;
}


//------------------------------------------------------------------------------------------
// Shuts down the uC to conserve power
void shutDown(void)
{
	// shutdown timer0
	OCR0B	=	0;
	TIMSK0	=	0;
	TCCR0B	&=	~(TIMER0_PRESCALE);

	// Shut down LEDs
	pwrLedOff();
	pwmLedEnable();
	pwmLedOff();
	
	// sleep!  and ignore button-up
	while ((PINB & (1<<PWR_SW)) != 0)
	{
		pwmLedOff();
		sleep_enable();
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_cpu();
	}

	pwmLedEnable();
	pwrLedOn();

	// enable the timer
	TCCR0B	|=	(TIMER0_PRESCALE);
	TIMSK0	=	(1<<TOIE0);
}


//------------------------------------------------------------------------------------------
// Program
int main(void)
{
	init();

	shutDown();

    while(1)
    {
		if (_tick)
		{
			poleButtonState();

			processMode();

			cycleEffects();

			_tick = 0;
		}
	}

	return 0;
}


//------------------------------------------------------------------------------------------
// Pin-change interrupt handler
ISR(PCINT0_vect)
{
	// only start button state processing if it's been processed completely
	if (_state == STATE_UNKNOWN)
		_state = STATE_TRIGGERED;
}


//------------------------------------------------------------------------------------------
// Timer0 overflow interrupt handler
ISR(TIM0_OVF_vect)
{
	// process timer delay to get ticks
	static uint8_t timerDelay = 0;
	if (timerDelay++ < TIMER_MS_DIV)
		return;

	// intervals on 1ms
	timerDelay = 0;
	_tick = 1;
}
