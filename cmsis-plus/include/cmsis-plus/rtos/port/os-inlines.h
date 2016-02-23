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
 */

#ifndef CMSIS_PLUS_RTOS_OS_IMPL_INLINES_H_
#define CMSIS_PLUS_RTOS_OS_IMPL_INLINES_H_

// ----------------------------------------------------------------------------

#include <cmsis-plus/rtos/os-app-config.h>

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
    namespace impl
    {
      // ----------------------------------------------------------------------

      namespace scheduler
      {
        inline result_t
        initialize (void)
        {
          return result::ok;
        }

        inline result_t
        start (void)
        {
          vTaskStartScheduler ();
          return result::ok;
        }

        inline void
        lock (void)
        {
          vTaskSuspendAll ();
        }

        inline void
        unlock (rtos::scheduler::status_t status __attribute__((unused)))
        {
          xTaskResumeAll ();
        }

      } /* namespace scheduler */

    // ----------------------------------------------------------------------

    } /* namespace impl */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* CMSIS_PLUS_RTOS_OS_IMPL_INLINES_H_ */
