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

// ----------------------------------------------------------------------------

#ifdef  __cplusplus

#include <cstdint>
#include <cstddef>

namespace os
{
  namespace rtos
  {
    class Thread;

    namespace port
    {
      // ----------------------------------------------------------------------

      namespace stack
      {
        // Align stack to 8 bytes.
        using element_t = long long;
      } /* namespace stack */

      // Simple list of threads.
      class Tasks_list
      {
      public:

        Tasks_list ();
        ~Tasks_list ();
#if 1
        void
        add (Thread* thread);
        void
        remove (Thread* thread);
        void
        remove (std::size_t pos);

        /**
         * @brief Get top priority task.
         * @return Pointer to task.
         */
        Thread*
        top_prio_task (void);

        bool
        empty(void);

        std::size_t
        length ();

        // TODO add iterator begin(), end()

#endif
      protected:

        Thread* array_[OS_INTEGER_MAX_NUMBER_OF_THREADS];
        std::size_t count_;
      };

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

      inline bool
      Tasks_list::empty(void)
      {
        return (count_ == 0);
      }

      inline std::size_t
      Tasks_list::length ()
      {
        return count_;
      }

    // ----------------------------------------------------------------------

    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* CMSIS_PLUS_RTOS_PORT_OS_DECLS_H_ */
