/*
 * DIPlib 3.0
 * This file contains the declaration for a timer object
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DIP_TIMER_H
#define DIP_TIMER_H

#include <chrono>
#include <ctime>
#include <iostream>

#include "diplib/library/types.h"


/// \file
/// \brief Declares the `dip::Timer` class.


namespace dip {


/// \addtogroup infrastructure
/// \{


/// \brief A timer object to help time algorithm execution.
///
/// The methods `GetCpu` and `GetWall` return the CPU and wall time, respectively, in seconds that passed in
/// between object creation and the last call to `Stop`. `Stop` does not actually stop the timer, it just
/// records the time it was last called. `Reset` resets the timer, as if it had just been created.
///
/// ```cpp
///     dip::Timer timer;
///     // do some computation
///     timer.Stop();
///     std::cout << "Computation 1: Wall time = " << timer.GetWall() << " s. CPU time = " << timer.GetCpu() << " s.\n";
///     timer.Reset();
///     // do some other computation
///     timer.Stop();
///     std::cout << "Computation 2: Wall time = " << timer.GetWall() << " s. CPU time = " << timer.GetCpu() << " s.\n";
/// ```
///
/// Note that it is also possible to directly put the timer object to the output stream:
///
/// ```cpp
///     dip::Timer timer;
///     // do some computation
///     timer.Stop();
///     std::cout << "Computation 1: " << timer << std::endl;
/// ```
///
/// Wall time is the real-world time that elapsed. CPU time is the time that the CPU spent working for the current
/// program. These differ in two ways: CPU time might pass slower if the program has to share resources with other
/// running programs; and CPU time might pass faster if there are multiple CPUs working for the same program. The
/// latter case means that, on a multi-threaded environment, CPU time is the sum of times for each of the executed
/// threads.
///
/// Wall time is obtained through `std::chrono::steady_clock`, and CPU time through `std::clock`. This object does
/// not do anything special with these standar library routines, except for providing a simpler interface.
class DIP_NO_EXPORT Timer {
   public:

      /// \brief The default-constructed object records its creation time as the start time for the timer.
      Timer() {
         Reset();
      }

      /// \brief Records the current time as the start time for the timer.
      void Reset() {
         startWall_ = endWall_ = std::chrono::steady_clock::now();
         startCpu_ = endCpu_ = clock();
      }

      /// \brief Records the current time as the stop time for the timer.
      void Stop() {
         endCpu_ = clock();
         endWall_ = std::chrono::steady_clock::now();
      }

      /// \brief Returns the CPI time elapsed in between the creation of the timer (or the last call to `Reset`) and the last call to `Stop`.
      dfloat GetCpu() const {
         return static_cast< dfloat >( endCpu_ - startCpu_ ) / static_cast< dfloat >( CLOCKS_PER_SEC );
      }

      /// \brief Returns the wall time elapsed in between the creation of the timer (or the last call to `Reset`) and the last call to `Stop`.
      dfloat GetWall() const {
         std::chrono::duration< dfloat > time = endWall_ - startWall_;
         return time.count();
      }

      /// \brief Returns the number of clock ticks per second for the CPU clock.
      dfloat CpuResolution() const {
         return 1.0 / static_cast< dfloat >( CLOCKS_PER_SEC );
      }

      /// \brief Returns the number of clock ticks per second for the wall clock.
      dfloat WallResolution() const {
         return static_cast< dfloat >( std::chrono::steady_clock::period::num ) / static_cast< dfloat >( std::chrono::steady_clock::period::den );
      }

   private:
      std::chrono::time_point< std::chrono::steady_clock > startWall_;
      std::chrono::time_point< std::chrono::steady_clock > endWall_;
      std::clock_t startCpu_;
      std::clock_t endCpu_;
};

/// \brief Reports elapsed time to a stream.
inline std::ostream& operator<<(
      std::ostream& os,
      Timer const& timer
) {
   os << "elapsed time = " << timer.GetWall() << " s (wall), " << timer.GetCpu() << " s (CPU)";
   return os;
}


/// \}

} // namespace dip

#endif // DIP_TIMER_H