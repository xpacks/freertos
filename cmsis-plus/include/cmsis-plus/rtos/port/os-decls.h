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
 * It is included in `cmsis-plus/rtos/os.h` to customise
 * it with port specific declarations.
 */

#ifndef CMSIS_PLUS_RTOS_PORT_OS_DECLS_H_
#define CMSIS_PLUS_RTOS_PORT_OS_DECLS_H_

// ----------------------------------------------------------------------------

#include <cmsis-plus/rtos/os-app-config.h>
#include <cmsis-plus/rtos/port/os-c-decls.h>

// ----------------------------------------------------------------------------

#ifdef  __cplusplus

#include <cstdint>
#include <cstddef>

namespace os
{
  namespace rtos
  {
    class Thread;
    class Waiting_threads_list;
    class Clock_threads_list;
    class Waiting_threads_list3;

    namespace port
    {
      class Thread;
      class Timer;
      class Mutex;
      class Condition_variable;
      class Semaphore;
      class Memory_pool;
      class Message_queue;
      class Event_flags;

      // ----------------------------------------------------------------------

      namespace stack
      {
        // Align stack to 8 bytes.
        using element_t = long long;
      } /* namespace stack */

      using Waiting_threads_list = rtos::Waiting_threads_list;
      using Clock_threads_list = rtos::Clock_threads_list;

    // ----------------------------------------------------------------------

    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

namespace os
{
  namespace rtos
  {
    namespace port
    {
    // ----------------------------------------------------------------------

    // ----------------------------------------------------------------------

    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* CMSIS_PLUS_RTOS_PORT_OS_DECLS_H_ */
