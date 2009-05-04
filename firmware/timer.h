/*
 * ==========================================================================
 *
 *       Filename:  timer.h
 *
 *    Description:  timer header
 *
 *        Created:  05/04/09 13:42:34
 *       Revision:  none
 *
 *         Author:  Cojocar Lucian, cojocar-gmail-com
 *
 * ==========================================================================
 */


#ifndef  TIMER_INC
#define  TIMER_INC

#define TIMER_ADC		TCNT1
#define TIMER_ADC_RESET	do {TIMER_ADC = 0;} while (0);

void init_timer_for_pwm(void);
void init_timer_for_adc(void);
void stop_timer_for_adc(void);

#endif   /* ----- #ifndef TIMER_INC  ----- */
