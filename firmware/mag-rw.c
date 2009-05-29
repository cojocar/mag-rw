
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
volatile uint16_t period;
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
	ACSR = 0; //_BV(ACIC);
}

ISR(SIG_INPUT_CAPTURE1)
{
	volatile uint16_t c;
	volatile uint8_t sreg;

	sreg = SREG;
	cli();
	c = ICR1;
	ICR1 = 0;
	SREG = sreg;

//	TIFR |= _BV(ICF1);
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
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX1);
  /*
  ADMUX = _BV (REFS0) | _BV (MUX0) | _BV (MUX1) | _BV (MUX2) | _BV (ADLAR);
  */
 
  /*
   * enable adc
   * prescaler 32  => ~33KHz
   * intreruperi
   */
  ADCSRA = _BV(ADPS0) | _BV(ADPS0) | _BV(ADIE) | _BV(ADEN) | _BV(ADSC);
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
	usart_put_string("START");
	for (i = 0; i <= count; ++ i) {
		usart_put_char((uint8_t)buf[i]);
	}
	//*/
	//usart_buf_print();
	///*
	/*
	usart_put_int16(zero_time[0]);
	usart_put_char(' ');
	usart_put_int16(zero_time[1]);
	usart_put_char(' ');
	*/
	usart_put_int32(count);
	usart_put_char(' ');
	usart_put_char('0' + bit_count);
	usart_put_char(' ');
	usart_put_int16(period);
	/*
	usart_put_char('m');
	usart_put_int16(min);
	usart_put_char('M');
	usart_put_int16(max);
	usart_put_char('\n');
	*/
	usart_buf_print();
	count = 0;
	bit_count = 0;
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

ISR(SIG_OVERFLOW1)
{
	if (zero_count > ZEROS1) {
	} else {
		STOP_MOTOR;
		usart_put_string("OVERFLOW\n");
	}
}

ISR(SIG_ADC)
{
	volatile uint8_t lvl;
	volatile uint16_t com_time;
	volatile uint16_t v;
	com_time = TCNT1;
	v = ADC;
	#define L	0xff
	#define LL	0x300	/* 0x3ff - 0xff */
	lvl = last_lvl;
	if (v < 0x200) {
		if (v > L) {
			lvl = 1;
		} else if (zero_count > ZEROS1) {
			lvl = 1;
			usart_buf_put_char('X');
		}
		/*else if (zero_count < ZEROS) {
			lvl = last_lvl;
		} else {
			lvl = 1;
			return;
		}*/
	} else {
		if (v < (LL)) {
			lvl = 0;
		} else if (zero_count > ZEROS1) {
			lvl = 0;
			usart_buf_put_char('Y');
		}
		/*else if (zero_count < ZEROS) {
			lvl = last_lvl;
		} else {
			lvl = 0;
			return;
		}*/
	}
	if (zero_count < ZEROS) {
		if (lvl != last_lvl) {
			++ zero_count;
		}
	} else if (zero_count == ZEROS) {
		init_timer_for_adc();
		++ zero_count;
	} else if (zero_count < ZEROS1) {
		if (lvl != last_lvl) {
			++ zero_count;
			if (period == 0) {
				period = com_time;
			} else {
				period = (com_time>>1) + (period>>1);
				//period = com_time;
			}
			/*
			usart_buf_put_int16(period);
			usart_buf_put_int16(v);
			usart_buf_put_string("V\n");
			*/
			TCNT1 = 0;
		}
	} else if (zero_count == ZEROS1) {
		if (lvl != last_lvl) {
			SFIOR |= _BV(ADTS2) | _BV(ADTS1);
			ADCSRA |= _BV(ADATE);
			TCNT1 = 0xffff - period - (period>>3);
			++ zero_count;
		}
	} else if (zero_count > ZEROS1) {
		///*
		usart_buf_put_int16(v);
		usart_buf_put_string("V\n");
		if (usart_buf_get_pos() >= USART_BUF_SIZE - 20) {
			STOP_MOTOR;
			stop_adc();
			stop_comp();
			stop_timer_for_adc();
			usart_buf_print();
		}
		//*/
		if (lvl != last_lvl) {
			put_bit(0);
		} else {
			put_bit(1);
		}
		TCNT1 = 0xffff - period;
	}
	last_lvl = lvl;
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
	period = 0;
	for (;;) {
		if (STATE == S_INIT) {
			loop_until_bit_is_clear(PINB, PB_READ_BUTTON);
			init_adc();
			START_MOTOR_DIR_B;
			STATE = S_READING;
		} else {
			if (STATE == S_READING) {
				if (zero_count <= ZEROS1) {
					ADCSRA |= _BV(ADSC);
				}
			} else {
				stop_adc();
				loop_until_bit_is_clear(PINB, PB_READ_BUTTON);
			}
		}
	}
}
