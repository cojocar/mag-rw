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
	S_READING
} STATE;

uint16_t count;

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




#define BUF_SZ	32
uint8_t buf[BUF_SZ];
uint8_t bit_count;
uint16_t com_tt[20];
uint8_t count_com;

uint8_t last_lvl;
uint32_t count_identic;
uint64_t count_adc;
uint8_t have_to_wait;
uint8_t zero_count;
uint32_t zero_samples;
uint32_t zero_samples_init;
uint8_t to_print;
uint16_t zero_time[2];

SIGNAL(SIG_OVERFLOW1) 
{
	/* adc timer overflow */
	if (zero_count >= ZEROS1) {
		usart_put_string("caca\n");
	}
	STOP_MOTOR;
}
#define TRESH 0
SIGNAL(SIG_ADC)
{
	
	uint8_t bit;
	int i;

	++ count_adc;
	uint8_t lvl;
	uint16_t com_time;
	if (ADC > 512) {
		lvl = 1;
	} else {
		lvl = 0;
	}
	if (lvl == last_lvl) {
		if (count_identic < 0xffffffff) {
			++ count_identic;
		}
	} else {
		if (count_identic >= TRESH) {
			last_lvl = lvl;

		/* a comutat */
			if (zero_count < ZEROS) {
				/*
				 * sunt intr-un sir de zerouri,
				 * salvez cate esantionari fac
				 * ar trebui sa am minim doi biti de 1
				 */
				zero_samples = count_identic;
				zero_samples_init = count_identic;
				++ zero_count;
			} else {
				/* deja am trecut de zerouri, urmeaza sa folosesc timerul */
				if (zero_count == ZEROS) {
					/* ratam un zero */
					/* de aici trecem la timer */
					//usart_put_string("dau drumul la timer\n");
					init_timer_for_adc();
					TCNT1 = 0;
					++ zero_count;
				} else {
					if (zero_count < ZEROS1) {
						zero_time[!lvl] = TCNT1;
						TCNT1 = 0;
						++ zero_count;
					} else {
						com_time = TCNT1;
						TCNT1 = 0;
						switch (!lvl) {
							case 0:
								if (com_time >= (zero_time[0] - (zero_time[0]>>2))) {
									/* nu e Y */
									bit = 0;
									zero_time[0] = (com_time + zero_time[0]) >> 1;
								} else {
									if (!to_print) {
										/* avem un bit de 1, primul front */
										/* Y */
										to_print = 1;
										bit = 1;
									} else {
										to_print = 0;
										bit = 1;
										/*
										zero_time[0] = com_time << 1;
										*/
										#if 0
										if (com_time >= (zero_time[1] - (zero_time[0]>>1))) {
											/* eroare */
											for (i = 0; i < count; ++ i) {
												usart_put_char(buf[i]);
											}
											usart_put_int16(count);
											usart_put_char(' ');
											usart_put_int16(zero_time[0]);
											usart_put_char(' ');
											usart_put_int16(zero_time[1]);
											usart_put_string("ERRB\n");
										}
										#endif
									}
								}
								break;
							case 1:
								if (com_time >= (zero_time[1] - (zero_time[0]>>2))) {
									/* nu X */
									bit = 0;
									zero_time[1] = (com_time + zero_time[1]) >> 1;
								} else {
									if (to_print) {
										/* X */
										to_print = 0;
										bit = 1;
									} else {
										/* A */
										to_print = 1;
										bit = 1;
										/*
										zero_time[1] = com_time << 1;
										*/
										/*
										if (com_time >= (zero_time[1] - (zero_time[1]>>2))) {
											for (i = 0; i < count; ++ i) {
												usart_put_char(buf[i]);
											}
											usart_put_int16(count);
											usart_put_char(' ');
											usart_put_int16(zero_time[0]);
											usart_put_char(' ');
											usart_put_int16(zero_time[1]);
											usart_put_string("ERRA\n");
										}
										*/
									}
								}
								break;
							default:
								usart_put_string("XXXX\n");
						}

						if (zero_count >= ZEROS1) {
							if (bit != 1 || to_print) {
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
									for (i = 0; i < BUF_SZ; ++ i) {
										usart_put_char(buf[i]);
									}
									usart_put_int16(com_time);
									usart_put_char(' ');
									usart_put_int16(zero_time[0]);
									usart_put_char(' ');
									usart_put_int16(zero_time[1]);
									usart_put_char(' ');
									usart_put_int32(count_adc);
									usart_put_char('\n');
									count = 0;
								}
							}
						}
					}
				}
			}
		} else {
			++ count_identic;
			last_lvl = !lvl;
		}
	}
}


void
delay(uint8_t us)
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
	sei();
	count = 0;
	bit_count = 0;
	zero_count = 0;
	have_to_wait = 0;
	count_identic = 0;
	to_print = 0;
	count_adc = 0;
	zero_time[0] = 0;
	zero_time[1] = 0;
	count_com = 0;
	STATE = S_INIT;
	for (;;) {
		if (STATE == S_INIT) {
			loop_until_bit_is_clear(PINA, PB_READ_BUTTON);
			START_MOTOR_DIR_A;
			STATE = S_READING;
			delay(100);

			STOP_MOTOR;
		} else {
			ADCSRA |= _BV(ADSC);
		}
	}
}
