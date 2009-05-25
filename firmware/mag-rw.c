/*
 * ==========================================================================
 *
 *       Filename:  mag-rw.c
 *
 *    Description:  main file of the project
 *
 *        Created:  05/04/09 13:51:48
 *       Revision:  none
 *
 *         Author:  Cojocar Lucian, cojocar-gmail-com
 *
 * ==========================================================================
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "usart.h"
#include "timer.h"


enum state {
	S_INIT,
	S_READING,
	S_READ
} STATE;

uint16_t count;
uint8_t t, r, skip;
#define BUF_SZ	16
uint8_t buf[BUF_SZ];
uint8_t bit_count;
uint8_t count_com;

uint8_t last_lvl;
uint32_t count_identic;
uint64_t count_adc;
uint32_t zero_count;
uint8_t to_print;
uint16_t zero_time[2];
uint16_t unu_time[2];

void
init_adc(void)
{
	/*
	 * right adjusted
	 * adc1
	 * use avcc
	 */
	ADMUX = _BV(REFS0) | _BV(MUX0);
	/*
	ADMUX = _BV (REFS0) | _BV (MUX0) | _BV (MUX1) | _BV (MUX2) | _BV (ADLAR);
	*/

	/*
	 * enable adc
	 * prescaler 128
	 * intreruperi
	 */
	ADCSRA =  /*_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) |*/ _BV(ADIE) | _BV(ADSC) | _BV(ADEN);	
	//ADCSRA |= _BV(ADPS1);
	/*
	 * free running mode
	 */
}

void
stop_adc(void) 
{
	ADCSRA &= ~_BV(ADEN);
}



inline void
output(void)
{
	STOP_MOTOR;
	STATE = S_READ;
	stop_timer_for_adc();
	stop_adc();

	uint16_t i;
	/*
	for (i = 0; i < BUF_SZ; ++ i) {
		usart_put_char(buf[i]);
	}*/
	usart_put_string("START");
	for (i = 0; i <= count; ++ i) {
		usart_put_char((uint8_t)buf[i]);
	}
	usart_put_int16(zero_time[0]);
	usart_put_char(' ');
	usart_put_int16(zero_time[1]);
	usart_put_char(' ');
	usart_put_int32(count);
	usart_put_char(' ');
	usart_put_char('0' + bit_count);

	usart_put_char('\n');
	count = 0;
	bit_count = 0;
}


SIGNAL(SIG_OVERFLOW1) 
{
	/* adc timer overflow */
	usart_put_string("OVERFLOW\n");
	STOP_MOTOR;
	if (zero_count >= ZEROS1) {
		output();
	}

	//STATE = S_INIT;
}


inline void
put_bit(uint8_t bit)
{
	//STOP_MOTOR;
	if (bit_count < 8) {
		buf[count] <<= 1;
		++ bit_count;
	} else {
		++ count;
		bit_count = 1;
		buf[count] = 0;
	}
	buf[count] |= bit;
	if ((count == (BUF_SZ-1)) && (bit_count == 8)) {
		output();
		usart_put_string("BUFFEND\n");
	}
}

uint16_t count_diferit;

#define TRESH 3
SIGNAL(SIG_ADC)
{
	
	uint8_t bit;

	++ count_adc;
	uint8_t lvl;
	uint16_t com_time;
	if (ADC > 170) {
		lvl = 1;
	} else {
		lvl = 0;
	}
	
	/*
	if (bit_is_set(TIFR, TOV1)) {
		STATE = S_READ;
		STOP_MOTOR;
		output();
	}*/
	/*
	if (zero_count >= ZEROS1) {
		put_bit(lvl);
	}*/
	if (lvl == last_lvl) {
		if (count_identic < 0xffffffff) {
			++ count_identic;
		}
	} else {
		++ count_diferit;
		if (count_diferit >= TRESH){ //&& (TCNT1 > 100)) {
			count_identic = 0;
			count_diferit = 0;
			last_lvl = lvl;

		/* a comutat */
			// STOP_MOTOR;
			if (zero_count < ZEROS) {
				/*
				 * sunt intr-un sir de zerouri,
				 * salvez cate esantionari fac
				 * ar trebui sa am minim doi biti de 1
				 */
				++ zero_count;
			} else {
				/* deja am trecut de zerouri, urmeaza sa folosesc timerul */
				if (zero_count == ZEROS) {
					/* ratam un zero */
					/* de aici trecem la timer */
					//usart_put_string("dau drumul la timer\n");
					//STOP_MOTOR;
					///*
					init_timer_for_adc();
					TCNT1 = 0;
					//*/
					++ zero_count;
				} else {
					if (zero_count < ZEROS1) {
						r = 2;
						++ zero_count;
						zero_time[!lvl] = TCNT1; //(zero_time[!lvl]>>1) + (TCNT1>>1);
						TCNT1 = 0;
					} else {
						//STOP_MOTOR;
						com_time = TCNT1;
						TCNT1 = 0;
						/*
						if (com_time <= (zero_time[!lvl]>>2)) {
							usart_put_string("t=1\n");
						} else 
							t = 4;
						} else	if (com_time <= ((zero_time[!lvl]>>1) + ((zero_time[!lvl]>>2)))) {
							t = 2;
						} else {
							t = 3;
						}*/
						if (com_time <= ((zero_time[!lvl]>>1) + ((zero_time[!lvl]>>5)))){//7)))) {
							t = 2;
						} else if (com_time >= ((zero_time[!lvl]) - (zero_time[!lvl]>>5))){//7))) {
							#if 0
							if (com_time >= ((zero_time[!lvl]) + (zero_time[!lvl]>>2))) {
								/* we missed 1 */
								put_bit(1);
								r = 1;
								t = 3;
							} else {
								t = 4;
								zero_time[!lvl] = com_time;
							}
							#endif
							t = 4;
							//zero_time[!lvl] = com_time;
						} else {
							t = 3;
						}
						
						if (skip) {
							skip = 0;
							r = 1;
							TCNT1 = 0;
							return;
						}
						/* not skip */
						if ((t - r) == 2) {
							///*
							zero_time[!lvl] = (com_time>>1) + (zero_time[!lvl]>>1);
							//*/
							bit = 0;
							skip = 0;
							r = 2;
						} else if ((t - r) == 1) {
							//zero_time[!lvl] = com_time << 1;
							bit = 1;
							skip = 1;
							r = 9;
						} else {
							///*
							usart_put_char('0' + t);
							usart_put_char('0' + r);
							//*/
							usart_put_string("t-r too large\n");
							usart_put_char('0'+!lvl);
							usart_put_int16(com_time);
							output();
							STATE = S_READ;
							return;
						}
						put_bit(bit);
						TCNT1 = 0;
					}
				}
			}
		} else {
			++ count_identic;
		}
	}
}


void
delay(uint32_t us)
{
	uint16_t count;
	while (us) {
		count = 2;
		while (count --)
			;
		-- us;
	}
}


int
main(void)
{
	usart_init();
	init_timer_for_pwm();
	DDRB |= _BV(PB_DIR_A) | _BV(PB_DIR_B);
	PORTB |= _BV(PB_READ_BUTTON);
	sei();
	count = 0;
	bit_count = 0;
	zero_count = 0;
	count_identic = 0;
	to_print = 0;
	count_adc = 0;
	zero_time[0] = 0;
	zero_time[1] = 0;
	count_com = 0;
	skip = 0;
	STATE = S_INIT;
	buf[0] = 0;
	for (;;) {
		if (STATE == S_INIT) {
			loop_until_bit_is_clear(PINB, PB_READ_BUTTON);
			init_adc();
			
			//init_timer_for_adc();
			
			START_MOTOR_DIR_B;
			STATE = S_READING;

		} else {
			if (STATE == S_READING) {
				ADCSRA |= _BV(ADSC);
			} else {
				stop_adc();
				loop_until_bit_is_clear(PINB, PB_READ_BUTTON);
			}
		}
	}
}
