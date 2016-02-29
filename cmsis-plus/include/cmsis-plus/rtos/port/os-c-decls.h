/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2016 Liviu Ionescu.
 * Copyright (c) 2013 ARM LIMITED
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

#if defined(OS_INCLUDE_PORT_RTOS_THREAD)

typedef struct os_thread_port_data_s
  {
    void* handle;
  }os_thread_port_data_t;

#endif /* OS_INCLUDE_PORT_RTOS_THREAD */

#if defined(OS_INCLUDE_PORT_RTOS_TIMER)

typedef struct os_timer_port_data_s
  {
    void* handle;
  }os_timer_port_data_t;

#endif /* OS_INCLUDE_PORT_RTOS_TIMER */

#if defined(OS_INCLUDE_PORT_RTOS_MUTEX)

typedef struct os_mutex_port_data_s
  {
    void* handle;
  }os_mutex_port_data_t;

#endif /* OS_INCLUDE_PORT_RTOS_MUTEX */

#if defined(OS_INCLUDE_PORT_RTOS_CONDITION_VARIABLE)

typedef struct os_condvar_port_data_s
  {
    void* handle;
  }os_condvar_port_data_t;

#endif /* OS_INCLUDE_PORT_RTOS_CONDITION_VARIABLE */

#if defined(OS_INCLUDE_PORT_RTOS_SEMAPHORE)

typedef struct os_semaphore_port_data_s
  {
    void* handle;
  }os_semaphore_port_data_t;

#endif

#if defined(OS_INCLUDE_PORT_RTOS_MEMORY_POOL)

typedef struct os_mempool_port_data_s
  {
    void* handle;
  }os_mempool_port_data_t;

#endif /* OS_INCLUDE_PORT_RTOS_MEMORY_POOL */

#if defined(OS_INCLUDE_PORT_RTOS_MESSAGE_QUEUE)

typedef struct os_mqueue_port_data_s
  {
    void* handle;
  }os_mqueue_port_data_t;

#endif /* OS_INCLUDE_PORT_RTOS_MESSAGE_QUEUE */

#if defined(OS_INCLUDE_PORT_RTOS_EVENT_FLAGS)

typedef struct os_evflags_port_data_s
  {
    void* handle;
  }os_evflags_port_data_t;

#endif /* OS_INCLUDE_PORT_RTOS_EVENT_FLAGS */

#endif /* CMSIS_PLUS_RTOS_PORT_OS_C_DECLS_H_ */
