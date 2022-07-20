/**
 * Copyright (c) 2017 Eric Bruneton
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "util/progress_bar.h"

#include <chrono>
#include <iostream>
#include <limits>

std::mutex ProgressBar::mutex_;

ProgressBar::ProgressBar(unsigned int progress_max) : progress_(0) {
  thread_ = std::thread([this, progress_max]() {
    constexpr unsigned int kProgressBarWidth = 64;
    std::unique_lock<std::mutex> lock(mutex_);
    while (true) {
      stop_.wait_for(lock, std::chrono::seconds(1));
      if (progress_ >= progress_max) {
        for (unsigned int i = 0; i < kProgressBarWidth + 2; ++i) {
          std::cout << " ";
        }
        std::cout << "\r";
        std::cout.flush();
        return;
      }
      float progress = static_cast<float>(progress_) / progress_max;
      unsigned int bar_progress =
          static_cast<unsigned int>(progress * kProgressBarWidth);
      std::cout << "[";
      for (unsigned int i = 0; i < kProgressBarWidth; ++i) {
        std::cout << (i <= bar_progress ? "#" : "-");
      }
      std::cout << "]\r";
      std::cout.flush();
    }
  });
}

ProgressBar::~ProgressBar() {
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    progress_ = std::numeric_limits<unsigned int>::max();
  }
  stop_.notify_one();
  thread_.join();
}

void RunJobs(std::function<void(unsigned int)> job, unsigned int job_count) {
  constexpr unsigned int kNumThreads = 8;
  std::thread threads[kNumThreads];
  for (unsigned int thread_id = 0; thread_id < kNumThreads; ++thread_id) {
    threads[thread_id] = std::thread([job, job_count, thread_id]() {
      for (unsigned int job_id = 0; job_id < job_count; ++job_id) {
        if (job_id % kNumThreads == thread_id) {
          job(job_id);
        }
      }
    });
  }
  for (unsigned int thread_id = 0; thread_id < kNumThreads; ++thread_id) {
    threads[thread_id].join();
  }
}
