/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2016 Liviu Ionescu.
 *
 * µOS++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * µOS++ is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This file is part of the CMSIS++ proposal, intended as a CMSIS
 * replacement for C++ applications.
 */

#ifndef CMSIS_PLUS_RTOS_OS_APP_CONFIG_H_
#define CMSIS_PLUS_RTOS_OS_APP_CONFIG_H_

// ----------------------------------------------------------------------------

#define OS_INTEGER_SYSTICK_FREQUENCY_HZ                     (1000)

#define OS_INTEGER_RTOS_MAIN_STACK_SIZE_BYTES               (1024)

// With 4 bits NVIC, there are 16 levels, 0 = highest, 15 = lowest

// Disable all interrupts from 15 to 4, keep 3-2-1 enabled
#define OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY (4)

// ----------------------------------------------------------------------------

// Special applications may require to add some custom
// storage in the thread control block.
//#define OS_INCLUDE_RTOS_CUSTOM_THREAD_USER_STORAGE          (1)
//typedef struct os_thread_user_storage_s
//{
//  int   dummy;
//} os_thread_user_storage_t;

// ----------------------------------------------------------------------------

// Request the inclusion of custom implementations.
#define OS_INCLUDE_RTOS_PORT_THREAD                         (1)
#if 0
#define OS_INCLUDE_RTOS_PORT_TIMER                          (1)
#define OS_INCLUDE_RTOS_PORT_SYSTICK_CLOCK_SLEEP_FOR        (1)
#define OS_INCLUDE_RTOS_PORT_MUTEX                          (1)
// #define OS_INCLUDE_RTOS_PORT_CONDITION_VARIABLE             (1)
#define OS_INCLUDE_RTOS_PORT_SEMAPHORE                      (1)
// #define OS_INCLUDE_RTOS_PORT_MEMORY_POOL                    (1)
#define OS_INCLUDE_RTOS_PORT_MESSAGE_QUEUE                  (1)
#define OS_INCLUDE_RTOS_PORT_EVENT_FLAGS                    (1)
#endif

// ----------------------------------------------------------------------------

#if 0
#define OS_TRACE_RTOS_CLOCKS
#define OS_TRACE_RTOS_CONDVAR
#define OS_TRACE_RTOS_EVFLAGS
#define OS_TRACE_RTOS_LISTS
#define OS_TRACE_RTOS_MEMPOOL
#define OS_TRACE_RTOS_MQUEUE
#define OS_TRACE_RTOS_MUTEX
#define OS_TRACE_RTOS_SEMAPHORE
#define OS_TRACE_RTOS_THREAD
#define OS_TRACE_RTOS_TIMER
#endif

// ----------------------------------------------------------------------------

#endif /* CMSIS_PLUS_RTOS_OS_APP_CONFIG_H_ */
