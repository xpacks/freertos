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

#include <stdio.h>
#include <cstdlib>

#include "FreeRTOS.h"
#include "task.h"

using namespace os;
using namespace os::rtos;

void*
task (void* args);

// -----------

void*
task (void* args __attribute__((unused)))
{
  int count = 0;
  for (; count < 3;)
    {
      ++count;
      Thread& t = this_thread::thread ();
      trace::printf ("%s %d\n", t.name (), count);

      Systick_clock::sleep_for (configTICK_RATE_HZ);
    }
  return nullptr;
}

int
os_main (int argc, char* argv[])
{
  trace::dump_args (argc, argv);

  trace::printf ("Test CMSIS++ RTOS based on FreeRTOS.\n");

  Thread th1
    { (thread::func_t) task, (thread::func_args_t) nullptr };

  thread::Attributes attr1
    { "th2" };
  Thread th2
    { attr1, (thread::func_t) task, (thread::func_args_t) nullptr };

  th1.join ();
  th2.join ();

  return 1;
}

#if 0
int
__main (int argc __attribute__((unused)), char* argv[] __attribute__((unused)))
  {
    trace::printf ("Test CMSIS++ RTOS based on FreeRTOS.\n");

    scheduler::initialize ();

    // Necessarily static
    static thread::Attributes attr
      { "main"};
    static Thread main_thread
      { attr, (thread::func_t) os_main_task, (thread::func_args_t) nullptr};

    scheduler::start ();

    return 0;
  }
#endif
