/*
* pins.h
*
* Created: 5/23/2013 8:50:01 PM
*  Author: Matthew Potter
*/ 


#ifndef PINS_H_
#define PINS_H_


//------------------------------------------------------------------------------------------
// Power switch
// While the power switch could use Int0, the button debounce routine & interrupt behavior
// are designed for pin-change interrupts.
// Pin should be set as input with pullup resistor enabled
#define PWR_SW				PB2
#define PWR_SW_INT			PCINT2

//------------------------------------------------------------------------------------------
// Power LED
// This is the LED that lights the power switch when the system is on.
#define PWR_LED				PB3
#define pwrLedEnable()		DDRB	|= (1<<PWR_LED)
#define pwrLedOn()			PORTB	|= (1<<PWR_LED)
#define pwrLedOff()			PORTB	&= ~(1<<PWR_LED)


//------------------------------------------------------------------------------------------
// PWM output
// The LED regulator uses PWM to control brightness.  0 = on, 1 = off.  Leave the pin
// floating high when CPU is in sleep mode in order to disable the light.
#define PWM_REGISTER		OCR0B
#define PWM_LED				PB1
#define pwmLedEnable()		DDRB	|= (1<<PWM_LED)
#define pwmLedDisable()		DDRB	&= ~(1<<PWM_LED)
#define pwmLedOff()			PORTB	|= (1<<PWM_LED)
#define pwmLedOn()			PORTB	&= ~(1<<PWM_LED)


#endif /* PINS_H_ */
