#include "antique.h"



//------------------------------------------------------------------------------------------
// Program globals
static	AntiqueBikeLight	light;
volatile uint8_t 			_tick			= 0;


//------------------------------------------------------------------------------------------
// Program
int main(void)
{
	light.init();

	light.shutDown();

    while(1)
    {
		if (_tick)
		{
			light.onTimerTick();
			_tick = 0;
		}
	}

	return 0;
}


//------------------------------------------------------------------------------------------
// Pin-change interrupt handler
ISR(PCINT0_vect)
{
	// call the bike light button handler here
	light.onButtonClick();
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
