/*
 * FreeRTOSTest1.c
 *
 * Created: 14.11.2017 17:52:04
 * Author : Martin Burger
 */ 

#include "all.h"
#include "mem_check.h"
//extern u8 __data_start;
//extern u8 __data_end;
extern u8 __bss_start;
extern u8 __bss_end;
extern u8 __heap_start;
	
xTaskHandle hLedBlink;

// task handles
xTaskHandle hUSARTC0_Send;
xTaskHandle hUSARTC0_Receive;

//xTaskHandle hUSARTC1_Send;
xTaskHandle hUSARTC1_Receive;

xTaskHandle hUSARTD0_Send;
//xTaskHandle hUSARTD0_Receive;

xTaskHandle hUSARTD1_Receive;

xTaskHandle hUSARTE0_Receive;

xTaskHandle hSwitchPrios;



extern void vApplicationIdleHook( void );

void vSwitchPrios(void *pvParameters);
void vLedBlink(void *pvParameters);

u32 idleCnt;

//----------------------------------------------
//
void vApplicationIdleHook( void )
{
	
	idleCnt++;

	#if RUNTIME_CHECKS == 1
	checkAllStacks();
	#endif

	SLEEPMGR_PREPARE_SLEEP(SLEEP_SMODE_IDLE_gc);
	cpu_sleep();
	SLEEPMGR_DISABLE_SLEEP();
}



//----------------------------------------------
//
int main(void)
{
	unsigned portBASE_TYPE txPriority,rxPriority;

	// knowing the reason for a reset may be of value
	// if you want to log/track sporadically  occurring
	// problems

	// software reset ?
	//
	if( RST.STATUS & RST_SRF_bm )
	{
		// reset this bit
		RST.STATUS = RST_SRF_bm;
	}
	// power on reset ?
	else if( RST.STATUS & RST_PORF_bm)
	{
		// reset this bit
		RST.STATUS = RST_PORF_bm;
	}
	// debugger reset ?
	else if( RST.STATUS & RST_PDIRF_bm)
	{
		// reset this bit
		RST.STATUS = RST_PDIRF_bm;
	}
	// external reset ?
	else if( RST.STATUS & RST_EXTRF_bm)
	{
		// reset this bit
		RST.STATUS = RST_EXTRF_bm;
	}

	size_t bytesLeft0 = xPortGetFreeHeapSize();
	// consistency check
	if(bytesLeft0 != configTOTAL_HEAP_SIZE)
	{
		error(ERR_LOW_HEAP_SPACE);
	}
	
	/*
	// try to understand the memory layout!
	// these values can help you
	//
	u8* be = (u8*)&__bss_start;  // the rtos heap is located within .bss !
	u8* hs = (u8*)&__bss_end;
	u8* bs = (u8*)&__heap_start; // this heap is not used by the FreeRTOS ! (since malloc() is not used)
	*/

	// There must be a minimum stack size to get the initializations done 
	// until the RTOS is running.
	// bss_end depends on configTOTAL_HEAP_SIZE. Decreasing configTOTAL_HEAP_SIZE gives
	// you room for the global stack.
	//
	s16 freeSpaceOnGlobalStack = SP - (uint16_t) &__bss_end;

	// this value is a guess. It is double checked later on.
	#define MINIMUM_STACK_NEEDED 60
	if( freeSpaceOnGlobalStack < MINIMUM_STACK_NEEDED )	
	{
		error(ERR_HEAP_TOO_LARGE); // decrease configTOTAL_HEAP_SIZE !
	}

	// how much stack do we have used up to now?
	u16 minimumStackLeft_before = get_mem_unused();
	// this should be nearly identical to freeSpaceOnGlobalStack
	// (so this is another consistency check)
	if( freeSpaceOnGlobalStack - minimumStackLeft_before > 10 )
	{
		error(ERR_HEAP_TOO_LARGE); // decrease configTOTAL_HEAP_SIZE !
	}
	//Init clock
	vInitClock();

	// all usart preparation including port pin setup
	initUsarts();

	// prepare the queues that hold the data that is ready to be sent out
	initChannelQueues();

	//Init port settings (other than serial)
	vPortPreparation();




	// Unused hardware modules are disabled to save power.
	disableUnusedModules();

	



	// Create the tasks
	txPriority = tskIDLE_PRIORITY+1;
	rxPriority = tskIDLE_PRIORITY+1;

	xTaskCreate( vLedBlink, (const char *) "ledBlink", configMINIMAL_STACK_SIZE, NULL, 1,&hLedBlink);

	//xTaskCreate( vUSARTC0_Receive, (const char *) "uc0r", configMINIMAL_STACK_SIZE, NULL, 2,&hUSARTC0_Receive);
	//xTaskCreate( vUSARTE0_Receive,( signed char * ) "ue0r", configMINIMAL_STACK_SIZE, NULL, 3, &hUSARTE0_Receive );
	//xTaskCreate( vUSARTC1_Receive,( signed char * ) "uc1r", configMINIMAL_STACK_SIZE, NULL, 4, &hUSARTC1_Receive );
	//xTaskCreate( vUSARTC0_Send,   ( const char * ) "uc0s", configMINIMAL_STACK_SIZE, NULL, 3, &hUSARTC0_Send );	
	//TODO check return codes

	// for serial data simulation
	// two timers are used to simulate serial input
#if SERIAL_SIMULATION_C1 == 1
	setupTimerCC1Interrupt();
#endif
#if SERIAL_SIMULATION_E0 == 1
	setupTimerCE0Interrupt();
#endif

#if NESTING_TEST == 1
	setupTimerCD0Interrupt(); // to test the medium level nesting ints
	setupTimerCD1Interrupt(); // to test the high level nesting ints
#endif

	// Check if there was more stack used than guessed (see above: MINIMUM_STACK_NEEDED).
	// minimumStackLeft is the number of bytes between bss-end and global (non-rtos) stack.
	// It is the minimum free stack that we had until now.
	// It must be >0 to ensure that the allocated data in .bss is not
	// touched during initialization of the tasks. 
	// If this error occurs the rtos heap size must be reduced. (The rtos stack
	// is located in .bss , but before the rtos scheduler is starting, the global 
	// stack at the top of the ram is used.)
	u16 minimumStackLeft_after = get_mem_unused();
	if( minimumStackLeft_after == 0)
	{
		error(ERR_LOW_GLOBAL_STACK_SPACE);
	}
		
	// Check the # of free bytes on the rtos-heap that may still be allocated by pvPortMalloc().
	// (the rtos heap size is specified in FreeRTOSConfig.h)
	size_t bytesLeft3 = xPortGetFreeHeapSize();

	// There must be a minimum of free rtos-heap space left
	// to successfully create the idle task in vTaskStartScheduler().
	#define MIN_STACKSIZE_TO_CREATE_IDLE_TASK 37 + configMINIMAL_STACK_SIZE
	if(bytesLeft3 < ( MIN_STACKSIZE_TO_CREATE_IDLE_TASK ) )
	{
		// It will not be possible to create the idle task.
		// If we don't stop execution here, it will stop immediately after 
		// because of other runtime stack checks.
		//		
		error(ERR_LOW_HEAP_SPACE);
	}

	vTaskStartScheduler();

	return 0;
}


void vLedBlink(void *pvParameters) {
	(void) pvParameters;

	PORTF.DIRSET = PIN0_bm; //LED1
	PORTF.DIRSET = PIN1_bm; //LED2
	PORTF.DIRSET = PIN2_bm; //LED3
	PORTF.DIRSET = PIN3_bm; //LED4
	
	PORTE.DIRSET = PIN0_bm; //LED5
	PORTE.DIRSET = PIN1_bm; //LED6
	PORTE.DIRSET = PIN2_bm; //LED7
	PORTE.DIRSET = PIN3_bm; //LED8

	for(;;) {
		PORTE.OUT = 0x0F;
		PORTF.OUT = 0x0F;
		vTaskDelay(500 / portTICK_RATE_MS);
		PORTE.OUT = 0x00;
		PORTF.OUT = 0x00;
		vTaskDelay(500 / portTICK_RATE_MS);
		//sendData();
	}
}

//---- end of file main.c -------------------------------------------------------


