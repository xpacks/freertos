/*
	FreeRTOS.org V4.8.0 - Copyright (C) 2003-2008 Richard Barry.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS.org is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS.org; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS.org, without being obliged to provide
	the source code for any proprietary components.  See the licensing section 
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

    ***************************************************************************
    ***************************************************************************
    *                                                                         *
    * SAVE TIME AND MONEY!  We can port FreeRTOS.org to your own hardware,    *
    * and even write all or part of your application on your behalf.          *
    * See http://www.OpenRTOS.com for details of the services we provide to   *
    * expedite your project.                                                  *
    *                                                                         *
    ***************************************************************************
    ***************************************************************************

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and 
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety 
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting, 
	licensing and training services.
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM3 port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* For backward compatibility, ensure configKERNEL_INTERRUPT_PRIORITY is 
defined.  The value should also ensure backward compatibility.  
FreeRTOS.org versions prior to V4.4.0 did not include this definition. */
#ifndef configKERNEL_INTERRUPT_PRIORITY
	#define configKERNEL_INTERRUPT_PRIORITY 255
#endif

/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL		( ( volatile unsigned portLONG *) 0xe000e010 )
#define portNVIC_SYSTICK_LOAD		( ( volatile unsigned portLONG *) 0xe000e014 )
#define portNVIC_INT_CTRL			( ( volatile unsigned portLONG *) 0xe000ed04 )
#define portNVIC_SYSPRI2			( ( volatile unsigned portLONG *) 0xe000ed20 )
#define portNVIC_SYSTICK_CLK		0x00000004
#define portNVIC_SYSTICK_INT		0x00000002
#define portNVIC_SYSTICK_ENABLE		0x00000001
#define portNVIC_PENDSVSET			0x10000000
#define portNVIC_PENDSV_PRI			( ( ( unsigned portLONG ) configKERNEL_INTERRUPT_PRIORITY ) << 16 )
#define portNVIC_SYSTICK_PRI		( ( ( unsigned portLONG ) configKERNEL_INTERRUPT_PRIORITY ) << 24 )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR			( 0x01000000 )

/* The priority used by the kernel is assigned to a variable to make access
from inline assembler easier. */
const unsigned portLONG ulKernelPriority = configKERNEL_INTERRUPT_PRIORITY;

/* Each task maintains its own interrupt status in the critical nesting
variable. */
unsigned portBASE_TYPE uxCriticalNesting = 0xaaaaaaaa;

/* 
 * Setup the timer to generate the tick interrupts.
 */
static void prvSetupTimerInterrupt( void );

/*
 * Exception handlers.
 */
void xPortPendSVHandler( void ) __attribute__ (( naked ));
void xPortSysTickHandler( void );
void vPortSVCHandler( void ) __attribute__ (( naked ));

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
void vPortStartFirstTask( unsigned long ulValue ) __attribute__ (( naked ));

/*-----------------------------------------------------------*/

/* 
 * See header file for description. 
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
	/* Simulate the stack frame as it would be created by a context switch
	interrupt. */
	*pxTopOfStack = portINITIAL_XPSR;	/* xPSR */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) pxCode;	/* PC */
	pxTopOfStack--;
	*pxTopOfStack = 0;	/* LR */
	pxTopOfStack -= 5;	/* R12, R3, R2 and R1. */
	*pxTopOfStack = ( portSTACK_TYPE ) pvParameters;	/* R0 */
	pxTopOfStack -= 9;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */
	*pxTopOfStack = 0x00000000; /* uxCriticalNesting. */

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

void vPortSVCHandler( void )
{
	asm volatile (
					"	ldr	r3, pxCurrentTCBConst2		\n" /* Restore the context. */
					"	ldr r1, [r3]					\n" /* Use pxCurrentTCBConst to get the pxCurrentTCB address. */
					"	ldr r0, [r1]					\n" /* The first item in pxCurrentTCB is the task top of stack. */
					"	ldmia r0!, {r1, r4-r11}			\n" /* Pop the registers that are not automatically saved on exception entry and the critical nesting count. */
					"	ldr r2, uxCriticalNestingConst2	\n" /* Restore the critical nesting count used by the task. */
					"	str r1, [r2]					\n"
					"	msr psp, r0						\n" /* Restore the task stack pointer. */
					"	orr r14, #0xd					\n"
					"	bx r14							\n"
					"									\n"
					"	.align 2						\n"
					"pxCurrentTCBConst2: .word pxCurrentTCB				\n"
					"uxCriticalNestingConst2: .word uxCriticalNesting	\n"
				);
}
/*-----------------------------------------------------------*/

void vPortStartFirstTask( unsigned long ulValue )
{
	/* ulValue is used from the asm code, but the compiler does not know
	this so remove the warning. */
	( void ) ulValue;
	
	asm volatile( 
					"	msr msp, r0								\n" /* Set the msp back to the start of the stack. */
					"	svc 0									\n" /* System call to start first task. */
				);
}
/*-----------------------------------------------------------*/

/* 
 * See header file for description. 
 */
portBASE_TYPE xPortStartScheduler( void )
{
	/* Make PendSV, CallSV and SysTick the same priroity as the kernel. */
	*(portNVIC_SYSPRI2) |= portNVIC_PENDSV_PRI;
	*(portNVIC_SYSPRI2) |= portNVIC_SYSTICK_PRI;

	/* Start the timer that generates the tick ISR.  Interrupts are disabled
	here already. */
	prvSetupTimerInterrupt();
	
	/* Start the first task. */
	vPortStartFirstTask( *((unsigned portLONG *) 0 ) );

	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* It is unlikely that the CM3 port will require this function as there
	is nothing to return to.  */
}
/*-----------------------------------------------------------*/

void vPortYieldFromISR( void )
{
	/* Set a PendSV to request a context switch. */
	*(portNVIC_INT_CTRL) |= portNVIC_PENDSVSET;

	/* This function is also called in response to a Yield(), so we want
	the yield to occur immediately. */
	portENABLE_INTERRUPTS();
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
	portDISABLE_INTERRUPTS();
	uxCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
	uxCriticalNesting--;
	if( uxCriticalNesting == 0 )
	{
		portENABLE_INTERRUPTS();
	}
}
/*-----------------------------------------------------------*/

void xPortPendSVHandler( void )
{
	/* This is a naked function. */

	__asm volatile
	( 
	"	mrs r0, psp						\n" 
	"									\n"
	"	ldr	r3, pxCurrentTCBConst		\n" /* Get the location of the current TCB. */
	"	ldr	r2, [r3]					\n"	
	"									\n"	
	"	ldr r1, uxCriticalNestingConst	\n" /* Save the remaining registers and the critical nesting count onto the task stack. */
	"	ldr r1, [r1]					\n"
	"	stmdb r0!, {r1,r4-r11}			\n"
	"	str r0, [r2]					\n" /* Save the new top of stack into the first member of the TCB. */
	"									\n"
	"	stmdb sp!, {r3, r14}			\n" 
	"	bl vTaskSwitchContext			\n"
	"	ldmia sp!, {r3, r14}			\n"
	"									\n"	/* Restore the context, including the critical nesting count. */
	"	ldr r1, [r3]					\n" 
	"	ldr r2, uxCriticalNestingConst	\n"
	"	ldr r0, [r1]					\n" /* The first item in pxCurrentTCB is the task top of stack. */
	"	ldmia r0!, {r1, r4-r11}			\n" /* Pop the registers and the critical nesting count. */
	"	str r1, [r2]					\n" /* Save the new critical nesting value into ulCriticalNesting. */ 
	"	msr psp, r0						\n" 
	"	orr r14, #0xd					\n"
	"									\n"	/* Exit with interrupts in the state required by the task. */	
	"	cbnz r1, sv_disable_interrupts	\n" /* If the nesting count is greater than 0 we need to exit with interrupts masked. */
	"	bx r14							\n"
	"									\n"
	"sv_disable_interrupts:				\n" 
	"	ldr r1, =ulKernelPriority 		\n"
	"	ldr r1, [r1]					\n"
	"	msr	basepri, r1					\n"
	"	bx r14							\n"
	"									\n"
	"	.align 2						\n"
	"pxCurrentTCBConst: .word pxCurrentTCB				\n"
	"uxCriticalNestingConst: .word uxCriticalNesting	\n"
	);
}
/*-----------------------------------------------------------*/

void xPortSysTickHandler( void )
{
	vTaskIncrementTick();
	
	/* If using preemption, also force a context switch. */
	#if configUSE_PREEMPTION == 1
		*(portNVIC_INT_CTRL) |= portNVIC_PENDSVSET;	
	#endif
}
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void prvSetupTimerInterrupt( void )
{
	/* Configure SysTick to interrupt at the requested rate. */
	*(portNVIC_SYSTICK_LOAD) = configCPU_CLOCK_HZ / configTICK_RATE_HZ;
	*(portNVIC_SYSTICK_CTRL) = portNVIC_SYSTICK_CLK | portNVIC_SYSTICK_INT | portNVIC_SYSTICK_ENABLE;
}
/*-----------------------------------------------------------*/

