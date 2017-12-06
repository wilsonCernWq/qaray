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

#ifndef QARAY_PARALLEL_FOR_H
#define QARAY_PARALLEL_FOR_H
#pragma once

#include <cstddef>
#include <vector>
#include <functional>
#include <atomic>

#ifndef USE_MULTI_THREADING
# define USE_MULTI_THREADING 1
#endif

#if USE_MULTI_THREADING
#else
# undef USE_TBB
# undef USE_OMP
#endif

#ifdef USE_TBB
# include <tbb/task_arena.h>
# include <tbb/task_scheduler_init.h>
# include <tbb/parallel_for.h>
# include <tbb/enumerable_thread_specific.h>
#endif
#ifdef USE_OMP
# include <omp.h>
#endif

namespace qaray {
namespace tasking {
size_t get_num_of_threads();
void set_num_of_threads(size_t num_of_threads);
void init();
void signal_start();
void signal_stop();
bool has_stop_signal();
void parallel_for(size_t start,
                  size_t end,
                  size_t step,
                  std::function<void(size_t)> T);

#if defined(USE_TBB)
template<typename T>
struct ThreadLocalStorage : public tbb::enumerable_thread_specific<T> {
  explicit ThreadLocalStorage(const T &t) :
      tbb::enumerable_thread_specific<T>(t) {};
};
#elif defined(USE_OMP)
template<typename T>
struct ThreadLocalStorage {
  const T data;
  explicit ThreadLocalStorage(const T &t) : data(t) {};
  T& local() {
    static std::vector<T*> list(get_num_of_threads(), nullptr);
    int tid = omp_get_thread_num();
    if (list[tid] == nullptr) { list[tid] = new T(data); }
    return *list[tid];
  }
};
#else
template<typename T>
struct ThreadLocalStorage {
  T data;
  explicit ThreadLocalStorage(const T &t) : data(t) {}
  T& local() { return data; }
};
#endif
}
}

#endif //QARAY_PARALLEL_FOR_H
