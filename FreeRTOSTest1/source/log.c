/*
 * log.c
 *
 * Created: 18.12.2017 18:01:37
 *  Author: Martin Burger
 */ 
#include <stdarg.h>
//#include <stdio.h>
#include <string.h>

#include "all.h" 

static void ftoa_fixed(char *buffer, double value);
static void ftoa_sci(char *buffer, double value);
static int my_vprintf(char const *fmt, va_list arg);
static void usart_setup(USART_data_t* usartData, USART_t * usart,s16 bsel, s8 bscale,dirctrl_t dirctrl,USART_DREINTLVL_t recIntLevel,USART_DREINTLVL_t dreIntLevel);
USART_data_t USARTC0_data;
extern xTaskHandle hEdulog;

xQueueHandle vEdulog_channelQueue;

void vEdulogtask(void *pvParameters) {
	(void) pvParameters;
	uint8_t      dataByte;
	for(;;) {
		if(xQueueReceive(vEdulog_channelQueue, &dataByte, comNO_BLOCK)) {
			
			if(!USART_TXBuffer_PutByte(&USARTC0_data, dataByte, portMAX_DELAY))
				error(ERR_QUEUE_SEND_FAILED);
		}		
	}
}

void initEdulog(void) {
	//-----------------------------
	// USARTC0 setup ( RXD0, TXD0 )
	/* PC3 (TXD0) as output. */
	PORTC.DIRSET = PIN3_bm;
	/* PC2 (RXD0) as input. */
	PORTC.DIRCLR = PIN2_bm;
	usart_setup( &USARTC0_data, &USARTC0, BSEL_57600, BSCALE_57600, receivetransmit,RX_INTRPT_LEVEL,TX_INTRPT_LEVEL);
	//-----------------------------
	// Init Edulog Queue
	const unsigned portBASE_TYPE chQueueLength = CHANNEL_QUEUE_LENGTH;
	if((vEdulog_channelQueue = xQueueCreate(chQueueLength, (unsigned portBASE_TYPE) sizeof(signed char))) == NULL)
	error(ERR_QUEUE_CREATE_HANDLE_NULL);

	//-----------------------------
	// Create Edulog Task
	xTaskCreate( vEdulogtask, (const char *) "edulogTask", configMINIMAL_STACK_SIZE, NULL, 1,&hEdulog);
}

int edulog(char const *fmt, ...) {
    va_list arg;
    int length;

    va_start(arg, fmt);
    length = my_vprintf(fmt, arg);
    va_end(arg);
    return length;
}


static int my_vprintf(char const *fmt, va_list arg) {
	int int_temp;
	char char_temp;
	char *string_temp;
	double double_temp;

	BaseType_t xHigherPriorityTaskWoken;

	char ch;
	int length = 0;

	char buffer[30];
	char str[100];

	xHigherPriorityTaskWoken = pdFALSE;

	while ( ch = *fmt++) {
		if ( '%' == ch ) {
			switch (ch = *fmt++) {
				/* %% - print out a single %    */
				case '%':
				str[length] = '%';
				length++;
				break;

				/* %c: print out a character    */
				case 'c':
				char_temp = va_arg(arg, int);
				str[length] = char_temp;
				//fputc(char_temp, file);
				length++;
				break;

				/* %s: print out a string       */
				case 's':
				string_temp = va_arg(arg, char *);
				for(int i = 0; i < strlen(string_temp);i++) {
					str[length+i] = string_temp[i];
				}
				//fputs(string_temp, file);
				length += strlen(string_temp);
				break;

				/* %d: print out an int         */
				case 'd':
				int_temp = va_arg(arg, int);
				itoa(int_temp, buffer, 10);
				for(int i = 0; i < strlen(buffer);i++) {
					str[length+i] = buffer[i];
				}
				//fputs(buffer, file);
				length += strlen(buffer);
				break;

				/* %x: print out an int in hex  */
				case 'x':
				int_temp = va_arg(arg, int);
				itoa(int_temp, buffer, 16);
				for(int i = 0; i < strlen(buffer);i++) {
					str[length+i] = buffer[i];
				}
				//fputs(buffer, file);
				length += strlen(buffer);
				break;

				case 'f':
				double_temp = va_arg(arg, double);
				ftoa_fixed(buffer, double_temp);
				for(int i = 0; i < strlen(buffer);i++) {
					str[length+i] = buffer[i];
				}
				//fputs(buffer, file);
				length += strlen(buffer);
				break;

				case 'e':
				double_temp = va_arg(arg, double);
				ftoa_sci(buffer, double_temp);
				for(int i = 0; i < strlen(buffer);i++) {
					str[length+i] = buffer[i];
				}
				//fputs(buffer, file);
				length += strlen(buffer);
				break;
			}
		}
		else {
			str[length] = ch;
			if(str[length] == '\n') {
				str[length] = 0x0D;
				length++;
				str[length] = '\n';
			}
			//putc(ch, file);
			length++;
		}
	}
	str[length] = '\0';
	length++;
	for(int i = 0; i < length;i++) {
		xQueueSendFromISR(vEdulog_channelQueue, &str[i], &xHigherPriorityTaskWoken);
	}
	
	
	return length;
}

int normalize(double *val) {
    int exponent = 0;
    double value = *val;

    while (value >= 1.0) {
        value /= 10.0;
        ++exponent;
    }

    while (value < 0.1) {
        value *= 10.0;
        --exponent;
    }
    *val = value;
    return exponent;
}

static void ftoa_fixed(char *buffer, double value) {  
    /* carry out a fixed conversion of a double value to a string, with a precision of 5 decimal digits. 
     * Values with absolute values less than 0.000001 are rounded to 0.0
     * Note: this blindly assumes that the buffer will be large enough to hold the largest possible result.
     * The largest value we expect is an IEEE 754 double precision real, with maximum magnitude of approximately
     * e+308. The C standard requires an implementation to allow a single conversion to produce up to 512 
     * characters, so that's what we really expect as the buffer size.     
     */

    int exponent = 0;
    int places = 0;
    static const int width = 4;

    if (value == 0.0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }         

    if (value < 0.0) {
        *buffer++ = '-';
        value = -value;
    }

    exponent = normalize(&value);

    while (exponent > 0) {
        int digit = value * 10;
        *buffer++ = digit + '0';
        value = value * 10 - digit;
        ++places;
        --exponent;
    }

    if (places == 0)
        *buffer++ = '0';

    *buffer++ = '.';

    while (exponent < 0 && places < width) {
        *buffer++ = '0';
        --exponent;
        ++places;
    }

    while (places < width) {
        int digit = value * 10.0;
        *buffer++ = digit + '0';
        value = value * 10.0 - digit;
        ++places;
    }
    *buffer = '\0';
}

void ftoa_sci(char *buffer, double value) {
    int exponent = 0;    
    static const int width = 4;

    if (value == 0.0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0.0) {
        *buffer++ = '-';
        value = -value;
    }

    exponent = normalize(&value);

    int digit = value * 10.0;
    *buffer++ = digit + '0';
    value = value * 10.0 - digit;
    --exponent;

    *buffer++ = '.';

    for (int i = 0; i < width; i++) {
        int digit = value * 10.0;
        *buffer++ = digit + '0';
        value = value * 10.0 - digit;
    }

    *buffer++ = 'e';
    itoa(exponent, buffer, 10);
}

static void usart_setup(USART_data_t* usartData,
						USART_t * usart,
						s16 bsel,
						s8 bscale,
						dirctrl_t dirctrl,
						USART_DREINTLVL_t recIntLevel, // Rx
						USART_DREINTLVL_t dreIntLevel) // Tx
{
	unsigned portBASE_TYPE uxQueueLength = SERIAL_PORT_BUFFER_LEN;

	// Setup for specific usart and initialize buffer queues
	//
	USART_InterruptDriver_Initialize(usartData, uxQueueLength, dirctrl, usart, dreIntLevel);


	USART_2x_Disable(usartData->usart);

	// set format: 8 Data bits, No Parity, 1 Stop bit.
	// this also implicitly sets the communication mode to asynchronous and data order to 'LSB first'
	//
	USART_Format_Set(usartData->usart, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);

	// Set communication mode to asynchronous
	//
	//USART_SetMode(usartData->usart, USART_CMODE_ASYNCHRONOUS_gc);

	// Set data order
	//USART_Set_Data_Order_LSB_first(usartData->usart);

	// Enable RXC interrupt.
	//
	USART_RxdInterruptLevel_Set(usartData->usart, recIntLevel);

	// Disable the Data Register Empty interrupt.
	// This is enabled when a byte is actually sent.
	USART_DreInterruptLevel_Set(usartData->usart, USART_DREINTLVL_OFF_gc);

	// Disable Txd interrupt. This is not used at all.
	// The Dre Interrupt is used instead to control transmission.
	USART_TxdInterruptLevel_Set(usartData->usart, USART_TXCINTLVL_OFF_gc);

	// set baud via driver function
	//
	USART_Baudrate_Set(usartData->usart, bsel, bscale);

	// Enable RX and TX.
	//
	#if SERIAL_SIMULATION == 0
	if((dirctrl == receivetransmit) || (dirctrl == receiveonly))
	{
		USART_Rx_Enable(usartData->usart);
	}
	#endif
	if((dirctrl == receivetransmit) || (dirctrl == transmitonly))
	{
		USART_Tx_Enable(usartData->usart);
	}
}