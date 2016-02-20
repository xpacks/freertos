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

#include <cmsis-plus/rtos/os.h>
#include <cmsis-plus/diag/trace.h>

#include <cmsis-plus/rtos/os-impl-inlines.h>

// ----------------------------------------------------------------------------

#include <cassert>
#include <cerrno>
#include <cstdlib>

// ----------------------------------------------------------------------------

namespace os
{
  namespace rtos
  {
    namespace thread
    {
      using main_func_t = int (*) (int argc, char* argv[]);

      using main_args_t = struct
        {
          main_func_t func;
          int argc;
          char** argv;
        };

      static void
      main_trampoline (main_args_t* args)
      {
        std::exit (args->func (args->argc, args->argv));
      }

    } /* namespace thread */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

int __attribute__((weak))
main (int argc, char* argv[])
{
  using namespace os::rtos;

  scheduler::initialize ();

  static thread::main_args_t args;
  args.func = os_main;
  args.argc = argc;
  args.argv = argv;

  // Necessarily static.
  static thread::Attributes attr
    { "main" };
  static Thread main_thread
    { attr, (thread::func_t) thread::main_trampoline,
        (thread::func_args_t) &args };

  scheduler::start ();

  main_thread.join ();

  return 0;
}

// ----------------------------------------------------------------------------

namespace os
{
  namespace rtos
  {
    static Systick_clock::rep __systick_now;
    static Realtime_clock::rep __rtc_now;

  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

void
os_systick_handler (void)
{
  using namespace os::rtos;

  // Prevent scheduler actions before starting it.
  if (scheduler::is_started ())
    {
      os_impl_systick_handler ();
    }
  __systick_now++;

#if !defined(OS_INCLUDE_REALTIME_CLOCK_DRIVER)
  static uint32_t ticks = Systick_clock::frequency_hz;

  if (--ticks == 0)
    {
      ticks = Systick_clock::frequency_hz;

      os_rtc_handler ();
    }
#endif
}

void
os_rtc_handler (void)
{
  using namespace os::rtos;

  // Prevent scheduler actions before starting it.
  if (scheduler::is_started ())
    {
      os_impl_rtc_handler ();
    }
  ++__rtc_now;
}

void __attribute__((weak))
os_impl_systick_handler (void)
{
  // TODO
}

void __attribute__((weak))
os_impl_rtc_handler (void)
{
  // TODO
}

// ----------------------------------------------------------------------------

namespace os
{
  namespace rtos
  {

#pragma GCC diagnostic push
// TODO: remove it when fully implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

    // ======================================================================

    /**
     * @details
     *
     * @note Can be invoked from Interrupt Service Routines.
     */
    Systick_clock::rep
    Systick_clock::now (void)
    {
      Critical_section_irq cs;

      // Prevent inconsistent values using the critical section.
      return __systick_now;
    }

    /**
     * @details
     * Return the very accurate current time, using the internal
     * SysTick cycle counter, which, at 100 MHz and 1000 ticks/sec
     * allows a 10 ns resolution.
     *
     * The function adjust for border conditions, when the SysTick
     * counter is reloaded during the call.
     *
     * @note Can be invoked from Interrupt Service Routines.
     */
    Systick_clock::rep
    Systick_clock::now (current_t* details)
    {
      assert(details != nullptr);

      // The core frequency can be returned right away, since
      // is not expected to change during this call.
      details->core_frequency_hz = SystemCoreClock;

      // The timer reload value, it is the divisor - 1.
      uint32_t load = SysTick->LOAD;
      // Can also be returned right away.
      details->divisor = load + 1;

      // The ticks and cycles must be read atomically, use a critical
      // section and adjust for border condition, when the event
      // came exactly after entering the critical section.
      uint64_t ticks;
      uint32_t val;
        {
          Critical_section_irq cs;

          // Sample ticks counter inside critical section.
          ticks = __systick_now;

          // Initial sample of the decrementing counter.
          // Might happen before the event, will be used as such.
          val = SysTick->VAL;

          // Check overflow. If the exception is pending, it means the
          // ticks counter was not yet updated, but the counter was reloaded.
          if (SysTick->CTRL & SCB_ICSR_PENDSTSET_Msk)
            {
              // Sample the decrementing counter again to validate the
              // initial sample.
              uint32_t val_update = SysTick->VAL;
              if (val_update > val)
                {
                  // The second sample is more accurate.
                  val = val_update;
                }
              ticks++; // Adjust to next tick.
            }
        }
      details->cycles = load - val;
      details->ticks = ticks;

      return ticks;
    }

    /**
     * @details
     * Put the current thread to sleep, until the next n-th
     * SysTick occurs. Depending when the call is issued, the
     * first tick counted may be very short.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Systick_clock::sleep_for (Systick_clock::sleep_rep ticks)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("Systick_clock::sleep_for(%d_ticks)\n", ticks);

      vTaskDelay (ticks);

      return result::ok;
    }

    /**
     * @details
     *
     * @note Can be invoked from Interrupt Service Routines.
     */
    Realtime_clock::rep
    Realtime_clock::now (void)
    {
      return __rtc_now;
    }

    /**
     * @details
     * Put the current thread to sleep, until the next n-th
     * RTC second occurs. Depending when the call is issued, the
     * first second counted may be very short.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Realtime_clock::sleep_for (Realtime_clock::sleep_rep secs)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("Realtime_clock::sleep_for(%ds)\n", secs);
      __rtc_now += secs;
      return result::ok;
    }

    namespace scheduler
    {
      status_t is_started_ = false;
      status_t is_locked_ = false;

      /**
       * @details
       * Create all RTOS internal objects and be ready to run.
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      result_t
      initialize (void)
      {
        os_assert_err(!scheduler::in_handler_mode (), EPERM);

        trace::printf ("%s() \n", __func__);

        return impl::scheduler::initialize ();
      }

      /**
       *
       * @note Can be invoked from Interrupt Service Routines (obviously).
       */
      bool
      in_handler_mode (void)
      {
        // In Handler mode, IPSR holds the exception number.
        // If 0, the core is in thread mode.
        return (__get_IPSR () != 0);
      }

      /**
       * @details
       * The scheduler cannot be stopped, will run forever, but
       * thread switching can be locked/unlocked.
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      result_t
      start (void)
      {
        os_assert_err(!scheduler::in_handler_mode (), EPERM);

        trace::printf ("%s() \n", __func__);

        is_started_ = true;
        is_locked_ = false;

        return impl::scheduler::start ();
      }

      /**
       * @details
       * Lock the scheduler (prevent it for doing thread switches) and
       * return the previous status, to be restored by unlock().
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      status_t
      lock (void)
      {
        os_assert_throw(!scheduler::in_handler_mode (), EPERM);

        impl::scheduler::lock ();

        status_t tmp = is_locked_;
        is_locked_ = true;
        return tmp;
      }

      /**
       * @details
       * Actually restore the scheduler status based on the given
       * parameter, usually returned by a lock(). This allows for
       * embedded critical sections to preserve the locked status
       * until the outer one completes and invokes unlock().
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      status_t
      unlock (status_t status)
      {
        os_assert_throw(!scheduler::in_handler_mode (), EPERM);

        status_t tmp = is_locked_;

        impl::scheduler::unlock (status);

        is_locked_ = status;

        return tmp;
      }
    } /* namespace scheduler */

#if 0
    namespace scheduler
      {

        void
        __register_thread (Thread* thread)
          {
            // TODO
#if defined(TESTING)
            thread->__run_function ();
#endif
          }

        void
        __unregister_thread (Thread* thread)
          {
            return;
          }

      } /* namespace scheduler */
#endif

    namespace critical
    {
      // Enter an IRQ critical section
      status_t
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
      status_t
      exit (status_t status)
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

    // ======================================================================

    // ----- Signal flags -----

    namespace flags
    {
      /**
       * @details
       *
       * @note Can be invoked from Interrupt Service Routines.
       */
      result_t
      set (Thread& thread, event_flags_t flags, event_flags_t* out_flags)
      {
        return result::ok;
      }

      /**
       * @details
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      result_t
      clear (Thread& thread, event_flags_t flags, event_flags_t* out_flags)
      {
        os_assert_err(!scheduler::in_handler_mode (), EPERM);

        return result::ok;
      }

      /**
       * @details
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      result_t
      wait (event_flags_t flags, event_flags_t* out_flags)
      {
        os_assert_err(!scheduler::in_handler_mode (), EPERM);

        return result::ok;
      }

      /**
       * @details
       *
       * @note Can be invoked from Interrupt Service Routines.
       */
      result_t
      try_wait (event_flags_t flags, event_flags_t* ret)
      {
        return result::ok;
      }

      /**
       * @details
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      result_t
      timed_wait (event_flags_t flags, event_flags_t* out_flags,
                  systicks_t ticks)
      {
        os_assert_err(!scheduler::in_handler_mode (), EPERM);

        return result::ok;
      }

    } /* namespace flags */

// ======================================================================

#if 0
    void*
    no_thread_func (void* args);

    void*
    no_thread_func (void* args)
      {
        return nullptr;
      }

    Thread no_thread
      { nullptr, (thread::func_t) no_thread_func, nullptr};
#endif

    namespace this_thread
    {
      /**
       * @details
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      Thread&
      thread (void)
      {
        os_assert_throw(!scheduler::in_handler_mode (), EPERM);

        TaskHandle_t th = xTaskGetCurrentTaskHandle ();

        void* p = pvTaskGetThreadLocalStoragePointer (th, 0);
        assert(p != nullptr);

        return *((Thread*) p);
      }

      /**
       * @details
       * Pass control to next thread that is in \b READY state.
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      void
      yield (void)
      {
        os_assert_throw(!scheduler::in_handler_mode (), EPERM);

        taskYIELD();
      }

      /**
       * @details
       * Remove the current thread from the ready list and pass
       * control to the next thread that is in \b READY state.
       *
       * @warning Cannot be invoked from Interrupt Service Routines.
       */
      void
      suspend (void)
      {
        os_assert_throw(!scheduler::in_handler_mode (), EPERM);

        this_thread::thread ().suspend ();
      }

    } /* namespace this_thread */

// ======================================================================

    Named_object::Named_object (const char* name) :
        name_ (name != nullptr ? name : "-")
    {
      ;
    }

// ======================================================================

    namespace thread
    {
      const Attributes initializer
        { nullptr };
    } /* namespace thread */

    static unsigned portBASE_TYPE
    makeFreeRtosPriority (thread::priority_t priority)
    {
      unsigned portBASE_TYPE fr_prio = tskIDLE_PRIORITY;

      fr_prio += (priority - thread::priority::idle);

      return fr_prio;
    }

    static thread::priority_t
    makeCmsisPriority (unsigned portBASE_TYPE priority)
    {
      thread::priority_t cm_prio = thread::priority::idle;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
      cm_prio += (thread::priority_t) (priority - tskIDLE_PRIORITY);
#pragma GCC diagnostic pop

      return cm_prio;
    }

    void
    Thread::trampoline (Thread* thread)
    {
      thread->exit (thread->func_ (thread->func_args_));
    }

    /**
     * @details
     *
     * Create a new thread, with default attributes.
     *
     * The thread is created executing function with args as its
     * sole argument. If the start_routine returns, the effect
     * shall be as if there was an implicit call to exit() using
     * the return value of function as the exit status. Note that
     * the thread in which main() was originally invoked differs
     * from this. When it returns from main(), the effect shall
     * be as if there was an implicit call to exit() using the
     * return value of main() as the exit status.
     *
     * Compatible with pthread_create().
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Thread::Thread (thread::func_t function, thread::func_args_t args) :
        Thread
          { thread::initializer, function, args }
    {
      ;
    }

    /**
     * @details
     *
     * Create a new thread, with attributes specified by attr.
     * If attr is NULL, the default attributes shall be used.
     * If the attributes specified by attr are modified later,
     * the thread's attributes shall not be affected.
     *
     * The thread is created executing function with args as its
     * sole argument. If the start_routine returns, the effect
     * shall be as if there was an implicit call to exit() using
     * the return value of function as the exit status. Note that
     * the thread in which main() was originally invoked differs
     * from this. When it returns from main(), the effect shall
     * be as if there was an implicit call to exit() using the
     * return value of main() as the exit status.
     *
     * Compatible with pthread_create().
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Thread::Thread (const thread::Attributes& attr, thread::func_t function,
                    thread::func_args_t args) :
        Named_object
          { attr.name () }
    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      assert(function != nullptr);
      assert(attr.th_priority != thread::priority::none);

      // Get attributes from user structure.
      prio_ = attr.th_priority;
      stack_size_bytes_ = attr.th_stack_size_bytes;
      stack_addr_ = attr.th_stack_address;

      func_ = function;
      func_args_ = args;

      trace::printf ("%s @%p %s\n", __func__, this, name ());

#if 0
      // TODO: check min size
      // TODO: align stack
      if (stack_addr_ == nullptr)
        {
          // TODO: alloc default stack size
        }
      state_ = thread::state::inactive;

      scheduler::__register_thread (this);

#else

      uint16_t stack_size_words = (uint16_t) (stack_size_bytes_
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
      res = xTaskCreate((TaskFunction_t ) trampoline,
                        (const portCHAR *) name (), stack_size_words, this,
                        makeFreeRtosPriority (prio_), &th);
      assert(res == pdPASS);

      func_result_ = nullptr;

      // Remember pointer to implementation.
      impl_ = th;

      // Store the pointer to this Thread as index 0 in the FreeRTOS
      // local storage pointers.
      vTaskSetThreadLocalStoragePointer (th, 0, this);

      EventGroupHandle_t eh = xEventGroupCreate ();
      xEventGroupClearBits (eh, 1);

      impl_event_flags_ = eh;
#endif
    }

    /**
     * @details
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Thread::~Thread ()
    {
      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      vEventGroupDelete (impl_event_flags_);

#if 0
      scheduler::__unregister_thread (this);
#endif
    }

    void
    Thread::suspend (void)
    {
      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      vTaskSuspend (impl_);
    }

    /**
     * @details
     * Internal, no POSIX equivalent.
     *
     * @note Can be invoked from Interrupt Service Routines.
     */
    void
    Thread::wakeup (void)
    {
      trace::printf ("%s() @%p %s\n", __func__, this, name ());
      wakeup_reason_ = result::ok;

      vTaskResume (impl_);
      // TODO
    }

#if 0
    /**
     * @details
     * Internal, no POSIX equivalent, used to notify timeouts or cancels.
     *
     * @note Can be invoked from Interrupt Service Routines.
     */
    void
    Thread::wakeup (result_t reason)
      {
        assert(reason == EINTR || reason == ETIMEDOUT);

        trace::printf ("%s(&d) @%p %s \n", __func__, reason, this, name ());
        wakeup_reason_ = reason;

        // TODO
      }
#endif

    /**
     * @details
     *
     * No POSIX equivalent.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    thread::priority_t
    Thread::sched_prio (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), thread::priority::error);

      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      UBaseType_t p = uxTaskPriorityGet (impl_);
      thread::priority_t prio = makeCmsisPriority (p);
      assert(prio == prio_);

      return prio;
    }

    /**
     * @details
     * Set the scheduling priority for the thread to the value given
     * by prio.
     *
     * pthread_setschedprio()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_setschedprio.html
     *
     * If an implementation detects use of a thread ID after the end
     * of its lifetime, it is recommended that the function should
     * fail and report an [ESRCH] error.
     *
     * The pthread_setschedprio() function shall not return an error
     * code of [EINTR].
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Thread::sched_prio (thread::priority_t prio)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      assert(prio != thread::priority::none);

      trace::printf ("%s(%d) @%p %s\n", __func__, prio, this, name ());

      prio_ = prio;

      vTaskPrioritySet (impl_, makeFreeRtosPriority (prio));

      return result::ok;
    }

    /**
     * @details
     * Suspend execution of the calling thread until the target thread
     * terminates, unless the target thread has already terminated.
     * On return from a successful join() call with a non-NULL
     * exit_ptr argument, the value passed to exit() by the
     * terminating thread shall be made available in the location
     * referenced by exit_ptr. When a join() returns successfully,
     * the target thread has been terminated. The results of
     * multiple simultaneous calls to join() specifying the
     * same target thread are undefined. If the thread calling
     * join() is cancelled, then the target thread shall not be
     * detached.
     *
     * pthread_join()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_join.html
     *
     * The join() function may fail if:
     * [EDEADLK] A deadlock was detected.
     *
     * The join() function shall not return an error code of [EINTR].
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Thread::join (void** exit_ptr)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      // TODO optimise it to use events instead of sleep_for().
      for (;;)
        {
          if (sched_state_ == thread::state::terminated)
            {
              break;
            }
          //Systick_clock::sleep_for (1);
          xEventGroupWaitBits (impl_event_flags_, 1, pdTRUE, pdFALSE,
          portMAX_DELAY);
        }

      if (exit_ptr != nullptr)
        {
          *exit_ptr = func_result_;
        }
      trace::printf ("%s() @%p %s joined\n", __func__, this, name ());

      return result::ok;
    }

    /**
     * @details
     * Indicate to the implementation that storage for the thread
     * thread can be reclaimed when that thread terminates. If
     * thread has not terminated, detach() shall not cause it
     * to terminate. The behaviour is undefined if the value
     * specified by the thread argument to detach() does not
     * refer to a joinable thread.
     *
     * pthread_detach()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_detach.html
     *
     * The detach() function shall not return an error code of [EINTR].
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Thread::detach (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      // TODO
      return result::ok;
    }

    /**
     * @details
     * pthread_cancel()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cancel.html
     *
     *
     * The cancel() function shall not return an error code of [EINTR].
     * If an implementation detects use of a thread ID after the end
     * of its lifetime, it is recommended that the function should
     * fail and report an [ESRCH] error.
     * error number is returned.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Thread::cancel (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      // TODO
      return result::ok;
    }

    bool
    Thread::interrupted (void)
    {
      return false;
    }

    /**
     * @details
     * Terminate the calling thread and make the value value_ptr
     * available to any successful join with the terminating thread.
     * Any cancellation cleanup handlers that have been pushed and
     * not yet popped shall be popped in the reverse order that
     * they were pushed and then executed. After all cancellation
     * cleanup handlers have been executed, if the thread has any
     * thread-specific data, appropriate destructor functions
     * shall be called in an unspecified order. Thread termination
     * does not release any application visible process resources,
     * including, but not limited to, mutexes and file descriptors,
     * nor does it perform any process-level cleanup actions,
     * including, but not limited to, calling any atexit() routines
     * that may exist.
     * An implicit call to exit() is made when a thread other
     * than the thread in which main() was first invoked returns
     * from the start routine that was used to create it.
     * The function's return value shall serve as the thread's
     * exit status.
     * The behaviour of exit() is undefined if called from a
     * cancellation cleanup handler or destructor function that
     * was invoked as a result of either an implicit or explicit
     * call to exit().
     * After a thread has terminated, the result of access to
     * local (auto) variables of the thread is undefined.
     * Thus, references to local variables of the exiting
     * thread should not be used for the exit() value_ptr
     * parameter value.
     *
     * pthread_exit()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_exit.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    void
    Thread::exit (void* value_ptr)
    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      func_result_ = value_ptr;
      sched_state_ = thread::state::terminated;

      xEventGroupSetBits (impl_event_flags_, 1);

      impl_ = nullptr;
      vTaskDelete (nullptr);
    }

#if defined(TESTING)
    void
    Thread::__run_function (void)
    {
      assert(func_ != nullptr);
      func_ (func_args_);
    }
#endif

// ======================================================================

    namespace timer
    {
      const Attributes initializer
        { nullptr };
    } /* namespace timer */

    /**
     * @details
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Timer::Timer (timer::func_t function, timer::func_args_t args) :
        Timer
          { timer::initializer, function, args }
    {
      ;
    }

    /**
     * @details
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Timer::Timer (const timer::Attributes& attr, timer::func_t function,
                  timer::func_args_t args) :
        Named_object
          { attr.name () }

    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      assert(function != nullptr);

      type_ = attr.tm_type;
      func_ = function;
      func_args_ = args;

      // TODO
      trace::printf ("%s() @%p \n", __func__, this);
    }

    /**
     * @details
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Timer::~Timer ()
    {
      trace::printf ("%s() @%p \n", __func__, this);
      // TODO
    }

    /**
     * @details
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Timer::start (systicks_t ticks)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s(%d) @%p \n", __func__, ticks, this);
      // TODO
      return result::ok;
    }

    /**
     * @details
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Timer::stop (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);
      // TODO
      return result::ok;
    }

// ======================================================================

    namespace mutex
    {
      const Attributes normal_initializer
        { nullptr };

      const Recursive_attributes recursive_initializer
        { nullptr };
    } /* namespace mutex */

    /**
     * @details
     * Initialise the mutex with default attributes.
     *
     * pthread_mutex_init()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_init.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Mutex::Mutex () :
        Mutex
          { mutex::normal_initializer }
    {
      ;
    }

    /**
     * @details
     * Initialize the mutex with attributes specified by _attr_.
     * Upon successful initialization, the state of the mutex becomes
     * initialized and unlocked.
     *
     * pthread_mutex_init()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_init.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Mutex::Mutex (const mutex::Attributes& attr) :
        Named_object
          { attr.name () }, //
        type_ (attr.mx_type), //
        protocol_ (attr.mx_protocol), //
        robustness_ (attr.mx_robustness) //
    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      prio_ceiling_ = attr.mx_priority_ceiling;
      owner_ = nullptr;
      count_ = 0;

      if (type_ == mutex::type::recursive)
        {
          impl_ = xSemaphoreCreateRecursiveMutex ();
        }
      else
        {
          impl_ = xSemaphoreCreateMutex ();
        }
      trace::printf ("%s() @%p \n", __func__, this);
    }

    /**
     * @details
     * Destroy the mutex object.
     * It shall be safe to destroy an initialized mutex that is
     * unlocked. Attempting to destroy a locked mutex or a mutex
     * that is referenced (for example, while being used in a
     * pthread_cond_timedwait() or pthread_cond_wait()) by another
     * thread results in undefined behavior.

     *
     * pthread_mutex_destroy()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_init.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Mutex::~Mutex ()
    {
      trace::printf ("%s() @%p \n", __func__, this);

      vSemaphoreDelete(impl_);
    }

    /**
     * @details
     * If the mutex is free, lock it. If the mutex is already
     * locked by another thread, the calling thread shall block
     * until the mutex becomes available. This operation shall
     * return with the mutex object referenced by mutex in the
     * locked state with the calling thread as its owner. If a
     * thread attempts to relock a mutex that it has already
     * locked, Mutex::lock() shall behave as described in the
     * **Relock** column of the following table. If a thread
     * attempts to unlock a mutex that it has not locked or a
     * mutex which is unlocked, Mutex::unlock() shall behave as
     * described in the **Unlock When Not Owner** column of the
     * following table.
     *
     * TODO: add table
     *
     * Where the table indicates recursive behavior, the mutex
     * shall maintain the concept of a lock count. When a thread
     * successfully acquires a mutex for the first time, the
     * lock count shall be set to one. Every time a thread
     * relocks this mutex, the lock count shall be incremented
     * by one. Each time the thread unlocks the mutex, the
     * lock count shall be decremented by one. When the lock
     * count reaches zero, the mutex shall become available
     * for other threads to acquire.
     *
     * pthread_mutex_lock()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_lock.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Mutex::lock (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);

      BaseType_t res;

      if (type_ == mutex::type::recursive)
        {
          res = xSemaphoreTakeRecursive(impl_, portMAX_DELAY);
        }
      else
        {
          res = xSemaphoreTake(impl_, portMAX_DELAY);
        }

      if (res != pdTRUE)
        {
          return ENOTRECOVERABLE;
        }
      return result::ok;
    }

    /**
     * @details
     * Try to lock the mutex as Mutex::lock(), except that if the
     * mutex object referenced by mutex is currently locked (by
     * any thread, including the current thread), the call shall
     * return immediately. If the mutex type is PTHREAD_MUTEX_RECURSIVE
     * and the mutex is currently owned by the calling thread,
     * the mutex lock count shall be incremented by one and the
     * Mutex::trylock() function shall immediately return success.
     *
     * If _mutex_ is a robust mutex and the process containing
     * the owning thread terminated while holding the mutex lock,
     * a call to Mutex::lock() shall return the error value
     * [EOWNERDEAD]. If _mutex_ is a robust mutex and the owning
     * thread terminated while holding the mutex lock, a call
     * to Mutex::lock() may return the error value [EOWNERDEAD]
     * even if the process in which the owning thread resides
     * has not terminated. In these cases, the mutex is locked
     * by the thread but the state it protects is marked as
     * inconsistent. The application should ensure that the
     * state is made consistent for reuse and when that is
     * complete call Mutex::consistent(). If the application
     * is unable to recover the state, it should unlock the
     * mutex without a prior call to Mutex::consistent(), after
     * which the mutex is marked permanently unusable.
     *
     * pthread_mutex_trylock()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_lock.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Mutex::try_lock (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);

      BaseType_t res;

      if (type_ == mutex::type::recursive)
        {
          res = xSemaphoreTakeRecursive(impl_, 0);
        }
      else
        {
          res = xSemaphoreTake(impl_, 0);
        }

      if (res != pdTRUE)
        {
          return EAGAIN;
        }
      return result::ok;
    }

    /**
     * @details
     * Try to lock the mutex object referenced by mutex. If the mutex
     * is already locked, the calling thread shall block until the
     * mutex becomes available as in the Mutex::lock() function. If the
     * mutex cannot be locked without waiting for another thread to
     * unlock the mutex, this wait shall be terminated when the specified
     * timeout expires.
     *
     * The timeout shall expire after the number of time units (that
     * is when the value of that clock equals or exceeds (now()+duration).
     * The resolution of the timeout shall be the resolution of the
     * clock on which it is based (the SysTick clock for CMSIS).
     *
     * Under no circumstance shall the function fail with a timeout
     * if the mutex can be locked immediately.
     *
     * As a consequence of the priority inheritance rules (for
     * mutexes initialized with the PRIO_INHERIT protocol),
     * if a timed mutex wait is terminated because its timeout
     * expires, the priority of the owner of the mutex shall be
     * adjusted as necessary to reflect the fact that this thread
     * is no longer among the threads waiting for the mutex.
     *
     * pthread_mutex_timedlock()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_timedlock.html
     *
     * Differences from the standard:
     * - the timeout is not expressed as an absolute time point, but
     * as a relative number of system ticks.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Mutex::timed_lock (systicks_t ticks)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s(%d_ticks) @%p \n", __func__, ticks, this);

      BaseType_t res;

      if (type_ == mutex::type::recursive)
        {
          res = xSemaphoreTakeRecursive(impl_, ticks);
        }
      else
        {
          res = xSemaphoreTake(impl_, ticks);
        }

      if (res != pdTRUE)
        {
          return ETIMEDOUT;
        }
      return result::ok;
    }

    /**
     * @details
     * Release the mutex object referenced by _mutex_. The manner
     * in which a mutex is released is dependent upon the mutex's
     * type attribute. If there are threads blocked on the mutex
     * object referenced by mutex when Mutex::unlock() is called,
     * resulting in the mutex becoming available, the scheduling
     * policy shall determine which thread shall acquire the mutex.
     *
     * (In the case of PTHREAD_MUTEX_RECURSIVE mutexes, the mutex
     * shall become available when the count reaches zero and the
     * calling thread no longer has any locks on this mutex.)
     *
     * pthread_mutex_unlock()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_lock.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Mutex::unlock (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);

      BaseType_t res;

      if (type_ == mutex::type::recursive)
        {
          res = xSemaphoreGiveRecursive(impl_);
        }
      else
        {
          res = xSemaphoreGive(impl_);
        }

      if (res != pdTRUE)
        {
          return ENOTRECOVERABLE;
        }
      return result::ok;
    }

    /**
     * @details
     * Return the current priority ceiling of the mutex.
     *
     * pthread_mutex_getprioceiling()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_getprioceiling.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    thread::priority_t
    Mutex::prio_ceiling (void) const
    {
      assert(!scheduler::in_handler_mode ());

      trace::printf ("%s() @%p \n", __func__, this);

      return prio_ceiling_;
    }

    /**
     * @details
     * Attempt to lock the mutex as if by a call to Mutex::lock(),
     * except that the process of locking the mutex need not adhere
     * to the priority protect protocol. On acquiring the mutex
     * it shall change the mutex's priority ceiling and then
     * release the mutex as if by a call to Mutex::unlock().
     * When the change is successful, the previous value of
     * the priority ceiling shall be returned in old_ceiling.
     *
     * If Mutex::set_prio_ceiling() function fails, the mutex
     * priority ceiling shall not be changed.
     *
     * pthread_mutex_setprioceiling()
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_getprioceiling.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Mutex::prio_ceiling (thread::priority_t prio_ceiling,
                         thread::priority_t* old_prio_ceiling)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);

      thread::priority_t prio;
        {
          // TODO: lock() must not adhere to the priority protocol.
          lock ();
          prio = prio_ceiling_;
          prio_ceiling_ = prio_ceiling;
          unlock ();
        }

      if (old_prio_ceiling != nullptr)
        {
          *old_prio_ceiling = prio;
        }
      return result::ok;
    }

    /**
     * @details
     * If the robust mutex is in an inconsistent state, the
     * Mutex::consistent() function can be used to mark the
     * state protected by the mutex referenced by mutex as
     * consistent again.
     *
     * If an owner of a robust mutex terminates while holding
     * the mutex, the mutex becomes inconsistent and the next
     * thread that acquires the mutex lock shall be notified
     * of the state by the return value [EOWNERDEAD]. In this
     * case, the mutex does not become normally usable again
     * until the state is marked consistent.
     *
     * If the thread which acquired the mutex lock with the
     * return value [EOWNERDEAD] terminates before calling
     * either Mutex::consistent() or Mutex::unlock(), the
     * next thread that acquires the mutex lock shall be
     * notified about the state of the mutex by the return
     * value [EOWNERDEAD].
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Mutex::consistent (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);
      // TODO
      return result::ok;
    }

// ======================================================================

    namespace condvar
    {
      const Attributes initializer
        { nullptr };
    } /* namespace condvar */

    /**
     * @details
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Condition_variable::Condition_variable () :
        Condition_variable
          { condvar::initializer }
    {
      ;
    }

    /**
     * @details
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Condition_variable::Condition_variable (const condvar::Attributes& attr) :
        Named_object
          { attr.name () }
    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);
    }

    /**
     * @details
     * It shall be safe to destroy an initialised condition variable
     * upon which no threads are currently blocked. Attempting to
     * destroy a condition variable upon which other threads are
     * currently blocked results in undefined behaviour.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Condition_variable::~Condition_variable ()
    {
      trace::printf ("%s() @%p \n", __func__, this);
      // TODO
    }

    /**
     * @details
     * Unblock at least one of the threads that are blocked
     * on the specified condition variable.
     *
     * When each thread unblocked as a result of a
     * Condition_variable::signal() returns from its call to
     * Condition_variable::wait() or Condition_variable::timedwait(),
     * the thread shall own the mutex with which it called
     * Condition_variable::wait() or Condition_variable::timed_wait().
     * The thread(s) that are unblocked shall contend for
     * the mutex according to the scheduling policy (if applicable),
     * and as if each had called Mutex::lock().
     *
     * Condition_variable::signal() may be called by a thread
     * whether or not it currently owns the mutex that threads
     * calling Condition_variable::wait() or
     * Condition_variable::timed_wait() have associated with
     * the condition variable during their waits; however,
     * if predictable scheduling behavior is required, then
     * that mutex shall be locked by the thread calling
     * Condition_variable::broadcast().
     *
     * The Condition_variable::signal() function shall
     * have no effect if there are no threads currently
     * blocked on this condition variable.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Condition_variable::signal ()
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);
      // TODO
      return result::ok;
    }

    /**
     * @details
     * Unblock all threads currently blocked on the specified
     * condition variable.
     *
     * If more than one thread is blocked on a condition variable,
     * the scheduling policy shall determine the order in which
     * threads are unblocked.
     *
     * When each thread unblocked as a result of a
     * Condition_variable::broadcast() returns from its call to
     * Condition_variable::wait() or Condition_variable::timed_wait(),
     * the thread shall own the mutex with which it called
     * Condition_variable::wait() or Condition_variable::timed_wait().
     * The thread(s) that are unblocked shall contend for
     * the mutex according to the scheduling policy (if applicable),
     * and as if each had called Mutex::lock().
     *
     * Condition_variable::broadcast() may be called by a thread
     * whether or not it currently owns the mutex that threads
     * calling Condition_variable::wait() or
     * Condition_variable::timed_wait() have associated with
     * the condition variable during their waits; however,
     * if predictable scheduling behaviour is required, then
     * that mutex shall be locked by the thread calling
     * Condition_variable::broadcast().
     *
     * The Condition_variable::broadcast() function shall
     * have no effect if there are no threads currently
     * blocked on this condition variable.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Condition_variable::broadcast ()
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);
      // TODO
      return result::ok;
    }

    /**
     * @details
     * Block on a condition variable. The application shall ensure
     * that this function is called with mutex locked by
     * the calling thread; otherwise, an error (for
     * PTHREAD_MUTEX_ERRORCHECK and robust mutexes) or
     * undefined behaviour (for other mutexes) results.
     *
     * TODO: add more.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Condition_variable::wait (Mutex& mutex)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p \n", __func__, this);
      // TODO
      return result::ok;
    }

    /**
     * @details
     * Block on a condition variable. The application shall ensure
     * that this function is called with mutex locked by
     * the calling thread; otherwise, an error (for
     * ERRORCHECK and robust mutexes) or
     * undefined behaviour (for other mutexes) results.
     *
     * TODO: add more.
     *
     * The timeout shall expire after the number of time units (that
     * is when the value of that clock equals or exceeds (now()+duration).
     * The resolution of the timeout shall be the resolution of the
     * clock on which it is based (the SysTick clock for CMSIS).
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Condition_variable::timed_wait (Mutex& mutex, systicks_t ticks)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s(%d_ticks) @%p \n", __func__, ticks, this);
      // TODO
      return result::ok;
    }

    // ======================================================================

    namespace semaphore
    {
      const Attributes counting_initializer
        { nullptr };

      const Binary_attributes binary_initializer
        { nullptr };

    } /* namespace semaphore */

    /**
     * @details
     * Create and initialise a semaphore with default attributes.
     * Initialise the semaphore with count=0 and no limit for count.
     *
     * Compatible with POSIX `sem_init()`.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_init.html#
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Semaphore::Semaphore () :
        Semaphore
          { semaphore::counting_initializer }
    {
      ;
    }

    /**
     * @details
     * Create and initialise a semaphore with custom attributes.
     *
     * Compatible with POSIX `sem_init()`.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_init.html#
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Semaphore::Semaphore (const semaphore::Attributes& attr) :
        Named_object
          { attr.name () }, //
        initial_count_ (attr.sm_initial_count), //
        max_count_ (attr.sm_max_count)
    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      assert(max_count_ > 0);
      assert(attr.sm_initial_count <= max_count_);

      count_ = attr.sm_initial_count;

      // TODO check if initial count can be negative, to validate.

      impl_ = xSemaphoreCreateCounting(max_count_, initial_count_);

      trace::printf ("%s() @%p %s %d %d\n", __func__, this, name (), count_,
                     max_count_);
    }

    /**
     * @details
     * It is safe to destroy an initialised semaphore upon which
     * no threads are currently blocked. The effect of destroying
     * a semaphore upon which other threads are currently blocked
     * is undefined.
     *
     * Compatible with POSIX `sem_destroy()`.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_destroy.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Semaphore::~Semaphore ()
    {
      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      vSemaphoreDelete(impl_);
    }

    /**
     * @details
     * Performe a post operation on the semaphore, informing
     * the waiting consumers that one more resource is available.
     * The semaphore count is incremented, up to max_count.
     *
     * If the semaphore count resulting from this operation is
     * positive, then no threads were blocked waiting for the
     * semaphore to become unlocked.
     *
     * If the count of the semaphore resulting from this operation
     * is zero, then one of the threads blocked waiting for the
     * semaphore shall be allowed to return successfully from its
     * call to Semaphore::wait().
     *
     * If the Process Scheduling option is supported, the thread
     * to be unblocked shall be chosen in a manner appropriate to
     * the scheduling policies and parameters in effect for the
     * blocked threads. In the case of the schedulers SCHED_FIFO
     * and SCHED_RR, the highest priority waiting thread shall
     * be unblocked, and if there is more than one highest
     * priority thread blocked waiting for the semaphore, then
     * the highest priority thread that has been waiting the
     * longest shall be unblocked. If the Process Scheduling
     * option is not defined, the choice of a thread to unblock
     * is unspecified. If the scheduling policy is SCHED_SPORADIC,
     * the semantics are as per SCHED_FIFO.
     *
     * Compatible with POSIX `sem_post()`
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_post.html
     *
     * @note Can be invoked from Interrupt Service Routines.
     *
     * @warning Applications using these functions may be subject to priority inversion.
     */
    result_t
    Semaphore::post (void)
    {
      trace::printf ("%s() @%p %s\n", __func__, this, name ());

#if 0
      Critical_section_irq cs; // ---- Critical section
      if (count_ > this->max_count_)
        {
          return EOVERFLOW;
        }

      ++count_;

      if (count_ == 0)
        {
          // Wakeup one thread
          if (list_.get_length () > 0)
            {
              list_.get_top ()->wakeup ();
            }
        }
#else
      portBASE_TYPE thread_woken = pdFALSE;

      if (scheduler::in_handler_mode ())
        {
          if (xSemaphoreGiveFromISR (impl_, &thread_woken) != pdTRUE)
            {
              return EOVERFLOW;
            }
          portEND_SWITCHING_ISR(thread_woken);
        }
      else
        {
          if (xSemaphoreGive (impl_) != pdTRUE)
            {
              return EOVERFLOW;
            }
        }
      ++count_;
#endif
      return result::ok;
    }

    /**
     * @details
     * Perform a lock operation on the semaphore.
     *
     * If the current value is positive, it is decremented, and
     * the call returns immediately
     *
     * If the semaphore
     * value is currently zero, then the calling thread shall not
     * return from the call to wait() until it either
     * locks the semaphore or the call is interrupted by a signal.
     *
     * Upon successful return, the state of the semaphore shall
     * be locked and shall remain locked until the post()
     * function is executed and returns successfully.
     *
     * The function is interruptible by the delivery of an external
     * event (signal, thread cancel, etc).
     *
     * Compatible with POSIX `sem_wait()`.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_wait.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     *
     * @warning Applications using these functions may be subject to priority inversion.
     */
    result_t
    Semaphore::wait ()
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p %s\n", __func__, this, name ());
#if 0
        {
          Critical_section_irq cs; // ---- Critical section

          --count_;
          if (count_ >= 0)
            {
              return result::ok;
            }

          // Add current thread to the semaphore waiting  list
          list_.add (&this_thread::thread ());
          // `count_` is negative.
        }
      this_thread::yield ();

      // Return EINTR?
#else
      if (xSemaphoreTake (impl_, portMAX_DELAY) != pdTRUE)
        {
          return ENOTRECOVERABLE;
        }
      --count_;
#endif
      return result::ok;
    }

    /**
     * @details
     * Tries to perform a lock operation only if the semaphore is currently
     * not locked; that is, if the semaphore value is currently
     * positive it decrements it and returns success.
     * Otherwise, it shall not lock the semaphore.
     *
     * Upon successful return, the state of the semaphore shall
     * be locked and shall remain locked until the post()
     * function is executed and returns successfully.
     *
     * Compatible with POSIX `sem_trywait()`.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_wait.html
     *
     * @note Can be invoked from Interrupt Service Routines.
     *
     * @warning Applications using these functions may be subject to priority inversion.
     */
    result_t
    Semaphore::try_wait ()
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p %s\n", __func__, this, name ());

#if 0
        {
          Critical_section_irq cs; // ---- Critical section

          if (count_ > 0)
            {
              --count_;
              return result::ok;
            }

          // Count may be 0 or negative
          return EAGAIN;
        }
#else
      portBASE_TYPE thread_woken = pdFALSE;

      if (scheduler::in_handler_mode ())
        {
          if (xSemaphoreTakeFromISR (impl_, &thread_woken) != pdTRUE)
            {
              return EAGAIN;
            }
          portEND_SWITCHING_ISR(thread_woken);
        }
      else if (xSemaphoreTake (impl_, 0) != pdTRUE)
        {
          return EAGAIN;
        }

      --count_;
      return result::ok;
#endif
    }

    /**
     * @details
     * Try to perform a lock operation; if the semaphore cannot be
     * locked without waiting for another process or thread to
     * unlock the semaphore by performing a Semaphore::post()
     * function, this wait shall be terminated when the specified
     * timeout expires.
     *
     * The timeout shall expire after the number of time units (that
     * is when the value of that clock equals or exceeds (now()+duration).
     * The resolution of the timeout shall be the resolution of the
     * clock on which it is based (the SysTick clock for CMSIS).
     *
     * Under no circumstance shall the function fail with a timeout
     * if the semaphore can be locked immediately. The validity of
     * the timeout need not be checked if the semaphore can be
     * locked immediately.
     *
     * Compatible with POSIX `sem_timedwait()`, except the time point
     * is replaced with a duration.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_timedwait.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     *
     * @warning Applications using these functions may be subject to priority inversion.
     */
    result_t
    Semaphore::timed_wait (systicks_t ticks)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s(%d_ticks) @%p %s\n", __func__, ticks, this, name ());

#if 0
        {
          Critical_section_irq cs; // ---- Critical section

          --count_;
          if (count_ >= 0)
            {
              return result::ok;
            }

          // Add current thread to the semaphore waiting list.
          list_.add (&this_thread::thread ());
        }
      Systick_clock::sleep_for (ticks);

      // TODO: return EINTR
      return ETIMEDOUT;
#else
      if (xSemaphoreTake (impl_, ticks > 0 ? ticks : 1) != pdTRUE)
        {
          return ETIMEDOUT;
        }

      --count_;
      return result::ok;
#endif
    }

    /**
     * @details
     * Set the counter to the initial value.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Semaphore::reset (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      Critical_section_irq cs; // ---- Critical section

      if (count_ < 0)
        {
          // There are waiting tasks
          return EAGAIN;
        }

      // TODO: reset waiting list

      count_ = initial_count_;
      return result::ok;
    }

    // ======================================================================

    namespace mempool
    {
      const Attributes initializer
        { nullptr };

    } /* namespace semaphore */

    /**
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Memory_pool::Memory_pool (mempool::size_t blocks,
                              mempool::size_t block_size_bytes) :
        Memory_pool
          { mempool::initializer, blocks, block_size_bytes }
    {
      ;
    }

    /**
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Memory_pool::Memory_pool (const mempool::Attributes& attr,
                              mempool::size_t blocks,
                              mempool::size_t block_size_bytes) :
        Named_object
          { attr.name () }, //
        blocks_ (blocks), //
        block_size_bytes_ (block_size_bytes)

    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      pool_addr_ = (char*) attr.mp_pool_address;

      assert(blocks_ > 0);
      // Blocks must be large enough to store the index, used
      // to construct the list of free blocks.
      assert(block_size_bytes_ >= sizeof(std::ptrdiff_t));

      flags_ = 0;

      trace::printf ("%s() @%p %s %d %d\n", __func__, this, name (), blocks_,
                     block_size_bytes_);

      if (pool_addr_ == nullptr)
        {
          pool_addr_ = new (std::nothrow) char[blocks_ * block_size_bytes_];
          flags_ |= flags_allocated;
        }

      os_assert_throw(pool_addr_ != nullptr, ENOMEM);

      // Construct a linked list of blocks. Store the pointer at
      // the beginning of each block. Each block
      // will hold the address of the next free block, or nullptr at the end.
      char* p = pool_addr_;
      for (std::size_t i = 1; i < blocks_; ++i)
        {
          *(void**) p = (p + block_size_bytes_);
          p += block_size_bytes_;
        }
      *(void**) p = nullptr;

      first_ = pool_addr_; // Pointer to first block.
      last_ = p; // Pointer to last block.

      count_ = 0; // No allocated blocks.
    }

    /**
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Memory_pool::~Memory_pool ()
    {
      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      if (flags_ | flags_allocated)
        {
          delete[] (pool_addr_);
        }
    }

    /*
     * Internal function used to return the first block in the
     * free list.
     */
    void*
    Memory_pool::try_first (void)
    {
      if (first_ != nullptr)
        {
          void* p = (void*) first_;
          first_ = *(void**) first_;
          ++count_;
          return p;
        }
      return nullptr;
    }

    /**
     * @details
     * Allocate a fixed size memory block from the memory pool.
     *
     * It uses a critical section to protect simultaneous access from
     * other threads or interrupts.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    void*
    Memory_pool::alloc (void)
    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      bool queued = false;
      for (;;)
        {
            {
              Critical_section_irq cs; // ---- Critical section

              void* p = try_first ();
              if (p != nullptr)
                {
                  return p;
                }

              if (!queued)
                {
                  // Add this thread to the waiting list.
                  // Will be removed by free().
                  list_.add (&this_thread::thread ());
                  queued = true;
                }
            }
          this_thread::suspend ();

          if (this_thread::thread ().interrupted ())
            {
              return nullptr;
            }
        }

      /* NOTREACHED */
      return nullptr;
    }

    /**
     * @details
     * Allocate a fixed size memory block from the memory pool.
     *
     * It uses a critical section to protect simultaneous access from
     * other threads or interrupts.
     *
     * @note Can be invoked from Interrupt Service Routines.
     */
    void*
    Memory_pool::try_alloc (void)
    {
      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      return try_first ();
    }

    /**
     * @details
     * Allocate a fixed size memory block from the memory pool.
     *
     * It uses a critical section to protect simultaneous access from
     * other threads or interrupts.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    void*
    Memory_pool::timed_alloc (systicks_t ticks)
    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s(%d) @%p %s\n", __func__, ticks, this, name ());

      bool queued = false;

      Systick_clock::rep start = Systick_clock::now ();
      for (;;)
        {
          Systick_clock::sleep_rep slept_ticks;
            {
              Critical_section_irq cs; // ---- Critical section

              void* p = try_first ();
              if (p != nullptr)
                {
                  return p;
                }

              Systick_clock::rep now = Systick_clock::now ();
              slept_ticks = (Systick_clock::sleep_rep) (now - start);
              if (slept_ticks >= ticks)
                {
                  if (queued)
                    {
                      list_.remove (&this_thread::thread ());
                    }
                  return nullptr;
                }

              if (!queued)
                {
                  // Add this thread to the waiting list.
                  // Will be removed by free() or if timeout occurs.
                  list_.add (&this_thread::thread ());
                  queued = true;
                }
            }

          Systick_clock::sleep_for (ticks - slept_ticks);

          if (this_thread::thread ().interrupted ())
            {
              return nullptr;
            }
        }

      /* NOTREACHED */
      return nullptr;
    }

    /**
     * @details
     * Return a memory block previously allocated by `alloc()` or
     * `calloc()` back to the memory pool.
     *
     * It uses a critical section to protect simultaneous access from
     * other threads or interrupts.
     *
     * @note Can be invoked from Interrupt Service Routines.
     */
    result_t
    Memory_pool::free (void* block)
    {
      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      // Validate pointer.
      if ((block < pool_addr_)
          || (block >= (pool_addr_ + blocks_ * block_size_bytes_)))
        {
          return EINVAL;
        }

      Critical_section_irq cs; // ---- Critical section

      // Perform a push_back() on the single linked FIFO list,
      // i.e. add the block to the end of the list.
      if (last_ != nullptr)
        {
          // Link the block after the current last.
          *(void**) last_ = block;
        }

      // Mark block as end of list.
      *(void**) block = nullptr;

      // Now this block is the last one.
      last_ = block;

      // If the list was empty (all blocks were allocated, so no first),
      // this block will be both the first and the last.
      if (first_ == nullptr)
        {
          first_ = block;
        }

      --count_;

      if (!list_.empty ())
        {
          // Wake-up one thread, if any.
          list_.get_top ()->wakeup ();
        }

      return result::ok;
    }

    /**
     * @details
     * Reset the memory pool to initial state, all block are free.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Memory_pool::reset (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      // TODO
      return result::ok;
    }

    // ======================================================================

    namespace mqueue
    {
      const Attributes initializer
        { nullptr };

    } /* namespace mqueue */

    /**
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Message_queue::Message_queue (mqueue::size_t msgs,
                                  mqueue::size_t msg_size_bytes) :
        Message_queue (mqueue::initializer, msgs, msg_size_bytes)
    {
      ;
    }

    /**
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    Message_queue::Message_queue (const mqueue::Attributes&attr,
                                  mqueue::size_t msgs,
                                  mqueue::size_t msg_size_bytes) :
        Named_object
          { attr.name () }, //
        msgs_ (msgs), //
        msg_size_bytes_ (msg_size_bytes)
    {
      os_assert_throw(!scheduler::in_handler_mode (), EPERM);

      queue_addr_ = attr.queue_address;
      queue_size_bytes_ = attr.queue_size_bytes;
      if (queue_addr_ != nullptr)
        {
          assert(queue_size_bytes_ > 0);
          assert(queue_size_bytes_ >= (std::size_t )(msgs * msg_size_bytes));
        }
      else
        {
          // TODO: dynamically alloc queue (msgs * msg_size_bytes).
        }

      count_ = 0;

      impl_ = xQueueCreate(msgs_, queue_size_bytes_);

      trace::printf ("%s() @%p %s %d %d\n", __func__, this, name (), msgs_,
                     msg_size_bytes_);
    }

    /**
     *
     */
    Message_queue::~Message_queue ()
    {
      trace::printf ("%s() @%p %s\n", __func__, this, name ());

      // TODO: Free dynamically allocated structures.

      vQueueDelete (impl_);
    }

    /**
     * @details
     * The send() function shall add the message pointed to by the argument
     * msg to the message queue. The nbytes argument specifies the length
     * of the message, in bytes, pointed to by msg. The value of nbytes
     * shall be less than or equal to the msg_size_bytes parameter of the
     * message queue object, or send() shall fail.
     *
     * If the specified message queue is not full, send() shall behave
     * as if the message is inserted into the message queue at the
     * position indicated by the mprio argument. A message with a
     * larger numeric value of mprio shall be inserted before messages
     * with lower values of mprio. A message shall be inserted after
     * other messages in the queue, if any, with equal mprio. The
     * value of mprio shall be less than {MQ_PRIO_MAX}.
     *
     * If the specified message queue is full, send() shall block
     * until space becomes available to enqueue the message, or
     * until send() is cancelled/interrupted. If more than one
     * thread is waiting to send when space becomes available
     * in the message queue and the Priority Scheduling option is
     * supported, then the thread of the highest priority that has
     * been waiting the longest shall be unblocked to send its
     * message. Otherwise, it is unspecified which waiting thread
     * is unblocked.
     *
     * Compatible with POSIX `mq_send()` with O_NONBLOCK not set.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_timedsend.html
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Message_queue::send (const char* msg, std::size_t nbytes,
                         mqueue::priority_t mprio)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);
      os_assert_err(msg != nullptr, EINVAL);
      os_assert_err(nbytes > msg_size_bytes_, EINVAL);

      // FreeRTOS will store the full message, regardless of the nbytes.
      BaseType_t res = xQueueSend(impl_, msg, portMAX_DELAY);

      if (res != pdTRUE)
        {
          return ENOTRECOVERABLE;
        }

      ++count_;
      return result::ok;
    }

    /**
     * @details
     * The send() function shall add the message pointed to by the argument
     * msg to the message queue. The nbytes argument specifies the length
     * of the message, in bytes, pointed to by msg. The value of nbytes
     * shall be less than or equal to the msg_size_bytes parameter of the
     * message queue object, or send() shall fail.
     *
     * If the message queue is not full, send() shall behave
     * as if the message is inserted into the message queue at the
     * position indicated by the mprio argument. A message with a
     * larger numeric value of mprio shall be inserted before messages
     * with lower values of mprio. A message shall be inserted after
     * other messages in the queue, if any, with equal mprio. The
     * value of mprio shall be less than {MQ_PRIO_MAX}.
     *
     * If the message queue is full, the message shall
     * not be queued and try_send() shall return an error.
     *
     * Compatible with POSIX `mq_send()` with O_NONBLOCK set.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_timedsend.html
     *
     * @note Can be invoked from Interrupt Service Routines.
     */
    result_t
    Message_queue::try_send (const char* msg, std::size_t nbytes,
                             mqueue::priority_t mprio)
    {
      os_assert_err(msg != nullptr, EINVAL);
      os_assert_err(nbytes > msg_size_bytes_, EINVAL);

      portBASE_TYPE thread_woken = pdFALSE;

      if (scheduler::in_handler_mode ())
        {
          if (xQueueSendFromISR (impl_, msg, &thread_woken) != pdTRUE)
            {
              return EAGAIN;
            }
          portEND_SWITCHING_ISR(thread_woken);
        }
      else
        {
          if (xQueueSend (impl_, msg, 0) != pdTRUE)
            {
              return EAGAIN;
            }
        }

      ++count_;
      return result::ok;
    }

    /**
     * @details
     * The send() function shall add the message pointed to by the argument
     * msg to the message queue. The nbytes argument specifies the length
     * of the message, in bytes, pointed to by msg. The value of nbytes
     * shall be less than or equal to the msg_size_bytes attribute of the
     * message queue object, or send() shall fail.
     *
     * If the message queue is not full, send() shall behave
     * as if the message is inserted into the message queue at the
     * position indicated by the mprio argument. A message with a
     * larger numeric value of mprio shall be inserted before messages
     * with lower values of mprio. A message shall be inserted after
     * other messages in the queue, if any, with equal mprio. The
     * value of mprio shall be less than {MQ_PRIO_MAX}.
     *
     * If the message queue is full, the wait for sufficient
     * room in the queue shall be terminated when the specified timeout
     * expires.
     *
     * The timeout shall expire after the number of time units (that
     * is when the value of that clock equals or exceeds (now()+duration).
     * The resolution of the timeout shall be the resolution of the
     * clock on which it is based (the SysTick clock for CMSIS).
     *
     * Under no circumstance shall the operation fail with a timeout
     * if there is sufficient room in the queue to add the message
     * immediately.
     *
     * Compatible with POSIX `mq_timedsend()`.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_timedsend.html
     *
     * Differences from the standard:
     * - the timeout is not expressed as an absolute time point, but
     * as a relative number of system ticks.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Message_queue::timed_send (const char* msg, std::size_t nbytes,
                               mqueue::priority_t mprio, systicks_t ticks)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);
      os_assert_err(msg != nullptr, EINVAL);
      os_assert_err(nbytes > msg_size_bytes_, EINVAL);

      // FreeRTOS will store the full message, regardless of the nbytes.
      BaseType_t res = xQueueSend(impl_, msg, ticks > 0 ? ticks : 1);

      if (res != pdTRUE)
        {
          return ETIMEDOUT;
        }

      ++count_;
      return result::ok;
    }

    /**
     * @details
     * The receive() function shall receive the oldest of the highest
     * priority message(s) from the message queue. If the size of the
     * buffer in bytes, specified by the nbytes argument, is less than
     * the msg_size_bytes attribute of the message queue, the function
     * shall fail and return an error. Otherwise, the selected message
     * shall be removed from the queue and copied to the buffer pointed
     * to by the msg argument.
     *
     * If the value of nbytes is greater than {SSIZE_MAX}, the result
     * is implementation-defined.
     *
     * If the argument mprio is not NULL, the priority of the selected
     * message shall be stored in the location referenced by mprio.
     *
     * If the message queue is empty, receive() shall block
     * until a message is enqueued on the message queue or until
     * receive() is cancelled/interrupted. If more than one thread
     * is waiting to receive a message when a message arrives at
     * an empty queue and the Priority Scheduling option is supported,
     * then the thread of highest priority that has been waiting the
     * longest shall be selected to receive the message. Otherwise,
     * it is unspecified which waiting thread receives the message.
     *
     * Compatible with POSIX `mq_receive()` with O_NONBLOCK not set.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_receive.html#
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Message_queue::receive (char* msg, std::size_t nbytes,
                            mqueue::priority_t* mprio)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);
      os_assert_err(msg != nullptr, EINVAL);
      os_assert_err(nbytes > msg_size_bytes_, EMSGSIZE);

      BaseType_t res = xQueueReceive(impl_, msg, portMAX_DELAY);
      if (res != pdTRUE)
        {
          return ENOTRECOVERABLE;
        }

      --count_;
      return result::ok;
    }

    /**
     * @details
     * The receive() function shall receive the oldest of the highest
     * priority message(s) from the message queue. If the size of the
     * buffer in bytes, specified by the nbytes argument, is less than
     * the msg_size_bytes attribute of the message queue, the function
     * shall fail and return an error. Otherwise, the selected message
     * shall be removed from the queue and copied to the buffer pointed
     * to by the msg argument.
     *
     * If the value of nbytes is greater than {SSIZE_MAX}, the result
     * is implementation-defined.
     *
     * If the argument mprio is not NULL, the priority of the selected
     * message shall be stored in the location referenced by mprio.
     *
     * If the message queue is empty, no message shall be removed
     * from the queue, and receive() shall return an error.
     *
     * Compatible with POSIX `mq_receive()` with O_NONBLOCK set.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_receive.html#
     *
     * @note Can be invoked from Interrupt Service Routines.
     */
    result_t
    Message_queue::try_receive (char* msg, std::size_t nbytes,
                                mqueue::priority_t* mprio)
    {
      os_assert_err(msg != nullptr, EINVAL);
      os_assert_err(nbytes > msg_size_bytes_, EMSGSIZE);

      return EAGAIN;

      portBASE_TYPE thread_woken = pdFALSE;

      if (scheduler::in_handler_mode ())
        {
          if (xQueueReceiveFromISR (impl_, msg, &thread_woken) != pdTRUE)
            {
              return EAGAIN;
            }
          portEND_SWITCHING_ISR(thread_woken);
        }
      else
        {
          if (xQueueReceive (impl_, msg, 0) != pdTRUE)
            {
              return EAGAIN;
            }
        }

      --count_;
      return result::ok;
    }

    /**
     * @details
     * The receive() function shall receive the oldest of the highest
     * priority message(s) from the message queue. If the size of the
     * buffer in bytes, specified by the nbytes argument, is less than
     * the msg_size_bytes attribute of the message queue, the function
     * shall fail and return an error. Otherwise, the selected message
     * shall be removed from the queue and copied to the buffer pointed
     * to by the msg argument.
     *
     * If the value of nbytes is greater than {SSIZE_MAX}, the result
     * is implementation-defined.
     *
     * If the argument mprio is not NULL, the priority of the selected
     * message shall be stored in the location referenced by mprio.
     *
     * If the message queue is empty, receive() shall block
     * until a message is enqueued on the message queue or until
     * receive() is cancelled/interrupted. If more than one thread
     * is waiting to receive a message when a message arrives at
     * an empty queue and the Priority Scheduling option is supported,
     * then the thread of highest priority that has been waiting the
     * longest shall be selected to receive the message. Otherwise,
     * it is unspecified which waiting thread receives the message.
     *
     * The timed_receive() function shall receive the oldest of
     * the highest priority messages from the message queue as described
     * for the receive() function. However, if no message exists on the
     * queue to satisfy the receive, the wait for such a message shall
     * be terminated when the specified timeout expires.
     *
     * The timeout shall expire after the number of time units (that
     * is when the value of that clock equals or exceeds (now()+duration).
     * The resolution of the timeout shall be the resolution of the
     * clock on which it is based (the SysTick clock for CMSIS).
     *
     * Under no circumstance shall the operation fail with a timeout
     * if a message can be removed from the message queue immediately.
     * The validity of the abstime parameter need not be checked if a
     * message can be removed from the message queue immediately.
     *
     * Compatible with POSIX `mq_receive()` with O_NONBLOCK set.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_receive.html#
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Message_queue::timed_receive (char* msg, std::size_t nbytes,
                                  mqueue::priority_t* mprio, systicks_t ticks)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);
      os_assert_err(msg != nullptr, EINVAL);
      os_assert_err(nbytes > msg_size_bytes_, EMSGSIZE);

      BaseType_t res = xQueueReceive(impl_, msg, ticks > 0 ? ticks : 1);
      if (res != pdTRUE)
        {
          return ETIMEDOUT;
        }

      --count_;
      return result::ok;
    }

    /**
     * @details
     * Clear both send and receive counter and return the queue to the
     * initial state.
     *
     * @warning Cannot be invoked from Interrupt Service Routines.
     */
    result_t
    Message_queue::reset (void)
    {
      os_assert_err(!scheduler::in_handler_mode (), EPERM);

      xQueueReset(impl_);

      count_ = 0;
      return result::ok;
    }

    namespace impl
    {
      // ====================================================================

      Prioritised_list::Prioritised_list ()
      {
        for (std::size_t i = 0; i < sizeof(array_) / sizeof(array_[0]); ++i)
          {
            array_[i] = nullptr;
          }
        count_ = 0;
      }

      Prioritised_list::~Prioritised_list ()
      {
        ;
      }

      void
      Prioritised_list::add (Thread* thread)
      {
        assert(thread != nullptr);
        assert(count_ < (sizeof(array_) / sizeof(array_[0]) + 1));

        array_[count_++] = thread;
      }

      void
      Prioritised_list::remove (Thread* thread)
      {
        // TODO
      }

      Thread*
      Prioritised_list::get_top (void)
      {
        Thread* thread = nullptr;
        thread::priority_t prio = thread::priority::none;
        std::size_t pos = 0;

        for (std::size_t i = 0; i < sizeof(array_) / sizeof(array_[0]); ++i)
          {
            if (array_[i]->sched_prio () > prio)
              {
                prio = array_[i]->sched_prio ();
                thread = array_[i];
                pos = i;
              }
          }

        for (std::size_t i = pos; i < (sizeof(array_) / sizeof(array_[0]) - 2);
            ++i)
          {
            array_[i] = array_[i + 1];
          }
        count_--;
        array_[count_] = nullptr;

        return thread;
      }

    } /* namespace impl */

  // ========================================================================

#pragma GCC diagnostic pop

  } /* namespace rtos */
} /* namespace os */
