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

#include "cmsis_rv.h"

#include <cmsis-plus/rtos/os-c-api.h>
#include <cmsis-plus/diag/trace.h>

int
os_main (int argc, char* argv[])
{
  trace_dump_args (argc, argv);

  return cmsis_rv ();
}

extern void
(*TST_IRQHandler) (void);

void
WWDG_IRQHandler (void);

void
WWDG_IRQHandler (void)
{
  if (TST_IRQHandler != NULL)
    {
      TST_IRQHandler ();
      return;
    }
  asm volatile ("bkpt 0");
  for (;;)
    ;
}

void
vApplicationStackOverflowHook (void* task, const char* name);

void
vApplicationStackOverflowHook (void* task, const char* name)
{
  printf (">>> Stack overflow %p '%s'!\n", task, name);
}
