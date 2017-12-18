/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <avr/io.h>

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION		1
#define configUSE_IDLE_HOOK			1
#define configUSE_TICK_HOOK			0


#define configCPU_CLOCK_HZ			( ( unsigned portLONG ) 32000000 )
#ifndef F_CPU
# warning ("F_CPU undefined !")
#else
#if (F_CPU != 32000000)
#  warning ("F_CPU and configCPU_CLOCK_HZ are not consistent !")
#endif
#endif

#define configTICK_RATE_HZ			( ( portTickType ) 1000 )

// atxmega  32A4 has a 2 byte program counter and 2 byte stack pointer, push/pop increments/decrements by 2
// atxmega 128A1 has a 3 byte program counter and 2 byte stack pointer, push/pop increments/decrements by 3
// atxmega 256a3 has a 3 byte program counter and 2 byte stack pointer, push/pop increments/decrements by 3
//
#define config24BITADDRESSING		1 // 0 for 32A4 ; 1 for 128A1,256A3

// controls whether the registers RAMPD,RAMPX,RAMPZ are saved in the context of a task.
// The gcc compiler does it, but I think that most applications don't need it. So leaving
// this to 0 makes the context switch faster and uses less stack space. If you suspect you need this,
// or just want to be on the safe side, set this value to 1. Rethink your decision if you are using 
// external RAM or a CPU with large program flash (>128k).
//
#define configEXTENDED_ADRESSING	0

// This enables round robin scheduling of low-level interrupts.
// When this option is 0, low-level interrupts are priority controlled.
// This option is not available for med- and high-level interrupts (the xmega hardware does not support this).
//
#define configENABLE_ROUND_ROBIN	1

// This defines the interrupt level on which the kernel is running (tick-interrupt).
// This shall be the lowest possible value: 0.
//
#define configKERNEL_INTERRUPT_PRIORITY			0	// kernel interrupt level is low-level, don't change!

// This value defines which ISRs are allowed to call the light-weight
// api functions (the fromISR... ones).
// ISRs running in a level < configKERNEL_INTERRUPT_PRIORITY are never allowed to call the 
// light-weight api functions.  
#define configMAX_SYSCALL_INTERRUPT_PRIORITY	2	// 0=low-level ,1=medium-level, 2=high-level

#if (configMAX_SYSCALL_INTERRUPT_PRIORITY < configKERNEL_INTERRUPT_PRIORITY)
	#error configMAX_SYSCALL_INTERRUPT_PRIORITY must be >= configKERNEL_INTERRUPT_PRIORITY !
#endif

#define configMAX_PRIORITIES			( ( unsigned portBASE_TYPE ) 4 )
#define configMINIMAL_STACK_SIZE		( ( unsigned short ) 200 )
#define configTOTAL_HEAP_SIZE			( (size_t ) ( 3000 ) )
#define configUSE_MALLOC_FAILED_HOOK	1
#define configMAX_TASK_NAME_LEN			( 8 )
#define configUSE_TRACE_FACILITY		1
#define configUSE_16_BIT_TICKS			0    // 0 means 32bit ticks
#define configIDLE_SHOULD_YIELD			1
#define configQUEUE_REGISTRY_SIZE		0
#define	configCHECK_FOR_STACK_OVERFLOW 	2    // use both method 1 and method 2

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		0
#define configMAX_CO_ROUTINE_PRIORITIES ( 4 )
#define	configNUMBER_OF_COROUTINES	6

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet			1
#define INCLUDE_uxTaskPriorityGet			0
#define INCLUDE_vTaskDelete					0
#define INCLUDE_vTaskCleanUpResources		0
#define INCLUDE_vTaskSuspend				1 // needed to block on queues
#define INCLUDE_vTaskDelayUntil				1
#define INCLUDE_vTaskDelay					1

#define INCLUDE_uxTaskGetStackHighWaterMark	1 // used to check if stack is going low
#define	INCLUDE_xTaskGetCurrentTaskHandle	1


#endif /* FREERTOS_CONFIG_H */
