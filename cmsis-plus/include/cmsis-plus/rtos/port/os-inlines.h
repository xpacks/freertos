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
 *
 * If contains the implementation for all objects, using the
 * FreeRTOS API.
 *
 * This file is included in all src/os-*.cpp files.
 */

#ifndef CMSIS_PLUS_RTOS_PORT_OS_INLINES_H_
#define CMSIS_PLUS_RTOS_PORT_OS_INLINES_H_

// ----------------------------------------------------------------------------

#include <cmsis-plus/rtos/os-app-config.h>
#include <cmsis-plus/rtos/os-c-decls.h>

// ----------------------------------------------------------------------------

#ifdef  __cplusplus

#include <FreeRTOS.h>
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

#include <cmsis_device.h>

namespace os
{
  namespace rtos
  {
    namespace port
    {
      // ----------------------------------------------------------------------

      namespace scheduler
      {
        inline result_t
        __attribute__((always_inline))
        initialize (void)
        {
          return result::ok;
        }

        inline bool
        __attribute__((always_inline))
        in_handler_mode (void)
        {
          // In Handler mode, IPSR holds the exception number.
          // If 0, the core is in thread mode.
          return (__get_IPSR () != 0);
        }

        inline result_t
        __attribute__((always_inline))
        start (void)
        {
          vTaskStartScheduler ();
          return result::ok;
        }

        inline void
        __attribute__((always_inline))
        lock (void)
        {
          vTaskSuspendAll ();
        }

        inline void
        __attribute__((always_inline))
        unlock (rtos::scheduler::status_t status __attribute__((unused)))
        {
          xTaskResumeAll ();
        }

      } /* namespace scheduler */

      namespace critical
      {
        // Enter an IRQ critical section
        inline rtos::critical::status_t
        __attribute__((always_inline))
        enter (void)
        {
#if 0
          // TODO: on M0 & M0+ cores there is no BASEPRI
          uint32_t pri = __get_BASEPRI();
          __set_BASEPRI_MAX(OS_INTEGER_CRITICAL_SECTION_INTERRUPT_PRIORITY << ((8 - __NVIC_PRIO_BITS)));
          return pri;
#else
          taskENTER_CRITICAL();
          return 0;
#endif
        }

        // Exit an IRQ critical section
        inline rtos::critical::status_t
        __attribute__((always_inline))
        exit (rtos::critical::status_t status __attribute__((unused)))
        {
#if 0
          uint32_t pri = __get_BASEPRI();
          __set_BASEPRI(status);
          return pri;
#else
          taskEXIT_CRITICAL();
          return 0;
#endif
        }
      }

      class Systick_clock
      {
      public:

        inline static result_t
        __attribute__((always_inline))
        sleep_for (duration_t ticks)
        {
          vTaskDelay (ticks);
          return result::ok;
        }
      };

      // ======================================================================

#if defined(OS_INCLUDE_PORT_RTOS_THREAD)

      namespace this_thread
      {
        inline rtos::Thread&
        __attribute__((always_inline))
        thread (void)
        {
          TaskHandle_t th = xTaskGetCurrentTaskHandle ();

          void* p = pvTaskGetThreadLocalStoragePointer (th, 0);
          assert(p != nullptr);

          return *((rtos::Thread*) p);
        }

        inline void
        __attribute__((always_inline))
        yield (void)
        {
          taskYIELD();
        }

      } /* namespace this_thread */

      inline unsigned portBASE_TYPE
      makeFreeRtosPriority (thread::priority_t priority)
      {
        unsigned portBASE_TYPE fr_prio = tskIDLE_PRIORITY;

        fr_prio += (priority - thread::priority::idle);

        return fr_prio;
      }

      inline thread::priority_t
      makeCmsisPriority (unsigned portBASE_TYPE priority)
      {
        thread::priority_t cm_prio = thread::priority::idle;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
        cm_prio += (thread::priority_t) (priority - tskIDLE_PRIORITY);
#pragma GCC diagnostic pop

        return cm_prio;
      }

      class Thread
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::Thread* obj)
        {
          uint16_t stack_size_words = (uint16_t) (obj->stack_size_bytes_
              / (sizeof(StackType_t)));
          if (stack_size_words < configMINIMAL_STACK_SIZE)
            {
              stack_size_words = configMINIMAL_STACK_SIZE;
            }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
          BaseType_t res;
#pragma GCC diagnostic pop

          TaskHandle_t th;
          res = xTaskCreate((TaskFunction_t ) rtos::Thread::_invoke_with_exit,
                            (const portCHAR *) obj->name (), stack_size_words,
                            obj, makeFreeRtosPriority (obj->prio_), &th);
          assert(res == pdPASS);

          obj->func_result_ = nullptr;

          // Remember pointer to implementation.
          obj->port_.handle = th;

          // Store the pointer to this Thread as index 0 in the FreeRTOS
          // local storage pointers.
          vTaskSetThreadLocalStoragePointer (th, 0, obj);

          EventGroupHandle_t eh = xEventGroupCreate ();
          xEventGroupClearBits (eh, 1);

          obj->port_.event_flags = eh;
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::Thread* obj)
        {
          vEventGroupDelete (obj->port_.event_flags);
        }

        inline static void
        __attribute__((always_inline))
        suspend (rtos::Thread* obj)
        {
          vTaskSuspend (obj->port_.handle);
        }

        inline static void
        __attribute__((always_inline))
        wakeup (rtos::Thread* obj)
        {
          vTaskResume (obj->port_.handle);
        }

        inline static thread::priority_t
        __attribute__((always_inline))
        sched_prio (rtos::Thread* obj)
        {
          UBaseType_t p = uxTaskPriorityGet (obj->port_.handle);
          thread::priority_t prio = makeCmsisPriority (p);
          assert(prio == obj->prio_);

          return prio;
        }

        inline static result_t
        __attribute__((always_inline))
        sched_prio (rtos::Thread* obj, thread::priority_t prio)
        {
          obj->prio_ = prio;

          vTaskPrioritySet (obj->port_.handle, makeFreeRtosPriority (prio));

          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        join (rtos::Thread* obj)
        {
          for (;;)
            {
              if (obj->sched_state_ == thread::state::terminated)
                {
                  break;
                }
              //Systick_clock::sleep_for (1);
              xEventGroupWaitBits (obj->port_.event_flags, 1, pdTRUE, pdFALSE,
              portMAX_DELAY);
            }
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        detach (rtos::Thread* obj __attribute__((unused)))
        {
          return result::ok;
        }

        inline static void
        __attribute__((always_inline))
        exit (rtos::Thread* obj)
        {
          xEventGroupSetBits (obj->port_.event_flags, 1);

          obj->port_.handle = nullptr;
          vTaskDelete (nullptr);
        }

      };

#endif /* OS_INCLUDE_PORT_RTOS_THREAD */

      // ======================================================================

#if defined(OS_INCLUDE_PORT_RTOS_TIMER)

      class Timer
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::Timer* obj, timer::func_t function,
                timer::func_args_t args)
        {
          // Start with 1 tick period and change it at start(), when the
          // actual period is known.
          // Note: args are passed as ID, this requires a small patch in
          // timers.c to pass the pointer back.
          obj->port_.handle = xTimerCreate (
              obj->name (), 1,
              obj->type_ == timer::run::once ? pdFALSE : pdTRUE, (void*) args,
              (TaskFunction_t) function);

          os_assert_throw (obj->port_.handle != NULL, ENOMEM);
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::Timer* obj)
        {
          BaseType_t ret = xTimerDelete(obj->port_.handle, portMAX_DELAY);

          os_assert_throw (ret == pdTRUE, ENOTRECOVERABLE);
        }

        inline static result_t
        __attribute__((always_inline))
        start (rtos::Timer* obj, systicks_t ticks)
        {
          if (xTimerIsTimerActive (obj->port_.handle) != pdFALSE)
            {
              if (xTimerReset(obj->port_.handle, 0) != pdPASS)
                {
                  return ENOTRECOVERABLE;
                }
            }
          else
            {
              if (xTimerChangePeriod(obj->port_.handle, ticks, 0) != pdPASS)
                {
                  return ENOTRECOVERABLE;
                }
              else
                {
                  if (xTimerStart(obj->port_.handle, 0) != pdPASS)
                    {
                      return ENOTRECOVERABLE;
                    }
                }
            }
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        stop (rtos::Timer* obj)
        {
          if (xTimerIsTimerActive (obj->port_.handle) != pdFALSE)
            {
              return EAGAIN;
            }
          else
            {
              if (xTimerStop(obj->port_.handle, 0) != pdPASS)
                {
                  return ENOTRECOVERABLE;
                }
            }
          return result::ok;
        }
      };

#endif /* OS_INCLUDE_PORT_RTOS_TIMER */

      // ======================================================================

#if defined(OS_INCLUDE_PORT_RTOS_MUTEX)

      class Mutex
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::Mutex* obj)
        {
          if (obj->type_ == mutex::type::recursive)
            {
              obj->port_.handle = xSemaphoreCreateRecursiveMutex ();
            }
          else
            {
              obj->port_.handle = xSemaphoreCreateMutex ();
            }
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::Mutex* obj)
        {
          vSemaphoreDelete(obj->port_.handle);
        }

        inline static result_t
        __attribute__((always_inline))
        lock (rtos::Mutex* obj)
        {
          BaseType_t res;

          if (obj->type_ == mutex::type::recursive)
            {
              res = xSemaphoreTakeRecursive(obj->port_.handle, portMAX_DELAY);
            }
          else
            {
              res = xSemaphoreTake(obj->port_.handle, portMAX_DELAY);
            }

          if (res != pdTRUE)
            {
              return ENOTRECOVERABLE;
            }
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        try_lock (rtos::Mutex* obj)
        {
          BaseType_t res;

          if (obj->type_ == mutex::type::recursive)
            {
              res = xSemaphoreTakeRecursive(obj->port_.handle, 0);
            }
          else
            {
              res = xSemaphoreTake(obj->port_.handle, 0);
            }

          if (res != pdTRUE)
            {
              return EAGAIN;
            }
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        timed_lock (rtos::Mutex* obj, systicks_t ticks)
        {
          BaseType_t res;

          if (obj->type_ == mutex::type::recursive)
            {
              res = xSemaphoreTakeRecursive(obj->port_.handle, ticks);
            }
          else
            {
              res = xSemaphoreTake(obj->port_.handle, ticks);
            }

          if (res != pdTRUE)
            {
              return ETIMEDOUT;
            }
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        unlock (rtos::Mutex* obj)
        {
          BaseType_t res;

          if (obj->type_ == mutex::type::recursive)
            {
              res = xSemaphoreGiveRecursive(obj->port_.handle);
            }
          else
            {
              res = xSemaphoreGive(obj->port_.handle);
            }

          if (res != pdTRUE)
            {
              return ENOTRECOVERABLE;
            }
          return result::ok;
        }

        inline static thread::priority_t
        __attribute__((always_inline))
        prio_ceiling (const rtos::Mutex* obj)
        {
          return obj->prio_ceiling_;
        }

        inline static result_t
        __attribute__((always_inline))
        prio_ceiling (rtos::Mutex* obj, thread::priority_t prio_ceiling,
                      thread::priority_t* old_prio_ceiling)
        {
          thread::priority_t prio;
            {
              // TODO: lock() must not adhere to the priority protocol.
              obj->lock ();
              prio = obj->prio_ceiling_;
              obj->prio_ceiling_ = prio_ceiling;
              obj->unlock ();
            }

          if (old_prio_ceiling != nullptr)
            {
              *old_prio_ceiling = prio;
            }
          return result::ok;
        }

        inline static result_t
        consistent (rtos::Mutex* obj __attribute__((unused)))
        {
          return result::ok;
        }

      };
#endif /* OS_INCLUDE_PORT_RTOS_MUTEX */

      // ======================================================================

#if defined(OS_INCLUDE_PORT_RTOS_CONDITION_VARIABLE)

      class Condition_variable
        {
        public:

        };

#endif /* OS_INCLUDE_PORT_RTOS_CONDITION_VARIABLE */

      // ======================================================================

#if defined(OS_INCLUDE_PORT_RTOS_SEMAPHORE)

      class Semaphore
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::Semaphore* obj)
        {
          obj->port_.handle = xSemaphoreCreateCounting(obj->max_count_,
                                                       obj->initial_count_);
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::Semaphore* obj)
        {
          vSemaphoreDelete(obj->port_.handle);
        }

        inline static result_t
        __attribute__((always_inline))
        post (rtos::Semaphore* obj)
        {
          portBASE_TYPE thread_woken = pdFALSE;

          if (rtos::scheduler::in_handler_mode ())
            {
              if (xSemaphoreGiveFromISR(obj->port_.handle,
                  &thread_woken) != pdTRUE)
                {
                  return EOVERFLOW;
                }
              portEND_SWITCHING_ISR(thread_woken);
            }
          else
            {
              if (xSemaphoreGive (obj->port_.handle) != pdTRUE)
                {
                  return EOVERFLOW;
                }
            }
          ++(obj->count_);

          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        wait (rtos::Semaphore* obj)
        {
          if (xSemaphoreTake (obj->port_.handle, portMAX_DELAY) != pdTRUE)
            {
              return ENOTRECOVERABLE;
            }
          --(obj->count_);

          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        try_wait (rtos::Semaphore* obj)
        {
          portBASE_TYPE thread_woken = pdFALSE;

          if (rtos::scheduler::in_handler_mode ())
            {
              if (xSemaphoreTakeFromISR(obj->port_.handle,
                  &thread_woken) != pdTRUE)
                {
                  return EAGAIN;
                }
              portEND_SWITCHING_ISR(thread_woken);
            }
          else if (xSemaphoreTake (obj->port_.handle, 0) != pdTRUE)
            {
              return EAGAIN;
            }

          --(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        timed_wait (rtos::Semaphore* obj, systicks_t ticks)
        {
          if (xSemaphoreTake(obj->port_.handle,
              ticks > 0 ? ticks : 1) != pdTRUE)
            {
              return ETIMEDOUT;
            }

          --(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        reset (rtos::Semaphore* obj)
        {
          if (obj->count_ < 0)
            {
              // There are waiting tasks
              return EAGAIN;
            }

          // TODO: Check if this properly resets a semaphore.
          xQueueReset(obj->port_.handle);

          obj->count_ = obj->initial_count_;
          return result::ok;
        }

      };

#endif /* OS_INCLUDE_PORT_RTOS_SEMAPHORE */

      // ======================================================================

#if defined(OS_INCLUDE_PORT_RTOS_MEMORY_POOL)

      class Memory_pool
        {
        public:

        };

#endif /* OS_INCLUDE_PORT_RTOS_MEMORY_POOL */

      // ======================================================================

#if defined(OS_INCLUDE_PORT_RTOS_MESSAGE_QUEUE)

      class Message_queue
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::Message_queue* obj)
        {
          obj->port_.handle = xQueueCreate(obj->msgs_, obj->queue_size_bytes_);
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::Message_queue* obj)
        {
          vQueueDelete (obj->port_.handle);
        }

        inline static result_t
        __attribute__((always_inline))
        send (rtos::Message_queue* obj, const char* msg,
              std::size_t nbytes __attribute__((unused)),
              mqueue::priority_t mprio __attribute__((unused)))
        {
          // FreeRTOS will store the full message, regardless of the nbytes.
          BaseType_t res = xQueueSend(obj->port_.handle, msg, portMAX_DELAY);

          if (res != pdTRUE)
            {
              return ENOTRECOVERABLE;
            }

          ++(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        try_send (rtos::Message_queue* obj, const char* msg,
                  std::size_t nbytes __attribute__((unused)),
                  mqueue::priority_t mprio __attribute__((unused)))
        {
          portBASE_TYPE thread_woken = pdFALSE;

          if (rtos::scheduler::in_handler_mode ())
            {
              if (xQueueSendFromISR(obj->port_.handle, msg,
                  &thread_woken) != pdTRUE)
                {
                  return EAGAIN;
                }
              portEND_SWITCHING_ISR(thread_woken);
            }
          else
            {
              if (xQueueSend (obj->port_.handle, msg, 0) != pdTRUE)
                {
                  return EAGAIN;
                }
            }

          ++(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        timed_send (rtos::Message_queue* obj, const char* msg,
                    std::size_t nbytes __attribute__((unused)),
                    mqueue::priority_t mprio __attribute__((unused)),
                    systicks_t ticks)
        {
          // FreeRTOS will store the full message, regardless of the nbytes.
          BaseType_t res = xQueueSend(obj->port_.handle, msg, ticks);

          if (res != pdTRUE)
            {
              return ETIMEDOUT;
            }

          ++(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        receive (rtos::Message_queue* obj, char* msg,
                 std::size_t nbytes __attribute__((unused)),
                 mqueue::priority_t* mprio __attribute__((unused)))
        {
          BaseType_t res = xQueueReceive(obj->port_.handle, msg, portMAX_DELAY);
          if (res != pdTRUE)
            {
              return ENOTRECOVERABLE;
            }

          --(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        try_receive (rtos::Message_queue* obj, char* msg,
                     std::size_t nbytes __attribute__((unused)),
                     mqueue::priority_t* mprio __attribute__((unused)))
        {
          portBASE_TYPE thread_woken = pdFALSE;

          if (rtos::scheduler::in_handler_mode ())
            {
              if (xQueueReceiveFromISR (obj->port_.handle, msg,
                                        &thread_woken) != pdTRUE)
                {
                  return EAGAIN;
                }
              portEND_SWITCHING_ISR(thread_woken);
            }
          else
            {
              if (xQueueReceive (obj->port_.handle, msg, 0) != pdTRUE)
                {
                  return EAGAIN;
                }
            }

          --(obj->count_);
          return result::ok;

        }

        inline static result_t
        __attribute__((always_inline))
        timed_receive (rtos::Message_queue* obj, char* msg,
                       std::size_t nbytes __attribute__((unused)),
                       mqueue::priority_t* mprio __attribute__((unused)),
                       systicks_t ticks)
        {
          BaseType_t res = xQueueReceive(obj->port_.handle, msg, ticks);
          if (res != pdTRUE)
            {
              return ETIMEDOUT;
            }

          --(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        reset (rtos::Message_queue* obj)
        {
          xQueueReset(obj->port_.handle);

          obj->count_ = 0;
          return result::ok;
        }
      };

#endif /* OS_INCLUDE_PORT_RTOS_MESSAGE_QUEUE */

      // ======================================================================

#if defined(OS_INCLUDE_PORT_RTOS_EVENT_FLAGS)

      class Event_flags
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::Event_flags* obj)
        {
          obj->port_.handle = xEventGroupCreate ();
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::Event_flags* obj)
        {
          vEventGroupDelete (obj->port_.handle);
        }

        inline static result_t
        __attribute__((always_inline))
        wait (rtos::Event_flags* obj, flags::mask_t mask, flags::mask_t* oflags,
              flags::mode_t mode)
        {
          EventBits_t bits;

          if (mask != 0)
            {
              bits = xEventGroupWaitBits (
                  obj->port_.handle, mask,
                  ((mode & flags::mode::clear) != 0) ? pdTRUE : pdFALSE,
                  ((mode & flags::mode::all) != 0) ? pdTRUE : pdFALSE,
                  portMAX_DELAY);
            }
          else
            {
              bits = xEventGroupWaitBits (obj->port_.handle, (0 - 1),
              pdTRUE,
                                          pdFALSE, portMAX_DELAY);
            }
          if (oflags != nullptr)
            {
              *oflags = bits;
            }
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        try_wait (rtos::Event_flags* obj, flags::mask_t mask,
                  flags::mask_t* oflags, flags::mode_t mode)
        {
          EventBits_t bits;
          if (rtos::scheduler::in_handler_mode ())
            {
              bits = xEventGroupGetBitsFromISR (obj->port_.handle);
            }
          else
            {
              bits = xEventGroupGetBits(obj->port_.handle);
            }

          if ((mask != 0) && ((mode & flags::mode::all) != 0))
            {
              // Only if all desires signals are raised we're done.
              if ((bits & mask) == mask)
                {
                  if (oflags != nullptr)
                    {
                      *oflags = bits;
                    }
                  // Clear desired signals.
                  bits &= ~mask;
                  return result::ok;
                }
            }
          else if ((mask == 0) || ((mode & flags::mode::any) != 0))
            {
              // Any flag will do it.
              if (bits != 0)
                {
                  // Possibly return.
                  if (oflags != nullptr)
                    {
                      *oflags = bits;
                    }
                  // Since we returned them all, also clear them all.
                  bits = 0;
                  return result::ok;
                }
            }

          return EAGAIN;
        }

        inline static result_t
        __attribute__((always_inline))
        timed_wait (rtos::Event_flags* obj, flags::mask_t mask,
                    flags::mask_t* oflags, flags::mode_t mode, systicks_t ticks)
        {
          EventBits_t bits;

          if (mask != 0)
            {
              bits = xEventGroupWaitBits (
                  obj->port_.handle, mask,
                  ((mode & flags::mode::clear) != 0) ? pdTRUE : pdFALSE,
                  ((mode & flags::mode::all) != 0) ? pdTRUE : pdFALSE, ticks);
            }
          else
            {
              bits = xEventGroupWaitBits (obj->port_.handle, (0 - 1),
              pdTRUE,
                                          pdFALSE, ticks);
            }

          if (oflags != nullptr)
            {
              *oflags = bits;
            }
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        raise (rtos::Event_flags* obj, flags::mask_t mask,
               flags::mask_t* oflags)
        {
          EventBits_t bits;
          if (rtos::scheduler::in_handler_mode ())
            {
              portBASE_TYPE thread_woken = pdFALSE;

              if (xEventGroupSetBitsFromISR (obj->port_.handle, mask,
                                             &thread_woken) == pdFAIL)
                {
                  return ENOTRECOVERABLE;
                }

              bits = xEventGroupGetBitsFromISR (obj->port_.handle);
              portEND_SWITCHING_ISR(thread_woken);
            }
          else
            {
              bits = xEventGroupSetBits (obj->port_.handle, mask);
            }

          if (oflags != nullptr)
            {
              *oflags = bits;
            }

          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        clear (rtos::Event_flags* obj, flags::mask_t mask,
               flags::mask_t* oflags)
        {
          EventBits_t bits;
          if (rtos::scheduler::in_handler_mode ())
            {
              bits = xEventGroupClearBitsFromISR (obj->port_.handle, mask);
            }
          else
            {
              bits = xEventGroupClearBits (obj->port_.handle, mask);
            }

          if (oflags != nullptr)
            {
              *oflags = bits;
            }

          return result::ok;

        }

        inline static flags::mask_t
        __attribute__((always_inline))
        get (rtos::Event_flags* obj, flags::mask_t mask, flags::mode_t mode)
        {
          EventBits_t bits;
          flags::mask_t ret;
          if (rtos::scheduler::in_handler_mode ())
            {
              bits = xEventGroupGetBitsFromISR (obj->port_.handle);
              if (mask == 0)
                {
                  // Return the entire mask.
                  return bits;
                }
              ret = bits & mask;
              if ((mode & flags::mode::clear) != 0)
                {
                  // Clear the selected bits; leave the rest untouched.
                  xEventGroupClearBitsFromISR (obj->port_.handle, ~mask);
                }
            }
          else
            {
              bits = xEventGroupGetBits(obj->port_.handle);
              if (mask == 0)
                {
                  // Return the entire mask.
                  return bits;
                }
              ret = bits & mask;
              if ((mode & flags::mode::clear) != 0)
                {
                  // Clear the selected bits; leave the rest untouched.
                  xEventGroupClearBits (obj->port_.handle, ~mask);
                }
            }

          // Return the selected bits.
          return ret;
        }

        inline static bool
        __attribute__((always_inline))
        waiting (rtos::Event_flags* obj)
        {
          // This is a custom call added to FreeRTOS.
          return xEventFlagsWaiting (obj->port_.handle);
        }
      };

#endif /* OS_INCLUDE_PORT_RTOS_EVENT_FLAGS */

    // ======================================================================

    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* CMSIS_PLUS_RTOS_PORT_OS_INLINES_H_ */