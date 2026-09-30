// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#include "base/time.hpp"

#include <chrono>
#include <type_traits>

#include "base/testing.hpp"

namespace lib {

using namespace std::chrono_literals;

template <typename A, typename B>
concept Subtractable = requires(A a, B b) { a - b; };

TEST_CASE("Duration") {
  SECTION("ShouldCountIntegerNanosecondsGivenDefinition") {
    static_assert(std::is_same_v<Duration, std::chrono::nanoseconds>);
  }

  SECTION("ShouldBeExactGivenManySmallSteps") {
    // Ten steps of 0.1 s land exactly on 1 s, which double seconds do not.
    Duration total{};
    for (int i = 0; i < 10; ++i) {
      total += 100ms;
    }
    CHECK(total == 1s);
  }

  SECTION("ShouldBeZeroGivenValueInitialization") {
    CHECK(Duration{}.count() == 0);
  }
}

TEST_CASE("TimePoint") {
  SECTION("ShouldBeZeroGivenDefault") {
    CHECK(TimePoint{}.time_since_epoch() == Duration::zero());
  }

  SECTION("ShouldAdvanceByDurationGivenAddition") {
    CHECK((TimePoint{} + 250ms).time_since_epoch() == 250ms);
    CHECK(TimePoint{2s} - TimePoint{500ms} == 1500ms);
  }

  SECTION("ShouldNotMixGivenWallClockTimePoint") {
    static_assert(
        !Subtractable<TimePoint, std::chrono::steady_clock::time_point>);
    static_assert(Subtractable<TimePoint, TimePoint>);
  }
}

}  // namespace lib
