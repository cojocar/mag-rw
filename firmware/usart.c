/*
 * ==========================================================================
 *
 *       Filename:  usart.c
 *
 *    Description:  usart api
 *
 *        Created:  05/04/09 13:30:29
 *       Revision:  none
 *
 *         Author:  Cojocar Lucian, cojocar-gmail-com
 *
 * ==========================================================================
 */
#include <avr/io.h>

#include "config.h"
#include "usart.h"

#ifdef UART_BAUD_RATE
#	define UART_BAUD_CALC(UART_BAUD_RATE,F_OSC) ((F_OSC)/((UART_BAUD_RATE)*16l)-1)
#else
#	error "define baud rate!"
#endif

static uint16_t buf_pos;
static uint8_t	usart_buffer[USART_BUF_SIZE];

void
usart_init(void)
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
	buf_pos = 0;
}

inline void
usart_put_char(uint8_t c) {
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
}

inline void
usart_put_string(char *s)
{
	while (*s) {
		usart_put_char(*s);
		++ s;
	}
}

inline void
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

inline void
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

inline void
usart_buf_put_char(uint8_t x)
{
	usart_buffer[buf_pos ++] = x;
	if (buf_pos == USART_BUF_SIZE) {
		usart_put_string("UBUF_FULL");
		usart_buf_print();
		buf_pos = 0;
	}
}

void
usart_buf_print(void)
{
	uint16_t i;
	for (i = 0; i < buf_pos; ++ i) {
		usart_put_char(usart_buffer[i]);
	}
	usart_put_char('\n');
	buf_pos = 0;
}

inline void
usart_buf_put_string(char *s)
{
	while (*s) {
		usart_buf_put_char(*s);
		++ s;
	}
}


void
usart_buf_put_int16(uint16_t x)
{
	 uint8_t cif[10], i;
	/* 
	 * itoa function
	 */
	usart_buf_put_char('0');
	i = 0;
	while (x) {
		cif[i] = x % 10;
		++ i;
		x /= 10;
	}
	while (i--) {
		usart_buf_put_char(cif[i] + '0');
	}
}

inline uint16_t
usart_buf_get_pos(void)
{
	return buf_pos;
}
