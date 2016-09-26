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
 * Implementation routines for the CMSIS++ reference scheduler, mainly
 * the context switching and context creation.
 */

#if defined(__ARM_EABI__)

// ----------------------------------------------------------------------------

#include <string.h>
#include <cassert>

#include <cmsis-plus/rtos/os.h>

// ----------------------------------------------------------------------------

namespace os
{
  namespace rtos
  {
    namespace port
    {
      // ----------------------------------------------------------------------

      namespace scheduler
      {
        // --------------------------------------------------------------------

        extern "C" unsigned long uxSchedulerSuspended;

        state_t
        locked (state_t state)
        {
          BaseType_t st = xTaskGetSchedulerState ();
          if (st == taskSCHEDULER_NOT_STARTED)
            {
              return false;
            }

          state_t tmp = (st == taskSCHEDULER_SUSPENDED);

          // trace::printf("%s(%d)\n", __func__, state);
          if (state == tmp)
            {
              return tmp;
            }

          if (state)
            {
              // trace::printf("vTaskSuspendAll()\n");
              vTaskSuspendAll ();
            }
          else
            {
              // trace::printf("xTaskResumeAll()\n");
              xTaskResumeAll ();
            }
          // trace::printf("%s(%d)=%d f%d %d\n", __func__, state, tmp, xTaskGetSchedulerState(), uxSchedulerSuspended);

          return tmp;
        }

      // ----------------------------------------------------------------------

      } /* namespace scheduler */
    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

#endif /* defined(__ARM_EABI__) */
