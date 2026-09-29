// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#include "base/contract.hpp"

#include "base/testing.hpp"

namespace lib {

TEST_CASE("Contracts") {
  SECTION("ShouldThrowPreconditionErrorGivenFalseExpect") {
    REQUIRE_THROWS_AS(EXPECT(false), contract::PreconditionError);
  }
  SECTION("ShouldThrowPostconditionErrorGivenFalseEnsure") {
    REQUIRE_THROWS_AS(ENSURE(false), contract::PostconditionError);
  }
  SECTION("ShouldThrowInvariantErrorGivenFalseAssert") {
    REQUIRE_THROWS_AS(ASSERT(false), contract::InvariantError);
  }
  SECTION("ShouldHaveSourceLocationGivenFalseExpect") {
    bool thrown = false;
    try {
      EXPECT(false);
    } catch (const contract::PreconditionError& e) {
      thrown = true;
      CHECK(static_cast<std::string>(e.origin.file_name()).size());
    }
    REQUIRE(thrown);
  }
  SECTION("ShouldHaveSourceLocationGivenFalseEnsure") {
    bool thrown = false;
    try {
      ENSURE(false);
    } catch (const contract::PostconditionError& e) {
      thrown = true;
      CHECK(static_cast<std::string>(e.origin.file_name()).size());
    }
    REQUIRE(thrown);
  }
  SECTION("ShouldHaveSourceLocationGivenFalseAssert") {
    bool thrown = false;
    try {
      ASSERT(false);
    } catch (const contract::InvariantError& e) {
      thrown = true;
      CHECK(static_cast<std::string>(e.origin.file_name()).size());
    }
    REQUIRE(thrown);
  }
  SECTION("ShouldNotThrowGivenTrueExpect") {
    REQUIRE_NOTHROW(EXPECT(true));
  }
  SECTION("ShouldNotThrowGivenTrueEnsure") {
    REQUIRE_NOTHROW(ENSURE(true));
  }
  SECTION("ShouldNotThrowGivenTrueAssert") {
    REQUIRE_NOTHROW(ASSERT(true));
  }
}

}  // namespace lib
