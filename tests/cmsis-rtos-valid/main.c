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
