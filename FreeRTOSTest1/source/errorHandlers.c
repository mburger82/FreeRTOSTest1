//
// File: errorHandlers.c
//
// xmega demo program VG Feb,Mar,Oct 2011
//
// CPU:   atxmega32A4, atxmega128a1
//
// Version: 1.2.1
//
#include "all.h"

typedef void tskTCB;
extern volatile tskTCB * volatile pxCurrentTCB;

extern xTaskHandle hEdulog;
extern xTaskHandle hLedBlink;
unsigned portBASE_TYPE minStackSpace_Idle;
unsigned portBASE_TYPE minStackSpace_Edulog;
unsigned portBASE_TYPE minStackSpace_LedBlink;

// local prototypes
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );
void vApplicationMallocFailedHook( void );


//----------------------------------------------
// Checking all stacks uses a lot of CPU time!
// Disabling this reduces power consumption 
// from 32mA to 23 mA if the CPU is allowed to 
// sleep instead.
#if RUNTIME_CHECKS == 1

void checkAllStacks(void)
{
	const unsigned portBASE_TYPE min = 20;

	minStackSpace_Idle	= uxTaskGetStackHighWaterMark( ( xTaskHandle ) NULL );
	
	minStackSpace_Edulog = uxTaskGetStackHighWaterMark((xTaskHandle) hEdulog);

	minStackSpace_LedBlink = uxTaskGetStackHighWaterMark((xTaskHandle) hLedBlink);

	if(minStackSpace_Edulog<min ||
	   minStackSpace_LedBlink<min ||
	   minStackSpace_Idle<min)	
	{
		error(ERR_LOW_HEAP_SPACE);
	}
	
}
#endif

//----------------------------------------------
// catch heap overflow
//
void vApplicationMallocFailedHook( void )
{
	error(ERR_LOW_HEAP_SPACE);		
} 


//----------------------------------------------
// to report kernel detected stack overflows
//
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	/* Just to stop compiler warnings. */
	(void) pxTask;
	(void) pcTaskName;

	#if RUNTIME_CHECKS == 1
	checkAllStacks();
	#endif

	error(ERR_STACK_OVERFLOW);

}

//----------------------------------------------
//
// for errors that are not directly caused by 
// kernel problems 
//
void errorNonFatal(u8 errCode)
{
	(void)errCode;

	u8 a=42; (void)a;

}

//----------------------------------------------
//
void error(u8 errCode)
{
	u8 a;

	if(errCode==ERR_STACK_OVERFLOW)
		a = 1;
	else if(errCode==ERR_QUEUE_SEND_FAILED)
		a = 2;
	else if(errCode==ERR_BYTES_SHOULD_BE_AVAILABLE)
		a = 3;
    else
		a = 4;

// TODO from here:
//
// - log the occurance of any error in flash mem (inc counter)
// - print a message

	software_reset();
}

//----------------------------------------------
//
void software_reset(void)
{
	CPU_CCP  = CCP_IOREG_gc;
	RST.CTRL = RST_SWRST_bm ;
}


// EOF errorHandlers.c
