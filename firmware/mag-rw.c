
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
uint16_t min, max;

uint8_t last_lvl;
uint32_t count_identic;
uint64_t count_adc;
uint32_t zero_count;
uint8_t to_print;
uint16_t zero_time[2];
uint16_t unu_time[2];

void
start_comp(void)
{
	ACSR |= _BV(ACIE);
}

void
stop_comp(void)
{
	ACSR &= ~_BV(ACIE);
}

void stop_adc(void);

void
init_comp(void)
{
	stop_adc();
	/*
	 * in loc de AIN1 folosim ADC1
	 */
	SFIOR = _BV(ACME);
	ADMUX = _BV(MUX1);
	ACSR = 0;//_BV(ACIC);
}

ISR(SIG_INPUT_CAPTURE1)
{
	volatile uint16_t c;
	c = ICR1;
	ICR1 = 0;
	TIFR |= _BV(ICF1);
	usart_buf_put_int16(c);
	usart_buf_put_char('#');
	if (usart_buf_get_pos() >= USART_BUF_SIZE - 20) {
		STOP_MOTOR;
		stop_adc();
		stop_comp();
		stop_timer_for_adc();
		usart_buf_print();
	}
}

void
init_adc(void)
{
  /*
   * right adjusted
   * adc1
   * use avcc
   */
  //ADMUX = _BV(REFS0) | _BV(MUX0);
  /*
   * intrare diferentiala
   * pe ADC1 -
   * si ADC2 +
   */
  ADMUX = _BV(REFS0) | _BV(MUX0);
  /*
  ADMUX = _BV (REFS0) | _BV (MUX0) | _BV (MUX1) | _BV (MUX2) | _BV (ADLAR);
  */
 
  /*
   * enable adc
   * prescaler 32  => ~33KHz
   * intreruperi
   */
  ADCSRA = _BV(ADPS1) | /* _BV(ADPS0) |*/ _BV(ADIE) | _BV(ADSC) | _BV(ADEN);  
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
	///*
	usart_put_string("START");
	for (i = 0; i <= count; ++ i) {
		usart_put_char((uint8_t)buf[i]);
	}
	//*/
	usart_buf_print();
	///*
	/*
	usart_put_int16(zero_time[0]);
	usart_put_char(' ');
	usart_put_int16(zero_time[1]);
	usart_put_char(' ');
	usart_put_int32(count);
	usart_put_char(' ');
	usart_put_char('0' + bit_count);
	usart_put_char('m');
	usart_put_int16(min);
	usart_put_char('M');
	usart_put_int16(max);
	usart_put_char('\n');
	*/
	count = 0;
	bit_count = 0;
}


SIGNAL(SIG_OVERFLOW1) 
{
	/* adc timer overflow */
	STOP_MOTOR;
	usart_put_string("OVERFLOW\n");
	if (zero_count >= ZEROS1) {
		output();
	}

	//STATE = S_INIT;
}


inline void
put_bit(uint8_t bit)
{
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
uint16_t rem;

#define TRESH 0
ISR(SIG_COMPARATOR)
{
	volatile uint16_t com_time;
	//TCNT1 = 0x0;
	com_time = TCNT1;
	TCNT1 = 0;
	usart_buf_put_int16(com_time);
	usart_buf_put_char('v');
	if (usart_buf_get_pos() >= USART_BUF_SIZE - 20) {
		STOP_MOTOR;
		stop_adc();
		stop_comp();
		stop_timer_for_adc();
		usart_buf_print();
	}
}

#if 0
SIGNAL(SIG_ADC)
{
	
	uint8_t bit;

	++ count_adc;
	uint8_t lvl;
	uint16_t com_time;
	volatile uint16_t v;
	com_time = TCNT1;
	v = ADC;
	#if 0
	if (v > max/* && zero_count < ZEROS*/) {
		max = v;
	}
	if (v < min /* && zero_count < ZEROS*/) {
		min = v;
	}
	if (v > ((max-min)>>1)+min) {
		//max = v;
		lvl = 1;
	} else {
		//min = v;
		lvl = 0;
	}
	#endif
	#define L	0xff
	#define LL	0x300	/* 0x3ff - 0xff */
	lvl = last_lvl;
	if (v < 0x200) {
		if (v > L) {
			lvl = 1;
		} /*else if (zero_count < ZEROS) {
			lvl = last_lvl;
		} else {
			lvl = 1;
			return;
		}*/
	} else {
		if (v < (LL)) {
			lvl = 0;
		} /*else if (zero_count < ZEROS) {
			lvl = last_lvl;
		} else {
			lvl = 0;
			return;
		}*/
	}
	if (lvl != last_lvl) {
		if (rem == 0) {
			usart_buf_put_int16(count_identic);
			//usart_buf_put_int16(com_time);
			usart_buf_put_char('#');
		} else {
			-- rem;
		}
		count_identic = 0;
		last_lvl = lvl;
	} else {
		++ count_identic;
	}
	return;
	if (lvl != last_lvl) {
		if (zero_count >= ZEROS1) {
			if (rem == 0) {
				//usart_buf_put_int16(count_identic);
				usart_buf_put_int16(com_time);
				usart_buf_put_char('#');
			} else {
				-- rem;
			}
			if (usart_buf_get_pos() >= USART_BUF_SIZE - 20) {
				STOP_MOTOR;
				stop_adc();
				stop_adc();
				stop_timer_for_adc();
				usart_buf_print();
			}
			last_lvl = lvl;
			TCNT1 = 0;
			return;
		} else {
			++ zero_count;
		}
	} else {
		TCNT1 = 0;
		return;
	}
	#if 0
	usart_buf_put_int16(v);
	usart_buf_put_char('v');
	if (usart_buf_get_pos() >= USART_BUF_SIZE - 20) {
		stop_adc();
		stop_timer_for_adc();
		STOP_MOTOR;
		usart_buf_print();
	}
	#endif


	
	if (lvl == last_lvl) {
		if (count_identic < 0xffffffff) {
			++ count_identic;
		}
	} else {
		++ count_diferit;
		count_diferit = 30;
		//if (com_time > 30 || zero_count <= ZEROS) {
		if (count_diferit >= TRESH) {
			count_diferit = 0;
			last_lvl = lvl;

		/* a comutat */
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
					init_timer_for_adc();
					TCNT1 = 0;
					++ zero_count;
				} else {
					if (zero_count < ZEROS1) {
						r = 2;
						++ zero_count;
						zero_time[!lvl] = com_time; //(zero_time[!lvl]>>1) + (TCNT1>>1);
						///*
						usart_buf_put_int16(com_time);
						usart_buf_put_char('!');
						//*/
						TCNT1 = 0;
					} else {
						/*
						if (rem == 0) {
							usart_buf_put_int16(count_identic);
							//usart_buf_put_int16(com_time);
							usart_buf_put_char('#');
						} else {
							-- rem;
						}
						if (usart_buf_get_pos() >= USART_BUF_SIZE - 20) {
							STOP_MOTOR;
							stop_adc();
							stop_adc();
							stop_timer_for_adc();
							usart_buf_print();
						}
						*/
						TCNT1 = 0;
						if (com_time <= ((zero_time[!lvl]>>1) + ((zero_time[!lvl]>>9)))) {
							t = 2;
						} else if (com_time >= ((zero_time[!lvl]) - (zero_time[!lvl]>>4))){
							t = 4;
						} else {
							t = 3;
						}
						if (skip) {
							skip = 0;
							r = 1;
							count_identic = 0;
							TCNT1 = 0;
							return;
						}
						/* not skip */
						#if 0
						if ((t - r) == 2) {
							zero_time[!lvl] = (com_time>>1) + (zero_time[!lvl]>>1);
							bit = 0;
							skip = 0;
							r = 2;
						} else if ((t - r) == 1) {
							zero_time[!lvl] = com_time << 1;
							bit = 1;
							skip = 1;
							r = 9;
						} else {
							/*
							usart_put_char('0' + t);
							usart_put_char('0' + r);
							usart_put_string("t-r to large\n");
							usart_put_char('0'+!lvl);
							usart_put_int16(com_time);
							output();
							*/
							stop_adc();
							stop_timer_for_adc();
							usart_buf_print();
							STOP_MOTOR;
							STATE = S_READ;
							TCNT1 = 0;
							count_identic = 0;
							return;
						}
						put_bit(bit);
						#endif
					}
					count_identic = 0;
					TCNT1 = 0;
					return;
				}
			}

			count_identic = 0;
		} else {
			++ count_identic;
		}
	}
}

#endif

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
	STOP_MOTOR;
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
	min = 0xffff;
	max = 0x0000;
	//rem = 200;
	rem = 0;
	START_MOTOR_DIR_B;
	for (;;) {
		if (STATE == S_INIT) {
			loop_until_bit_is_clear(PINB, PB_READ_BUTTON);
			//loop_until_bit_is_set(PINB, PB_READ_BUTTON);
			init_adc();
			//stop_adc();
			init_comp();
			start_comp();
			
			init_timer_for_adc();
			//TIMER_ADC_RESET;
			
			//START_MOTOR_DIR_B;
			//START_MOTOR_DIR_A;
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
