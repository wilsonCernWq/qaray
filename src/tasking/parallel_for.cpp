///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 12/3/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.             //
///                                                                          //
/// Redistribution and use in source and binary forms, with or without       //
/// modification, are permitted provided that the following conditions are   //
/// met:                                                                     //
///  - Redistributions of source code must retain the above copyright        //
///    notice, this list of conditions and the following disclaimer.         //
///  - Redistributions in binary form must reproduce the above copyright     //
///    notice, this list of conditions and the following disclaimer in the   //
///    documentation and/or other materials provided with the distribution.  //
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS  //
/// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    //
/// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A          //
/// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       //
/// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   //
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT         //
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,    //
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    //
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      //
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE    //
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.     //
///                                                                          //
///--------------------------------------------------------------------------//

#include "parallel_for.h"

#ifdef USE_TBB
# include <tbb/task_arena.h>
# include <tbb/task_scheduler_init.h>
# include <tbb/parallel_for.h>
#endif

namespace qaray {
namespace tasking {
//---------------------------------------------------------------------------//
size_t init_num_of_threads()
{
  size_t num_of_threads = 1;
#if defined(USE_TBB)
# if TBB_INTERFACE_VERSION >= 9100
  num_of_threads = static_cast<size_t>(tbb::this_task_arena::max_concurrency());
# else
  num_of_threads = tbb::task_scheduler_init::default_num_threads();
# endif
#elif defined(USE_OMP)
# pragma omp parallel
  {
    num_of_threads = static_cast<size_t>(omp_get_num_threads());
  }
#endif
  return num_of_threads;
}
//---------------------------------------------------------------------------//
static size_t threadSize = init_num_of_threads();
//---------------------------------------------------------------------------//
size_t get_num_of_threads() { return threadSize; }
void set_num_of_threads(size_t num_of_threads)
{
  if (0 < num_of_threads && num_of_threads < threadSize) {
    threadSize = num_of_threads;
  }
}

void init()
{
#if defined(USE_TBB)
  tbb::task_scheduler_init init(static_cast<int>(threadSize));
#elif defined(USE_OMP)
  omp_set_num_threads(threadSize);
#endif
}

void parallel_for(size_t start, size_t end, size_t step,
                  const std::function<void(size_t)> &T)
{
#if defined(USE_TBB)
  tbb::parallel_for(start, end, step, T);
#elif defined(USE_OMP)
# pragma omp parallel for
  for (size_t i = start; i < end; i += step) { T(i); }
#else
  for (size_t i = start; i < end; i += step) { T(i); }
#endif
}

}
}