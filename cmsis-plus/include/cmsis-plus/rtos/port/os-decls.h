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
