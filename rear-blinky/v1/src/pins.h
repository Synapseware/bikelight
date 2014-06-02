/*
* pins.h
*
* Created: 12/31/2011 3:50:01 PM
*  Author: Matthew
*/ 


#ifndef PINS_H_
#define PINS_H_


//------------------------------------------------------------------------------------------
// Power switch
#define PWR_SW				PB0
#define pwrSwReleased()		((PORTB & (1<<PWR_SW)) != 0)
#define pwrSwPressed()		((PORTB & (1<<PWR_SW)) == 0)


//------------------------------------------------------------------------------------------
// Power LED
#define PWR_LED				PB3
#define pwrLedEnable()		DDRB	|= (1<<PWR_LED)
#define pwrLedOn()			PORTB	|= (1<<PWR_LED)
#define pwrLedOff()			PORTB	&= ~(1<<PWR_LED)
#define pwrLedTgl()			PINB	|= (1<<PWR_LED)


//------------------------------------------------------------------------------------------
// PWM output
#define PWM_REGISTER		OCR0B
#define PWM_LED				PB1
#define pwmLedEnable()		DDRB	|= (1<<PWM_LED)
#define pwmLedOff()			PORTB	&= ~(1<<PWM_LED)


#endif /* PINS_H_ */