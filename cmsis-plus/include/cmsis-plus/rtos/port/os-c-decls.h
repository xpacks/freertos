/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2016 Liviu Ionescu.
 * Copyright (c) 2013 ARM LIMITED
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * This file is part of the CMSIS++ proposal, intended as a CMSIS
 * replacement for C++ applications.
 *
 * It is included in `cmsis-plus/rtos/os-c-decls.h` to customise
 * it with port specific declarations.
 *
 * These structures (which basically contain handlers)
 * are conditionally included in the system objects
 * when they are implemented using the port native objects.
 */

#ifndef CMSIS_PLUS_RTOS_PORT_OS_C_DECLS_H_
#define CMSIS_PLUS_RTOS_PORT_OS_C_DECLS_H_

#include <cmsis-plus/os-app-config.h>
#include <stdint.h>
#include <stdbool.h>

#include <FreeRTOS.h>

typedef uint64_t os_port_clock_timestamp_t;
typedef uint32_t os_port_clock_duration_t;
typedef uint64_t os_port_clock_offset_t;

typedef bool os_port_scheduler_state_t;

typedef uint32_t os_port_irq_state_t;

typedef uint32_t os_port_thread_stack_element_t;
typedef uint64_t os_port_thread_stack_allocation_element_t;

typedef struct
{
  os_port_thread_stack_element_t* stack_ptr;
} os_port_thread_context_t;

#define OS_INTEGER_RTOS_STACK_FILL_MAGIC (0xEFBEADDE)

#define OS_HAS_INTERRUPTS_STACK

// ----------------------------------------------------------------------------

#if defined(OS_USE_RTOS_PORT_SCHEDULER)

typedef struct os_thread_port_data_s
{
  void* handle;
  StaticTask_t task;
} os_thread_port_data_t;

#endif /* OS_USE_RTOS_PORT_SCHEDULER */

#if defined(OS_USE_RTOS_PORT_TIMER)

typedef struct os_timer_port_data_s
{
  void* handle;
  StaticTimer_t timer;
} os_timer_port_data_t;

#endif /* OS_USE_RTOS_PORT_TIMER */

#if defined(OS_USE_RTOS_PORT_MUTEX)

typedef struct os_mutex_port_data_s
{
  void* handle;
  StaticSemaphore_t mutex;
} os_mutex_port_data_t;

#endif /* OS_USE_RTOS_PORT_MUTEX */

#if defined(OS_USE_RTOS_PORT_CONDITION_VARIABLE)

typedef struct os_condvar_port_data_s
  {
    void* handle;
  }os_condvar_port_data_t;

#endif /* OS_USE_RTOS_PORT_CONDITION_VARIABLE */

#if defined(OS_USE_RTOS_PORT_SEMAPHORE)

typedef struct os_semaphore_port_data_s
{
  void* handle;
  StaticSemaphore_t semaphore;
} os_semaphore_port_data_t;

#endif

#if defined(OS_USE_RTOS_PORT_MEMORY_POOL)

typedef struct os_mempool_port_data_s
  {
    void* handle;
  }os_mempool_port_data_t;

#endif /* OS_USE_RTOS_PORT_MEMORY_POOL */

#if defined(OS_USE_RTOS_PORT_MESSAGE_QUEUE)

typedef struct os_mqueue_port_data_s
{
  void* handle;
  StaticQueue_t queue;
} os_mqueue_port_data_t;

#endif /* OS_USE_RTOS_PORT_MESSAGE_QUEUE */

#if defined(OS_USE_RTOS_PORT_EVENT_FLAGS)

typedef struct os_evflags_port_data_s
{
  void* handle;
  StaticEventGroup_t flags;
} os_evflags_port_data_t;

#endif /* OS_USE_RTOS_PORT_EVENT_FLAGS */

#endif /* CMSIS_PLUS_RTOS_PORT_OS_C_DECLS_H_ */
