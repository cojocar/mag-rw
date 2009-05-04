/*
 * ==========================================================================
 *
 *       Filename:  cad.c
 *
 *    Description:  Convertor anaalog digital
 *
 *        Created:  04/30/09 08:50:59
 *       Revision:  none
 *
 *         Author:  Cojocar Lucian, cojocar-gmail-com
 *
 * ==========================================================================
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_OSC 16000000           /* oscillator-frequency in Hz */
#define UART_BAUD_RATE 9600
#define UART_BAUD_CALC(UART_BAUD_RATE,F_OSC) ((F_OSC)/((UART_BAUD_RATE)*16l)-1)

uint16_t count;

void init_timer(void);


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
init_uart(void)
{
	UBRRH = (uint8_t)(UART_BAUD_CALC(UART_BAUD_RATE,F_OSC)>>8);
	UBRRL = (uint8_t)UART_BAUD_CALC(UART_BAUD_RATE,F_OSC);

	/*
	 * Enable receiver and transmitter; enable RX interrupt
	 */
	UCSRB = (1 << RXEN) | (1 << TXEN);

	/*
	 * asynchronous 8N1
	 */
	UCSRC = (1 << URSEL) | (3 << UCSZ0);
}

void
usart_put_char(uint8_t c) {
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
}

void
usart_put_string(char *s)
{
	while (*s) {
		usart_put_char(*s);
		++ s;
	}
}

void
usart_put_int16(uint16_t x)
{
	 uint8_t cif[10], i;
	/* 
	 * itoa function
	 */
	usart_put_char('0');
	i = 0;
	while (x) {
		cif[i] = x % 10;
		++ i;
		x /= 10;
	}
	while (i--) {
		usart_put_char(cif[i] + '0');
	}
}
void
usart_put_int32(uint32_t x)
{
	 uint8_t cif[10], i;
	/* 
	 * itoa function
	 */
	usart_put_char('0');
	i = 0;
	while (x) {
		cif[i] = x % 10;
		++ i;
		x /= 10;
	}
	while (i--) {
		usart_put_char(cif[i] + '0');
	}
}

#if 0
void
usart_put_int64(uint64_t x)
{
	 uint8_t cif[20], i;
	/* 
	 * itoa function
	 */
	usart_put_char('0');
	i = 0;
	while (x) {
		cif[i] = x % 10;
		++ i;
		x /= 10;
	}
	while (i--) {
		usart_put_char(cif[i] + '0');
	}
}
#endif

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

#define TRESH	0
#define ZEROS	10
#define ZEROS1	(ZEROS+30)

SIGNAL(SIG_OVERFLOW1) 
//SIGNAL(_VECTOR(16)) 
{
	if (zero_count >= ZEROS1) {
		usart_put_string("caca\n");
	}
}

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
					init_timer();
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
init_timer(void)
{
	TCCR1A;
	/* prescaler 1, intr-o ms nu ar trebui sa avem overflow */
	/* dau enable intreruperi */
	TIMSK = _BV(TOIE1);
	TCCR1B = _BV(CS10) | _BV(CS11);
	//TCCR1B = _BV(CS12);

}

int
main(void)
{
	init_uart();
	init_adc();
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
	/*
	ADCSRA |= _BV(ADEN);
	*/
	for (;;) {
		ADCSRA |= _BV(ADSC);
		///*
		//loop_until_bit_is_clear(ADCSRA, ADSC);
		//*/
		/*
		usart_put_string("i:");
		usart_put_int16(count ++);
		usart_put_char('\n');
		*/
		/*
		usart_put_string("Gata conversie\n");
		*/

	}
}
