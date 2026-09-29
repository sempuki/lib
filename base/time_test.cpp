// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#include "base/time.hpp"

#include "base/testing.hpp"

namespace lib {

TEST_CASE("SimClock") {
  SECTION("ShouldReturnTimePointFromStaticNow") {
    auto time_point = SimClock::now();
    REQUIRE(std::is_same_v<decltype(time_point), TimePoint>);
  }
}

TEST_CASE("Duration") {
  SECTION("IsChronoDurationFractionsOfSecond") {
    REQUIRE(std::is_same_v<Duration, std::chrono::duration<double, std::ratio<1>>>);
  }

  SECTION("ShouldConvertToAndFromDouble") {
    Duration d{3.14};
    CHECK(d.count() == 3.14);
  }

  SECTION("ShouldBeZeroWhenValueInitialized") {
    // Like any chrono::duration, a default-initialized Duration is indeterminate.
    Duration d{};
    CHECK(d.count() == 0.0);
  }
}

TEST_CASE("TimePoint") {
  SECTION("IsChronoTimePoint") {
    REQUIRE(std::is_same_v<TimePoint, std::chrono::time_point<SimClock>>);
  }

  SECTION("ShouldConvertToAndFromDouble") {
    TimePoint t{Duration{3.14}};
    CHECK(t.time_since_epoch().count() == 3.14);
  }

  SECTION("ShouldBeZeroByDefault") {
    TimePoint t;
    CHECK(t.time_since_epoch().count() == 0.0);
  }
}

}  // namespace lib
