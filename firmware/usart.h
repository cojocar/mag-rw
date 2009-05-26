/*
 * ==========================================================================
 *
 *       Filename:  usart.h
 *
 *    Description:  usart header
 *
 *        Created:  05/04/09 14:24:40
 *       Revision:  none
 *
 *         Author:  Cojocar Lucian, cojocar-gmail-com
 *
 * ==========================================================================
 */


#ifndef  USART_INC
#define  USART_INC

void usart_init(void);
void usart_put_char(uint8_t);
void usart_put_int16(uint16_t);
void usart_put_string(char *);
void usart_put_int32(uint32_t);

void usart_buf_put_int16(uint16_t);
void usart_buf_put_string(char *);
void usart_buf_print(void);
void usart_buf_put_char(uint8_t);

#define USART_BUF_SIZE	128

#endif   /* ----- #ifndef USART_INC  ----- */
