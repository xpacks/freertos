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

#include <cmsis-plus/os-app-config.h>
#include <cmsis-plus/rtos/os-c-decls.h>

// ----------------------------------------------------------------------------

#ifdef  __cplusplus

#include <FreeRTOS.h>
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

#include <cmsis_device.h>
#include <cmsis-plus/diag/trace.h>

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

        inline void
        __attribute__((always_inline))
        greeting (void)
        {
          trace::printf ("FreeRTOS %s scheduler; preemptive.\n",
          tskKERNEL_VERSION_NUMBER);
#if defined(OS_USE_RTOS_PORT_TIMER) \
  || defined(OS_USE_RTOS_PORT_MUTEX) \
  || defined(OS_USE_RTOS_PORT_SEMAPHORE) \
  || defined(OS_USE_RTOS_PORT_MESSAGE_QUEUE) \
  || defined(OS_USE_RTOS_PORT_EVENT_FLAGS)
          trace::printf ("Using FreeRTOS features:");
#if defined(OS_USE_RTOS_PORT_TIMER)
          trace::printf (" timer");
#endif
#if defined(OS_USE_RTOS_PORT_MUTEX)
          trace::printf (" mutex");
#endif
#if defined(OS_USE_RTOS_PORT_SEMAPHORE)
          trace::printf (" semaphore");
#endif
#if defined(OS_USE_RTOS_PORT_MESSAGE_QUEUE)
          trace::printf (" queue");
#endif
#if defined(OS_USE_RTOS_PORT_EVENT_FLAGS)
          trace::printf (" flags");
#endif
          trace::printf (".\n");
#endif
        }

        [[noreturn]] inline void
        __attribute__((always_inline))
        start (void)
        {
          vTaskStartScheduler ();
          for (;;)
            ;
        }

#if 1

        inline void
        __attribute__((always_inline))
        lock (rtos::scheduler::status_t status)
        {
          if (status)
            {
              vTaskSuspendAll ();
            }
          else
            {
              xTaskResumeAll ();
            }
        }

#else

        inline void
        __attribute__((always_inline))
        lock (void)
          {
            vTaskSuspendAll ();
          }

        inline void
        __attribute__((always_inline))
        unlock (void)
          {
            xTaskResumeAll ();
          }

#endif

        inline void
        reschedule (void)
        {
          // Custom extension.
          vTaskPerformSuspend ();
        }
      } /* namespace scheduler */

      namespace interrupts
      {
        inline bool
        __attribute__((always_inline))
        in_handler_mode (void)
        {
          // In Handler mode, IPSR holds the exception number.
          // If 0, the core is in thread mode.
          return (__get_IPSR () != 0);
        }

        // Enter an IRQ critical section
        inline rtos::interrupts::status_t
        __attribute__((always_inline))
        critical_section::enter (void)
        {
#if 0
          // TODO: on M0 & M0+ cores there is no BASEPRI
          uint32_t pri = __get_BASEPRI ();
          __set_BASEPRI_MAX (
              OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY
              << ((8 - __NVIC_PRIO_BITS)));

          return pri;
#else
          if (!interrupts::in_handler_mode ())
            {
              taskENTER_CRITICAL();
              return 0;
            }
          else
            {
              return portSET_INTERRUPT_MASK_FROM_ISR();
            }
#endif
        }

        // Exit an IRQ critical section
        inline void
        __attribute__((always_inline))
        critical_section::exit (
            rtos::interrupts::status_t status __attribute__((unused)))
        {
#if 0
          __set_BASEPRI (status);
#else
          if (!interrupts::in_handler_mode ())
            {
              taskEXIT_CRITICAL();
            }
          else
            {
              portCLEAR_INTERRUPT_MASK_FROM_ISR(status);
            }
#endif
        }

        // ====================================================================

        // Enter an IRQ uncritical section
        inline rtos::interrupts::status_t
        __attribute__((always_inline))
        uncritical_section::enter (void)
        {
#if 1
          // TODO: on M0 & M0+ cores there is no BASEPRI
          uint32_t pri = __get_BASEPRI ();
          __set_BASEPRI (0);

          return pri;
#else
          if (!scheduler::in_handler_mode ())
            {
              taskENTER_CRITICAL();
              return 0;
            }
          else
            {
              return portSET_INTERRUPT_MASK_FROM_ISR();
            }
#endif
        }

        // Exit an IRQ critical section
        inline void
        __attribute__((always_inline))
        uncritical_section::exit (
            rtos::interrupts::status_t status __attribute__((unused)))
        {
#if 1
          __set_BASEPRI (status);
#else
          if (!scheduler::in_handler_mode ())
            {
              taskEXIT_CRITICAL();
            }
          else
            {
              portCLEAR_INTERRUPT_MASK_FROM_ISR(status);
            }
#endif
        }

        inline bool
        __attribute__((always_inline))
        is_priority_valid (void)
        {
          return true;
        }

      } /* namespace interrupts */

#if 1
      inline void
      __attribute__((always_inline))
      clock_systick::start (void)
      {
        ;
      }

      inline result_t
      __attribute__((always_inline))
      clock_systick::wait_for (clock::duration_t ticks)
      {
        vTaskDelay (ticks);
        return result::ok;
      }

      extern "C" void
      xPortSysTickHandler (void);

      inline void
      __attribute__((always_inline))
      clock_systick::_interrupt_service_routine (void)
      {
        xPortSysTickHandler ();
      }

      inline void
      __attribute__((always_inline))
      clock_rtc::_interrupt_service_routine (void)
      {
        ;
      }
#endif

      // ======================================================================

#if defined(OS_USE_RTOS_PORT_SCHEDULER)

      namespace this_thread
      {
        inline rtos::thread*
        __attribute__((always_inline))
        thread (void)
        {
          TaskHandle_t th = xTaskGetCurrentTaskHandle ();
          if (th == nullptr)
            {
              return nullptr;
            }
          void* p = pvTaskGetThreadLocalStoragePointer (th, 0);
          return ((rtos::thread*) p);
        }

        inline void
        __attribute__((always_inline))
        yield (void)
        {
          taskYIELD()
          ;
        }

        inline void
        __attribute__((always_inline))
        prepare_suspend (void)
        {
          // Custom extension.
          vTaskPrepareSuspend ();
        }

      } /* namespace this_thread */

      inline unsigned portBASE_TYPE
      makeFreeRtosPriority (rtos::thread::priority_t priority)
      {
        unsigned portBASE_TYPE fr_prio = tskIDLE_PRIORITY;

        fr_prio += (priority - rtos::thread::priority::idle);

        return fr_prio;
      }

      inline rtos::thread::priority_t
      makeCmsisPriority (unsigned portBASE_TYPE priority)
      {
        rtos::thread::priority_t cm_prio = rtos::thread::priority::idle;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
        cm_prio += (rtos::thread::priority_t) (priority - tskIDLE_PRIORITY);
#pragma GCC diagnostic pop

        return cm_prio;
      }

      inline void
      __attribute__((always_inline))
      thread::create (rtos::thread* obj)
      {
        StackType_t stack_size_words = (StackType_t) (
            obj->stack ().size () / (sizeof(StackType_t)));
        // stack_size_words &= ~1;

        assert(stack_size_words >= configMINIMAL_STACK_SIZE);

        StackType_t* stack_address =
            static_cast<StackType_t*> (obj->stack ().bottom ());

        // Preserve the first stack word, to check the magic
        // on thread termination.
        stack_address++;
        stack_size_words--;

        TaskHandle_t th;
        th = xTaskCreateStatic (
            (TaskFunction_t) rtos::thread::_invoke_with_exit,
            (const portCHAR *) obj->name (), stack_size_words, obj,
            makeFreeRtosPriority (obj->prio_), stack_address, &obj->port_.task);
        if (th != NULL)
          {
            obj->func_result_ = nullptr;

            // Remember pointer to implementation.
            obj->port_.handle = th;

            // Store the pointer to this thread as index 0 in the FreeRTOS
            // local storage pointers.
            vTaskSetThreadLocalStoragePointer (th, 0, obj);
#if defined(OS_TRACE_RTOS_THREAD)
            trace::printf ("port::thread::%s @%p %s = %p\n", __func__, obj,
                           obj->name (), th);
#endif

          }
        else
          {
            assert(th != NULL);
          }
      }

      [[noreturn]]
      inline void
      __attribute__((always_inline))
      thread::destroy_this (rtos::thread* obj)
      {
        void* handle = obj->port_.handle;
        // Remove the reference to the destroyed thread.
        obj->port_.handle = nullptr;
        trace::printf ("port::%s() vTaskDelete(%p)\n", __func__, handle);

        vTaskDelete (nullptr);
        assert(true);
        for (;;)
          ;
        // Does not return
      }

      inline void
      __attribute__((always_inline))
      thread::destroy_other (rtos::thread* obj)
      {
        void* handle = obj->port_.handle;
        if (handle != nullptr)
          {
            // Remove the reference to the destroyed thread.
            obj->port_.handle = nullptr;
            trace::printf ("port::%s() vTaskDelete(%p)\n", __func__, handle);
            vTaskDelete (handle);
          }
        // Does return
      }

      inline void
      __attribute__((always_inline))
      thread::resume (rtos::thread* obj)
      {
        if (rtos::interrupts::in_handler_mode ())
          {
            BaseType_t must_yield = xTaskResumeFromISR (obj->port_.handle);
            portEND_SWITCHING_ISR(must_yield);
          }
        else
          {
            vTaskGenericResume (obj->port_.handle);
            // trace_putchar('^');
            taskYIELD()
            ;
          }
      }

      inline rtos::thread::priority_t
      __attribute__((always_inline))
      thread::sched_prio (rtos::thread* obj)
      {
        UBaseType_t p = uxTaskPriorityGet (obj->port_.handle);
        rtos::thread::priority_t prio = makeCmsisPriority (p);
        assert(prio == obj->prio_);

        return prio;
      }

      inline result_t
      __attribute__((always_inline))
      thread::sched_prio (rtos::thread* obj, rtos::thread::priority_t prio)
      {
        obj->prio_ = prio;

        vTaskPrioritySet (obj->port_.handle, makeFreeRtosPriority (prio));

        // The manual says it is done, but apparently still needed.
        taskYIELD()
        ;

        return result::ok;
      }

      inline result_t
      __attribute__((always_inline))
      thread::detach (rtos::thread* obj __attribute__((unused)))
      {
        return result::ok;
      }

      // --------------------------------------------------------------------

#endif /* OS_USE_RTOS_PORT_SCHEDULER */

      // ======================================================================

#if defined(OS_USE_RTOS_PORT_TIMER)

      class timer
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::timer* obj, rtos::timer::func_t function,
                rtos::timer::func_args_t args)
        {
          // Start with 1 tick period and change it at start(), when the
          // actual period is known.
          // Note: args are passed as ID, this requires a small patch in
          // timers.c to pass the pointer back.
          obj->port_.handle = xTimerCreateStatic (
              obj->name (), 1,
              obj->type_ == rtos::timer::run::once ? pdFALSE : pdTRUE,
              (void*) args, (TaskFunction_t) function, &obj->port_.timer);

          os_assert_throw (obj->port_.handle != NULL, ENOMEM);
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::timer* obj)
        {
          BaseType_t ret = xTimerDelete(obj->port_.handle, portMAX_DELAY);

          os_assert_throw (ret == pdTRUE, ENOTRECOVERABLE);
        }

        inline static result_t
        __attribute__((always_inline))
        start (rtos::timer* obj, clock::duration_t ticks)
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
              if (xTimerChangePeriod(obj->port_.handle, ticks, 10) == pdFAIL)
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
        stop (rtos::timer* obj)
        {
          if (xTimerIsTimerActive (obj->port_.handle) == pdFALSE)
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

        // --------------------------------------------------------------------
      };

#endif /* OS_USE_RTOS_PORT_TIMER */

      // ======================================================================

#if defined(OS_USE_RTOS_PORT_MUTEX)

      class mutex
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::mutex* obj)
        {
          if (obj->type_ == rtos::mutex::type::recursive)
            {
              obj->port_.handle = xSemaphoreCreateRecursiveMutexStatic(
                  &obj->port_.mutex);
            }
          else
            {
              obj->port_.handle = xSemaphoreCreateMutexStatic(
                  &obj->port_.mutex);
            }
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::mutex* obj)
        {
          vSemaphoreDelete(obj->port_.handle);
        }

        inline static result_t
        __attribute__((always_inline))
        lock (rtos::mutex* obj)
        {
          BaseType_t res;

          if (obj->type_ == rtos::mutex::type::recursive)
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

          obj->owner_ = &rtos::this_thread::thread ();
          ++obj->count_;
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        try_lock (rtos::mutex* obj)
        {
          BaseType_t res;

          if (obj->type_ == rtos::mutex::type::recursive)
            {
              res = xSemaphoreTakeRecursive(obj->port_.handle, 0);
            }
          else
            {
              res = xSemaphoreTake(obj->port_.handle, 0);
            }

          if (res != pdTRUE)
            {
              return EWOULDBLOCK;
            }

          obj->owner_ = &rtos::this_thread::thread ();
          ++obj->count_;
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        timed_lock (rtos::mutex* obj, clock::duration_t ticks)
        {
          BaseType_t res;

          if (ticks == 0)
            {
              ticks = 1;
            }

          if (obj->type_ == rtos::mutex::type::recursive)
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

          obj->owner_ = &rtos::this_thread::thread ();
          ++obj->count_;
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        unlock (rtos::mutex* obj)
        {
          BaseType_t res;

          if (obj->owner_ != &rtos::this_thread::thread ())
            {
              return EPERM;
            }

          if (obj->type_ == rtos::mutex::type::recursive)
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

          --obj->count_;
          if (obj->count_ == 0)
            {
              obj->owner_ = nullptr;
            }

          return result::ok;
        }

        inline static rtos::thread::priority_t
        __attribute__((always_inline))
        prio_ceiling (const rtos::mutex* obj)
        {
          return obj->prio_ceiling_;
        }

        inline static result_t
        __attribute__((always_inline))
        prio_ceiling (rtos::mutex* obj, rtos::thread::priority_t prio_ceiling,
                      rtos::thread::priority_t* old_prio_ceiling)
        {
          rtos::thread::priority_t prio;
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
        consistent (rtos::mutex* obj __attribute__((unused)))
        {
          return result::ok;
        }

        // --------------------------------------------------------------------
      };
#endif /* OS_USE_RTOS_PORT_MUTEX */

      // ======================================================================

#if defined(OS_USE_RTOS_PORT_CONDITION_VARIABLE)

      class condition_variable
        {
        public:

        };

#endif /* OS_USE_RTOS_PORT_CONDITION_VARIABLE */

      // ======================================================================

#if defined(OS_USE_RTOS_PORT_SEMAPHORE)

      class semaphore
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::semaphore* obj)
        {
          UBaseType_t max = obj->max_count_;
          if (max == 0)
            {
              max = 1;
            }
          obj->port_.handle = xSemaphoreCreateCountingStatic(
              max, obj->initial_count_, &obj->port_.semaphore);
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::semaphore* obj)
        {
          vSemaphoreDelete(obj->port_.handle);
        }

        inline static result_t
        __attribute__((always_inline))
        post (rtos::semaphore* obj)
        {
#if 0
          if (obj->count_ >= obj->max_count_)
            {
              return EOVERFLOW;
            }
#endif
          portBASE_TYPE thread_woken = pdFALSE;

          if (rtos::interrupts::in_handler_mode ())
            {
              if (xSemaphoreGiveFromISR(obj->port_.handle,
                  &thread_woken) != pdTRUE)
                {
                  return EAGAIN;
                }
              portEND_SWITCHING_ISR(thread_woken);
            }
          else
            {
              if (xSemaphoreGive (obj->port_.handle) != pdTRUE)
                {
                  return EAGAIN;
                }
            }
          ++(obj->count_);

          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        wait (rtos::semaphore* obj)
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
        try_wait (rtos::semaphore* obj)
        {
          portBASE_TYPE thread_woken = pdFALSE;

          if (rtos::interrupts::in_handler_mode ())
            {
              if (xSemaphoreTakeFromISR(obj->port_.handle,
                  &thread_woken) != pdTRUE)
                {
                  return EWOULDBLOCK;
                }
              portEND_SWITCHING_ISR(thread_woken);
            }
          else if (xSemaphoreTake (obj->port_.handle, 0) != pdTRUE)
            {
              return EWOULDBLOCK;
            }

          --(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        timed_wait (rtos::semaphore* obj, clock::duration_t ticks)
        {

          if (ticks == 0)
            {
              ticks = 1;
            }

          if (xSemaphoreTake(obj->port_.handle,
              ticks) != pdTRUE)
            {
              return ETIMEDOUT;
            }

          --(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        reset (rtos::semaphore* obj)
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

        // --------------------------------------------------------------------
      };

#endif /* OS_USE_RTOS_PORT_SEMAPHORE */

      // ======================================================================

#if defined(OS_USE_RTOS_PORT_MEMORY_POOL)

      class memory_pool
        {
        public:

        };

#endif /* OS_USE_RTOS_PORT_MEMORY_POOL */

      // ======================================================================

#if defined(OS_USE_RTOS_PORT_MESSAGE_QUEUE)

      class message_queue
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::message_queue* obj)
        {
          obj->port_.handle = xQueueCreateStatic(
              obj->msgs_, obj->msg_size_bytes_,
              static_cast<uint8_t*> (obj->queue_addr_), &obj->port_.queue);
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::message_queue* obj)
        {
          vQueueDelete (obj->port_.handle);
        }

        inline static result_t
        __attribute__((always_inline))
        send (rtos::message_queue* obj, const void* msg,
              std::size_t nbytes __attribute__((unused)),
              rtos::message_queue::priority_t mprio __attribute__((unused)))
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
        try_send (rtos::message_queue* obj, const void* msg,
                  std::size_t nbytes __attribute__((unused)),
                  rtos::message_queue::priority_t mprio __attribute__((unused)))
        {
          portBASE_TYPE thread_woken = pdFALSE;

          if (rtos::interrupts::in_handler_mode ())
            {
              if (xQueueSendFromISR(obj->port_.handle, msg,
                  &thread_woken) != pdTRUE)
                {
                  return EWOULDBLOCK;
                }
              portEND_SWITCHING_ISR(thread_woken);
            }
          else
            {
              if (xQueueSend (obj->port_.handle, msg, 0) != pdTRUE)
                {
                  return EWOULDBLOCK;
                }
            }

          ++(obj->count_);
          return result::ok;
        }

        inline static result_t
        __attribute__((always_inline))
        timed_send (
            rtos::message_queue* obj, const void* msg,
            std::size_t nbytes __attribute__((unused)), clock::duration_t ticks,
            rtos::message_queue::priority_t mprio __attribute__((unused)))
        {

          if (ticks == 0)
            {
              ticks = 1;
            }

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
        receive (rtos::message_queue* obj, void* msg,
                 std::size_t nbytes __attribute__((unused)),
                 rtos::message_queue::priority_t* mprio __attribute__((unused)))
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
        try_receive (
            rtos::message_queue* obj, void* msg,
            std::size_t nbytes __attribute__((unused)),
            rtos::message_queue::priority_t* mprio __attribute__((unused)))
        {
          portBASE_TYPE thread_woken = pdFALSE;

          if (rtos::interrupts::in_handler_mode ())
            {
              if (xQueueReceiveFromISR (obj->port_.handle, msg,
                                        &thread_woken) != pdTRUE)
                {
                  return EWOULDBLOCK;
                }
              portEND_SWITCHING_ISR(thread_woken);
            }
          else
            {
              if (xQueueReceive (obj->port_.handle, msg, 0) != pdTRUE)
                {
                  return EWOULDBLOCK;
                }
            }

          --(obj->count_);
          return result::ok;

        }

        inline static result_t
        __attribute__((always_inline))
        timed_receive (
            rtos::message_queue* obj, void* msg,
            std::size_t nbytes __attribute__((unused)), clock::duration_t ticks,
            rtos::message_queue::priority_t* mprio __attribute__((unused)))
        {

          if (ticks == 0)
            {
              ticks = 1;
            }

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
        reset (rtos::message_queue* obj)
        {
          xQueueReset(obj->port_.handle);

          obj->count_ = 0;
          return result::ok;
        }

        // --------------------------------------------------------------------
      };

#endif /* OS_USE_RTOS_PORT_MESSAGE_QUEUE */

      // ======================================================================

#if defined(OS_USE_RTOS_PORT_EVENT_FLAGS)

      class event_flags
      {
      public:

        inline static void
        __attribute__((always_inline))
        create (rtos::event_flags* obj)
        {
          obj->port_.handle = xEventGroupCreateStatic (&obj->port_.flags);
        }

        inline static void
        __attribute__((always_inline))
        destroy (rtos::event_flags* obj)
        {
          vEventGroupDelete (obj->port_.handle);
        }

        inline static result_t
        __attribute__((always_inline))
        wait (rtos::event_flags* obj, flags::mask_t mask, flags::mask_t* oflags,
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
        try_wait (rtos::event_flags* obj, flags::mask_t mask,
                  flags::mask_t* oflags, flags::mode_t mode)
        {
          EventBits_t bits;
          if (rtos::interrupts::in_handler_mode ())
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

          return EWOULDBLOCK;
        }

        inline static result_t
        __attribute__((always_inline))
        timed_wait (rtos::event_flags* obj, flags::mask_t mask,
                    clock::duration_t ticks, flags::mask_t* oflags,
                    flags::mode_t mode)
        {
          EventBits_t bits;

          if (ticks == 0)
            {
              ticks = 1;
            }

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
        raise (rtos::event_flags* obj, flags::mask_t mask,
               flags::mask_t* oflags)
        {
          EventBits_t bits;
          if (rtos::interrupts::in_handler_mode ())
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
        clear (rtos::event_flags* obj, flags::mask_t mask,
               flags::mask_t* oflags)
        {
          EventBits_t bits;
          if (rtos::interrupts::in_handler_mode ())
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
        get (rtos::event_flags* obj, flags::mask_t mask, flags::mode_t mode)
        {
          EventBits_t bits;
          flags::mask_t ret;

          // FreeRTOS reserves some bits for system usage.
          mask &= ~0xff000000UL;
          if (rtos::interrupts::in_handler_mode ())
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
                  xEventGroupClearBitsFromISR (obj->port_.handle,
                                               ~mask & ~0xff000000UL);
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
                  xEventGroupClearBits (obj->port_.handle,
                                        ~mask & ~0xff000000UL);
                }
            }

          // Return the selected bits.
          return ret;
        }

        inline static bool
        __attribute__((always_inline))
        waiting (rtos::event_flags* obj)
        {
          // This is a custom call added to FreeRTOS.
          return xEventFlagsWaiting (obj->port_.handle);
        }

        // --------------------------------------------------------------------
      };

#endif /* OS_USE_RTOS_PORT_EVENT_FLAGS */

      // ======================================================================

      inline void
      __attribute__((always_inline))
      clock_highres::start (void)
      {
        ;
      }

      inline uint32_t
      __attribute__((always_inline))
      clock_highres::input_clock_frequency_hz (void)
      {
        // The SysTick is clocked with the CPU clock.
        return SystemCoreClock;
      }

      inline uint32_t
      __attribute__((always_inline))
      clock_highres::cycles_per_tick (void)
      {
        return SysTick->LOAD + 1;
      }

      inline uint32_t
      __attribute__((always_inline))
      clock_highres::cycles_since_tick (void)
      {
        uint32_t load_value = SysTick->LOAD;

        // Initial sample of the decrementing counter.
        // If this happens before the event, will be used as such.
        uint32_t val = SysTick->VAL;

        // Check overflow. If the exception is pending, it means that the
        // interrupt occurred during this critical section and was not
        // yet processed, so the total cycles count in steady_count_
        // does not yet reflect the correct value and needs to be
        // adjusted by one full cycle length.
        if (SysTick->CTRL & SCB_ICSR_PENDSTSET_Msk)
          {
            // Sample the decrementing counter again to validate the
            // initial sample.
            uint32_t val_subsequent = SysTick->VAL;

            // If the value did decrease, the timer did not recycle
            // between the two reads; in other words, the interrupt
            // occurred before the first read.
            if (val > val_subsequent)
              {
                // The count must be adjusted with a full cycle.
                return load_value + 1 + (load_value - val);
              }
          }

        return load_value - val;
      }

    // ========================================================================

    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* CMSIS_PLUS_RTOS_PORT_OS_INLINES_H_ */
