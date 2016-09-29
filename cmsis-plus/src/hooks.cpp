/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2016 Liviu Ionescu.
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

#include <cmsis-plus/diag/trace.h>

#include <stdlib.h>

#include <FreeRTOS.h>

extern void
os_rtos_idle_actions (void);

extern "C"
{

  void
  vApplicationStackOverflowHook (void* task, const char* name);

  void
  vApplicationIdleHook (void);

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )

  void
  vApplicationGetIdleTaskMemory (StaticTask_t **ppxIdleTaskTCBBuffer,
                                 StackType_t **ppxIdleTaskStackBuffer,
                                 uint32_t *pulIdleTaskStackSize);
  void
  vApplicationGetTimerTaskMemory (StaticTask_t **ppxTimerTaskTCBBuffer,
                                  StackType_t **ppxTimerTaskStackBuffer,
                                  uint32_t *pulTimerTaskStackSize);

#endif /* ( configSUPPORT_STATIC_ALLOCATION == 1 ) */
}

void
__attribute__((weak))
vApplicationStackOverflowHook (void* task, const char* name)
{
  trace_printf (">>> Stack overflow %p '%s'!\n", task, name);
  abort ();
}

#if (configUSE_IDLE_HOOK == 0)
#error configUSE_IDLE_HOOK must be 1
#endif

void
__attribute__((weak))
vApplicationIdleHook (void)
{
  os_rtos_idle_actions ();
}

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )

static StaticTask_t idle_task;
static StackType_t idle_stack[configIDLE_TASK_STACK_DEPTH];

void
__attribute__((weak))
vApplicationGetIdleTaskMemory (StaticTask_t **ppxIdleTaskTCBBuffer,
                               StackType_t **ppxIdleTaskStackBuffer,
                               uint32_t *pulIdleTaskStackSize)
{
  *ppxIdleTaskTCBBuffer = &idle_task;
  *ppxIdleTaskStackBuffer = idle_stack;
  *pulIdleTaskStackSize = sizeof(idle_stack) / sizeof(StackType_t);
}

static StaticTask_t timer_task;
static StackType_t timer_stack[configTIMER_TASK_STACK_DEPTH];

void
__attribute__((weak))
vApplicationGetTimerTaskMemory (StaticTask_t **ppxTimerTaskTCBBuffer,
                                StackType_t **ppxTimerTaskStackBuffer,
                                uint32_t *pulTimerTaskStackSize)
{
  *ppxTimerTaskTCBBuffer = &timer_task;
  *ppxTimerTaskStackBuffer = timer_stack;
  *pulTimerTaskStackSize = sizeof(timer_stack) / sizeof(StackType_t);
}

#endif /* ( configSUPPORT_STATIC_ALLOCATION == 1 ) */
