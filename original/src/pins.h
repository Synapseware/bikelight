/*
 * pins.h
 *
 * Created: 12/31/2011 3:50:01 PM
 *  Author: Matthew
 */ 


#ifndef PINS_H_
#define PINS_H_

/*
*/

//Temperature sensor input
#define TMP_IN			PINC0
#define TMP_IN_DDR		DDRC
#define TMP_IN_PORT		PORTC
#define TMP_IN_ADC		ADC0D

// Power switch input & interrupt handling
#define PWR_SW			PINC1
#define PWR_SW_DDR		DDRC
#define PWR_SW_PORT		PORTC
#define PWR_SW_IMSK		PCMSK1
#define PWR_SW_IEN		PCIE1
#define PWR_SW_INT		PCINT9
#define PWR_SW_VECT		PCINT1_vect

// LED driver enable control
// NOTE: Change this pin to enable bi-color LED functionality!
#define LED_EN			PINC2
#define LED_EN_DDR		DDRC
#define LED_EN_PORT		PORTC

// Power LED
#define PWR_LED			PINB1
#define PWR_LED_DDR		DDRB
#define PWR_LED_PORT	PORTB

// PWM output
#define PWM_O			PIND6
#define PWM_O_DDR		DDRD
#define PWM_O_PORT		PORTD


#endif /* PINS_H_ */