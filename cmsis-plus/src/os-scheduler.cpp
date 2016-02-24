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

#include <cmsis-plus/rtos/port/os-decls.h>
#include <cmsis-plus/rtos/port/os-inlines.h>

// ----------------------------------------------------------------------------

#include <cassert>
#include <cerrno>
#include <cstdlib>

// ----------------------------------------------------------------------------

namespace os
{
  namespace rtos
  {

#pragma GCC diagnostic push
// TODO: remove it when fully implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

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

    // ========================================================================

    namespace port
    {

      Tasks_list::Tasks_list ()
      {
        for (std::size_t i = 0; i < sizeof(array_) / sizeof(array_[0]); ++i)
          {
            array_[i] = nullptr;
          }
        count_ = 0;
      }

      Tasks_list::~Tasks_list ()
      {
        ;
      }

      void
      Tasks_list::add (Thread* thread)
      {
        assert(thread != nullptr);
        assert(count_ < (sizeof(array_) / sizeof(array_[0]) + 1));

        array_[count_++] = thread;
      }

      void
      Tasks_list::remove (std::size_t pos)
      {
        for (std::size_t i = pos; i < (sizeof(array_) / sizeof(array_[0]) - 2);
            ++i)
          {
            array_[i] = array_[i + 1];
          }
        count_--;
        array_[count_] = nullptr;
      }

      void
      Tasks_list::remove (Thread* thread)
      {

        for (std::size_t i = 0; i < sizeof(array_) / sizeof(array_[0]); ++i)
          {
            if (array_[i] == thread)
              {
                remove (i);
                return;
              }
          }
      }

#if 1
      Thread*
      Tasks_list::_top_prio_task (void)
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

        remove (pos);

        return thread;
      }
#endif

      void
      Tasks_list::wakeup_one ()
      {
        _top_prio_task ()->wakeup ();
      }

      void
      Tasks_list::wakeup_all ()
      {
        for (std::size_t i = 0; i < sizeof(array_) / sizeof(array_[0]); ++i)
          {
            array_[i]->wakeup ();
          }
        count_ = 0;
      }

    } /* namespace port */

  // ========================================================================

#pragma GCC diagnostic pop

  } /* namespace rtos */
} /* namespace os */
