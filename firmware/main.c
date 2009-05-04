#include <avr/io.h>
#include <avr/interrupt.h>

/*
#define F_OSC 4000000
#define UART_BAUD_RATE 4800
#define UART_BAUD_CALC(UART_BAUD_RATE,F_OSC) ((F_OSC)/((UART_BAUD_RATE)*16l)-1)

#define SPIDI	6	// Port B bit 6 (pin7): data in (data from MMC)
#define SPIDO	5	// Port B bit 5 (pin6): data out (data to MMC)
#define SPICLK	7	// Port B bit 7 (pin8): clock
#define SPICS	4	// Port B bit 4 (pin5: chip select for MMC
*/

#define INIT_SECT	0	/* sectorul initial de la care scriu */
#define CARD_BUFF 	512	/* numarul de octeti dintr-un sector < 512 */
//#define CARD_BUFF 	8	/* numarul de octeti dintr-un sector < 512 */

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

void
pwm_init0(void)
{
	/* CTC, toggle */
	TCCR0 = _BV(WGM01) | _BV(COM00);
	/* prescallerul, 1024 */
	TCCR0 |= _BV(CS00) | _BV(CS02); 

	/* 30 HZ */
	OCR0 = 207;
	/**/ 
	OCR0 = 207;

	DDRB |= _BV(PB3);
}

void
pwm_init1(void)
{
	/* CTC, toggle */

	TCCR1A = _BV(COM1A0);
	TCCR1B = _BV(WGM12);
	/* prescallerul, 1024 */
	TCCR1B |= _BV(CS10) | _BV(CS12); 

	
	//OCR1A = 0xffff;
	OCR1A = 0x0020;

	DDRD |= _BV(PB5);
}

int
main(void)
{
	pwm_init1();
	DDRD |= _BV(PB0);
	for (;;) {
	}
	return 0;
}
