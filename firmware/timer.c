/*
 * ==========================================================================
 *
 *       Filename:  timer.c
 *
 *    Description:  timer functions
 *
 *        Created:  05/04/09 13:34:24
 *       Revision:  none
 *
 *         Author:  Cojocar Lucian, cojocar-gmail-com
 *
 * ==========================================================================
 */

#include <avr/io.h>

#include "config.h"
#include "timer.h"

void
init_timer_for_adc(void)
{
	TCCR1A;
	/* prescaler 1, intr-o ms nu ar trebui sa avem overflow */
	/* dau enable intreruperi */
	TIMSK |= _BV(TOIE1) | _BV(TICIE1);
	TCCR1B |= _BV(CS11) | _BV(CS10) | _BV(ICNC1);
	//| _BV(CS10);
	//| _BV(CS11);
	//TCCR1B = _BV(CS12);

}


void
stop_timer_for_adc(void)
{
	TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));
	TIMSK &= ~(_BV(TOIE1) | _BV(TICIE1));

}

void
init_timer_for_pwm(void)
{
	/* CTC, toggle */
	TCCR0 = _BV(WGM01) | _BV(COM00);
	/* prescallerul, 1024 */
	TCCR0 |= _BV(CS00) | _BV(CS02); 

	/* 30 HZ */
	OCR0 = PWM_SPEED;
	/**/ 

	DDRB |= _BV(PB3);
}
