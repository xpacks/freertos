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

#include <stm32f4xx_hal.h>
#include <cmsis-plus/cortexm/exception-handlers.h>

#include "portmacro.h"

extern void
xPortSysTickHandler (void);

extern int
is_scheduler_running(void);

// This is a sample SysTick handler, use it if you need HAL timings.
void __attribute__ ((section(".after_vectors")))
SysTick_Handler (void)
{
  if (is_scheduler_running())
    {
      xPortSysTickHandler ();
    }

#if defined(USE_HAL_DRIVER)
  HAL_IncTick ();
#endif
}
