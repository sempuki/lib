// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#pragma once

#include <chrono>

namespace lib {

// Simulation time, as opposed to wall time. SimTime is a tag: it makes a
// simulation time point a different type from a wall-clock time point, so the
// two cannot be mixed by accident. It has no now(), because a global clock
// would give code a hidden source of time; current time comes from whatever
// drives the simulation.
//
// Time counts int64 nanoseconds, which is exact and covers about 292 years.
struct SimTime;

using Duration = std::chrono::nanoseconds;
using TimePoint = std::chrono::time_point<SimTime, Duration>;

}  // namespace lib
