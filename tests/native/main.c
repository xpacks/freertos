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

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include <cmsis-plus/diag/trace.h>

void
task (void* args);

// -----------

void
task (void* args __attribute__((unused)))
{
  int count = 0;
  for (;;)
    {
      ++count;
      trace_printf ("%d\n", count);

      vTaskDelay (configTICK_RATE_HZ);
    }
}

int
main (int argc __attribute__((unused)), char* argv[] __attribute__((unused)))
{
  trace_printf ("Test native FreeRTOS thread with second counting.\n");

  TaskHandle_t taskHandler = NULL;
  xTaskCreate(task, "task", 1024, NULL, tskIDLE_PRIORITY, &taskHandler);
  configASSERT( taskHandler );

  vTaskStartScheduler ();
  return 0;
}

