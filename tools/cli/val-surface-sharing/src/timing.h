/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef TOOLS_CLI_VAL_SURFACE_SHARING_SRC_TIMING_H_
#define TOOLS_CLI_VAL_SURFACE_SHARING_SRC_TIMING_H_

#include <chrono>

class StopWatch {
public:
    StopWatch() : start_time(), elapsed_time() {}

    ~StopWatch() {}

    void start_lap() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void end_lap() {
        auto end_time = std::chrono::high_resolution_clock::now();
        elapsed_time += end_time - start_time;
        start_time = end_time;
    }

    double elapsed_ms() {
        std::chrono::microseconds elapsed_us =
            std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time);
        return elapsed_us.count() / 1000.0;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::duration elapsed_time;
};

#endif // TOOLS_CLI_VAL_SURFACE_SHARING_SRC_TIMING_H_
