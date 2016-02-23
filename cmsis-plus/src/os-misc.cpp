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
    // ------------------------------------------------------------------------

#pragma GCC diagnostic push
// TODO: remove it when fully implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

    // ========================================================================

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

    // ========================================================================

    Named_object::Named_object (const char* name) :
        name_ (name != nullptr ? name : "-")
    {
      ;
    }

  // --------------------------------------------------------------------------

#pragma GCC diagnostic pop

  } /* namespace rtos */
} /* namespace os */
