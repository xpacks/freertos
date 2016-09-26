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

#include <cmsis-plus/os-app-config.h>
#include <cmsis-plus/rtos/port/os-c-decls.h>

// ----------------------------------------------------------------------------

#if !defined(OS_INTEGER_SYSTICK_FREQUENCY_HZ)
#define OS_INTEGER_SYSTICK_FREQUENCY_HZ (1000)
#endif

#if !defined(OS_INTEGER_RTOS_MIN_STACK_SIZE_BYTES)
#define OS_INTEGER_RTOS_MIN_STACK_SIZE_BYTES (256)
#endif

#if !defined(OS_INTEGER_RTOS_DEFAULT_STACK_SIZE_BYTES)
#define OS_INTEGER_RTOS_DEFAULT_STACK_SIZE_BYTES (2048)
#endif

#if !defined(OS_INTEGER_RTOS_MAIN_STACK_SIZE_BYTES)
#define OS_INTEGER_RTOS_MAIN_STACK_SIZE_BYTES (OS_INTEGER_RTOS_DEFAULT_STACK_SIZE_BYTES)
#endif

#if !defined(OS_INTEGER_RTOS_IDLE_STACK_SIZE_BYTES)
#define OS_INTEGER_RTOS_IDLE_STACK_SIZE_BYTES (OS_INTEGER_RTOS_DEFAULT_STACK_SIZE_BYTES)
#endif

#ifdef  __cplusplus

#include <cstdint>
#include <cstddef>

namespace os
{
  namespace rtos
  {
    namespace port
    {
      // ----------------------------------------------------------------------

      namespace stack
      {
        // Stack word.
        using element_t = os_port_thread_stack_element_t;

        // Align stack to 8 bytes.
        using allocation_element_t = os_port_thread_stack_allocation_element_t;

        // Currently not used by FreeRTOS.
        // Initial value for the minimum stack size in bytes.
        constexpr std::size_t min_size_bytes =
        OS_INTEGER_RTOS_MIN_STACK_SIZE_BYTES;

        // Initial value for the default stack size in bytes.
        constexpr std::size_t default_size_bytes =
        OS_INTEGER_RTOS_DEFAULT_STACK_SIZE_BYTES;

        // Used to fill in the stack.
        constexpr element_t magic = OS_INTEGER_RTOS_STACK_FILL_MAGIC; // DEADBEEF

      } /* namespace stack */

      // ----------------------------------------------------------------------

      namespace interrupts
      {
        // Type to store the entire processor interrupts mask.
        using state_t = os_port_irq_state_t;

        namespace state
        {
          constexpr state_t init = 0;
        } /* namespace state */

      } /* namespace interrupts */

      namespace scheduler
      {
        using state_t = os_port_scheduler_state_t;

        namespace state
        {
          constexpr state_t locked = true;
          constexpr state_t unlocked = false;
          constexpr state_t init = unlocked;
        } /* namespace state */

        extern state_t lock_state;

      } /* namespace scheduler */

      namespace statistics
      {
        using counter_t = uint64_t;

        using duration_t = uint64_t;

      } /* namespace statistics */

    // ----------------------------------------------------------------------

    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* CMSIS_PLUS_RTOS_PORT_OS_DECLS_H_ */
