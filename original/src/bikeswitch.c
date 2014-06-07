/*
 * bikeswitch.c
 *
 * Created: 12/10/2011 1:02:19 PM
 *  Author: Matthew
 
ATMega328p - uC powered bicycle LED bicycle headlight.
- Uses SMPS theory to regulate current flow for high power LED.
- Feedback system is an analog comparator, who's value is used to
adjust the duty cycle used by the SMPS.
- A temperature sensor monitors the heat of the high power LED
board and attempts to compensate the lighting intensity or effects
used to keep temperature within design limits (100 C max)
- 
 */ 

#include "bikeswitch.h"



//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------
// volatiles used by interrupt handlers
//------------------------------------------------------------------------------------------
volatile uint8_t _mode			= MODE_OFF;
volatile uint8_t _delay			= 0;
volatile uint8_t _proc			= 0;
volatile uint8_t _effectsDelay	= EFFECTS_WAIT;
volatile uint8_t _effectsIndex	= 0;
volatile uint8_t _button		= 0;
volatile uint8_t _state			= 0;
volatile uint8_t _cycleLimit	= 0;
volatile uint8_t _dutyCycle		= 0;


const uint8_t CYCLE_MIN			= CLK_DIV * 0.03;
const uint8_t CYCLE_MAX			= CLK_DIV * 0.87;

const uint8_t SMPS_LOW			= CLK_DIV * 0.35;
const uint8_t SMPS_HIGH			= CLK_DIV * 0.87;


//------------------------------------------------------------------------------------------
// System Setup
//------------------------------------------------------------------------------------------
void init(void)
{
	// adjust power saving modes (pg 40)
	PRR		=	(0<<PRTIM0) |			// enable timer0
				(0<<PRTIM1) |			// enable timer1
				(1<<PRTIM2) |			// disable timer2
				(0<<PRADC);				// disable ADC

	// enable pull-up control
	MCUCR	=	(0<<PUD);				// enable pull-up resistors (pg 66)

	// clear primary status flags (pg 47)
	MCUSR	=	(0<<WDRF)	|
				(0<<BORF)	|
				(0<<EXTRF)	|
				(0<<PORF);

	// enable WDT (pg 47)
	/*
	WDTCR	=	(1<<WDCE) | (1<<WDE);
	WDTCR	=	(1<<WDIF) |
				(1<<WDCE) |
				(1<<WDE) |
				(0<<WDP2) |				// configure WDR for 1/8s reset intervals
				(1<<WDP1) |
				(1<<WDP0);
	*/
	// sample on ADC3, PB3, pin 2
	ADMUX	=	(1<<REFS1) |	// see table on page 138
				(1<<REFS0) |	// 
				(1<<ADLAR) |	// left adjust
				(0<<MUX3) |		// MUX bits are 0001 (ADC1, PB2)
				(0<<MUX2) |
				(0<<MUX1) |
				(0<<MUX0);
	ADCSRA	=	(0<<ADEN) |		// disable ADC
				(0<<ADSC) |
				(0<<ADATE) |
				(0<<ADIE) |		// enable ADC interrupts
				(1<<ADPS2) |	// set prescaler to be clk/64
				(1<<ADPS1) |	// which is 125kHz
				(0<<ADPS0);
	DIDR0	|=	(1<<TMP_IN_ADC);

	// setup timer0 (pg 80 & 82) for fast PWM, output on COM0A
	TCCR0A	=	(1<<WGM01)	|
				(1<<WGM00)	|
				TIMER0_PWM_OUT;

	TCCR0B	=	(0<<WGM02)	|
				TIMER0_PRESCALE;

	OCR0A	=	0;						// Mode button select check interval (31,250/250 = 125Hz, or 125ms interval on checks)
	TIMSK0	|=	(1<<TOIE0);				// Enable overflow handler (pg 84)

	// setup PWM output and driver enable line
	PWM_O_DDR		|= (1<<PWM_O);
	LED_EN_DDR		|= (1<<LED_EN);

	// setup power LED
	PWR_LED_DDR		|= (1<<PWR_LED);
	PWR_LED_PORT	&= ~(1<<PWR_LED);

	// setup PWR_SWITCH pin as input with pull up resistor enabled
	PCICR		|=	(1<<PWR_SW_IEN);		// enable PCI1 block
	PWR_SW_IMSK	|=	(1<<PWR_SW_INT);		// enable pin-change on power switch pin
	PWR_SW_DDR	&=	~(1<<PWR_SW);			// set power switch as input pin
	PWR_SW_PORT	|=	(1<<PWR_SW);			// enable pull-up resistor on switch sensor

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	sei();

	// start in dim mode
	_mode = MODE_OFF;
}

//------------------------------------------------------------------------------------------
// Pin-change interrupt handler
//------------------------------------------------------------------------------------------
ISR(PWR_SW_VECT)
{
	_button	=	BUTTON_DEBOUNCE;			// restart button timer
	_state	=	STATE_DEBOUNCE;				// set button state
	PWR_LED_PORT	&=	~(1<<PWR_LED);		// turn on power LED
	LED_EN_PORT		|= (1<<LED_EN);			// enable LED driver
}

//------------------------------------------------------------------------------------------
// A/D Conversion complete
//------------------------------------------------------------------------------------------
ISR(ADC_vect)
{
	/*
	unsigned char result = ADCH;
	
	CYCLE_MIN = (result * 0.70);
	CYCLE_MAX = CYCLE_MIN;

	OCR0A = result;
	*/
}

//------------------------------------------------------------------------------------------
// Timer0 overflow interrupt handler
// Used to poll the switch input pin :)
// 8mHz / 256 / 250 = 125Hz
//------------------------------------------------------------------------------------------
ISR(TIMER0_OVF_vect)
{
	if (_button > 0 && _state == STATE_DEBOUNCE)
	{
		_button--;
		if (_mode == MODE_OFF)
			return;
	}
	else
	{
		if (_state != STATE_PROCESSED)
		{
			_state = STATE_PROCESSED;

			// change to the next mode
			_mode++;
			_effectsIndex = 0;
			if (_mode > MODE_MAX)
				_mode = MODE_OFF;
		}
	}

	// delay the effects a little...
	_effectsDelay--;
	if (_effectsDelay > 0)
		return;

	// reset the effects delay
	_effectsDelay = EFFECTS_WAIT;

	// signal main code to process lighting effects
	_proc = 1;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SetDriverLevel(uint8_t level)
{
	if (level > 1)
	{
		// PWM output
		TCCR0A		|=	TIMER0_PWM_OUT;		// Connect OC0A to timer
		PWM_O_PORT	&= ~(1<<PWM_O);			// Drive PWM low
	}
	else
	{
		// steady output
		TCCR0A		&=	~TIMER0_PWM_OUT;	// Set OC0A to normal mode, disconnected
		PWM_O_PORT	&=	~(1<<PWM_O);		// Drive PWM low
	}

	OCR0A = level;
}

//------------------------------------------------------------------------------------------
// processMode
// Blinks the power LED circuit to simulate certain lighting effects
// Executed 1/100s (10ms)
//------------------------------------------------------------------------------------------
void processMode(uint8_t mode)
{
	_proc = 0;

	// stead modes
	switch(mode)
	{
		case MODE_OFF:
			SetDriverLevel(0);
			shutDown();
			return;
		case MODE_DIM:
			SetDriverLevel(SMPS_LOW);
			return;
		case MODE_HIGH:
			SetDriverLevel(SMPS_HIGH);
			return;
	}

	// blink modes
	if (_delay == 0)
	{
		uint8_t brightness = 0;
		uint8_t delay = 0;
		uint8_t effectsMax = 0;

		switch (mode)
		{
			case MODE_BK:			// BLINK
				brightness = pgm_read_byte(&BLINK_TABLE[_effectsIndex++]);
				delay = pgm_read_byte(&BLINK_TABLE[_effectsIndex++]);
				effectsMax = pgm_read_byte(&BLINK_TABLE_LEN);
				break;
			case MODE_SP:			// SLEEPY-BLINK
				delay = SLEEP_TABLE_DELAY;
				brightness = pgm_read_byte(&SLEEP_TABLE[_effectsIndex++]);
				effectsMax = pgm_read_byte(&SLEEP_TABLE_LEN);
				break;
			case MODE_FD:			// FADE-OUT
				delay = FADE_DELAY;
				brightness = pgm_read_byte(&FADE_DN_TABLE[_effectsIndex++]);
				effectsMax = pgm_read_byte(&FADE_DN_TABLE_LEN);
				break;
		}

		SetDriverLevel(brightness);

		if (_effectsIndex >= effectsMax)
			_effectsIndex = 0;

		_delay = delay;
	}
	else
		_delay--;	
}

//------------------------------------------------------------------------------------------
// Shuts down the uC to conserve power
//------------------------------------------------------------------------------------------
void shutDown(void)
{
	PWR_LED_PORT |= (1<<PWR_LED);			// Shut down power LED
	LED_EN_PORT	 &= ~(1<<LED_EN);			// Shut down LED driver

	sleep_enable();
	sleep_cpu();
	sleep_disable();
}

//------------------------------------------------------------------------------------------
// Program
//------------------------------------------------------------------------------------------
int main(void)
{
	init();

	// shut down after initial startup
	shutDown();

    while(1)
    {
		wdt_reset();

		if (_proc)
		{
			processMode(_mode);

			// start a temperature read conversion
			//ADCSRA |= (1<<ADSC);
		}
	}

	return 0;
}
