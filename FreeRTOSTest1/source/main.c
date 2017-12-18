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
	resetReason_t resetReason = getResetReason(); // knowing the reason for a reset may be of value if you want to log/track sporadically  occurring if you want to log/track sporadically  occurring
	
	size_t bytesLeft0 = xPortGetFreeHeapSize();
	if(bytesLeft0 != configTOTAL_HEAP_SIZE)
	{
		error(ERR_LOW_HEAP_SPACE);
	}
	// There must be a minimum stack size to get the initializations done until the RTOS is running.
	// bss_end depends on configTOTAL_HEAP_SIZE. Decreasing configTOTAL_HEAP_SIZE gives you room for the global stack.
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
		error(ERR_HEAP_TOO_LARGE); // decrease configTOTAL_HEAP_SIZE!
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
	
	xTaskCreate( vLedBlink, (const char *) "ledBlink", configMINIMAL_STACK_SIZE, NULL, 1,&hLedBlink);

	// Check if there was more stack used than guessed (see above: MINIMUM_STACK_NEEDED). minimumStackLeft is the number of bytes between bss-end and global (non-rtos) stack.
	// It is the minimum free stack that we had until now. It must be >0 to ensure that the allocated data in .bss is not touched during initialization of the tasks.
	// If this error occurs the rtos heap size must be reduced. (The rtos stack is located in .bss , but before the rtos scheduler is starting, the global stack at the top of the ram is used.)
	u16 minimumStackLeft_after = get_mem_unused();
	if( minimumStackLeft_after == 0)
	{
		error(ERR_LOW_GLOBAL_STACK_SPACE);
	}		
	// Check the # of free bytes on the rtos-heap that may still be allocated by pvPortMalloc(). (the rtos heap size is specified in FreeRTOSConfig.h)
	size_t bytesLeft3 = xPortGetFreeHeapSize();
	// There must be a minimum of free rtos-heap space left to successfully create the idle task in vTaskStartScheduler().
	#define MIN_STACKSIZE_TO_CREATE_IDLE_TASK 37 + configMINIMAL_STACK_SIZE
	if(bytesLeft3 < ( MIN_STACKSIZE_TO_CREATE_IDLE_TASK ) )
	{	
		// It will not be possible to create the idle task. If we don't stop execution here, it will stop immediately after because of other runtime stack checks.
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


