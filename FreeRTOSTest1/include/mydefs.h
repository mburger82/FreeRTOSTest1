//
// file: mydefs.h
//
// xmega demo program VG Feb,Mar,Oct 2011
//
// CPU:   atxmega32A4, atxmega128a1
//
// Version: 1.2.1
//
// configuration parameters for the demo application
//
#ifndef _mydefs_h_
#define _mydefs_h_

#define RUNTIME_CHECKS			1	// reduces power consumption if disabled ! 

#define SERIAL_CHECK_SEND		1   // Check datagrams while sending (works with and without simulation mode).
									// This is for testing only, it adds runtime overhead and no functionality!

#define SERIAL_PORT_BUFFER_LEN	100  // each serial port has a buffer queue
#define CHANNEL_QUEUE_LENGTH	200  // each channel has output queue

// These defines are for the real UARTs
// The interrupt level used here MUST NOT be higher than configMAX_SYSCALL_INTERRUPT_PRIORITY !
// Other than the listed (and out-commented) options are not allowed.
//
#if (configMAX_SYSCALL_INTERRUPT_PRIORITY == 0)
	#define RX_INTRPT_LEVEL 	USART_RXCINTLVL_LO_gc // serial receiver
	#define TX_INTRPT_LEVEL 	USART_DREINTLVL_LO_gc // serial transmitter
#elif (configMAX_SYSCALL_INTERRUPT_PRIORITY == 1)
	#define RX_INTRPT_LEVEL 	USART_RXCINTLVL_LO_gc // serial receiver
//	#define RX_INTRPT_LEVEL 	USART_RXCINTLVL_MED_gc // serial receiver
	#define TX_INTRPT_LEVEL 	USART_DREINTLVL_LO_gc // serial transmitter
//	#define TX_INTRPT_LEVEL 	USART_DREINTLVL_MED_gc // serial transmitter
#elif (configMAX_SYSCALL_INTERRUPT_PRIORITY == 2)
	#define RX_INTRPT_LEVEL 	USART_RXCINTLVL_LO_gc // serial receiver
//	#define RX_INTRPT_LEVEL 	USART_RXCINTLVL_MED_gc // serial receiver
//	#define RX_INTRPT_LEVEL 	USART_RXCINTLVL_HI_gc // serial receiver
	#define TX_INTRPT_LEVEL 	USART_DREINTLVL_LO_gc // serial transmitter
//	#define TX_INTRPT_LEVEL 	USART_DREINTLVL_MED_gc // serial transmitter
//	#define TX_INTRPT_LEVEL 	USART_DREINTLVL_HI_gc // serial transmitter
#endif

// Define the interrupt level that is used for the serial
// simulation via timer interrupts.
#define	REC_TIMER_INTERRUPT_LEVEL		USART_DREINTLVL_LO_gc  //TC_OVFINTLVL_MED_gc

// Define the baudrates for the different serial input lines
// in the simulation mode.
//
#define USARTC1_BAUDRATE	 57600
#define USARTD1_BAUDRATE	 57600
#define USARTE0_BAUDRATE	 57600

#define DG_EOM ((u8)0x0a)


// for easier type writing:

typedef unsigned char  u8;
typedef   signed char  s8;
typedef unsigned short u16;
typedef   signed short s16;
typedef unsigned long  u32;
typedef   signed long  s32;


#endif // _mydefs_h_ 
