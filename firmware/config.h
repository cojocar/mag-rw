/*
 * ==========================================================================
 *
 *       Filename:  config.h
 *
 *    Description:  configuration file
 *
 *        Created:  05/04/09 13:43:44
 *       Revision:  none
 *
 *         Author:  Cojocar Lucian, cojocar-gmail-com
 *
 * ==========================================================================
 */


#ifndef  CONFIG_INC
#define  CONFIG_INC

#define PWM_SPEED	0x24
#define F_OSC 16000000           /* oscillator-frequency in Hz */
#define UART_BAUD_RATE 9600
#define ZEROS	10
#define ZEROS1	(ZEROS+10)

#define PB_READ_BUTTON	PB0
#define PB_DIR_A		PB1
#define PB_DIR_B		PB2

#define START_MOTOR_DIR(a)	PORTB |= _BV((a))
#define START_MOTOR_DIR_A	START_MOTOR_DIR(PB_DIR_A)
#define START_MOTOR_DIR_B	START_MOTOR_DIR(PB_DIR_B)
#define STOP_MOTOR			do {PORTB &= ~(_BV(PB_DIR_A) | _BV(PB_DIR_B));} while (0);

#endif   /* ----- #ifndef CONFIG_INC  ----- */
