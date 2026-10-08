// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#include "base/contract.hpp"

#include "base/testing.hpp"

namespace lib {

TEST_CASE("Contracts") {
  SECTION("ShouldThrowPreconditionErrorGivenFalseExpect") {
    // Postconditions.
    REQUIRE_THROWS_AS(EXPECT(false), contract::PreconditionError);
  }

  SECTION("ShouldThrowPostconditionErrorGivenFalseEnsure") {
    // Postconditions.
    REQUIRE_THROWS_AS(ENSURE(false), contract::PostconditionError);
  }

  SECTION("ShouldThrowInvariantErrorGivenFalseAssert") {
    // Postconditions.
    REQUIRE_THROWS_AS(ASSERT(false), contract::InvariantError);
  }

  SECTION("ShouldHaveSourceLocationGivenFalseExpect") {
    // Preconditions.
    bool thrown = false;

    // Under Test.
    try {
      EXPECT(false);
    } catch (const contract::PreconditionError& e) {
      thrown = true;
      CHECK(static_cast<std::string>(e.origin.file_name()).size());
    }

    // Postconditions.
    REQUIRE(thrown);
  }

  SECTION("ShouldHaveSourceLocationGivenFalseEnsure") {
    // Preconditions.
    bool thrown = false;

    // Under Test.
    try {
      ENSURE(false);
    } catch (const contract::PostconditionError& e) {
      thrown = true;
      CHECK(static_cast<std::string>(e.origin.file_name()).size());
    }

    // Postconditions.
    REQUIRE(thrown);
  }

  SECTION("ShouldHaveSourceLocationGivenFalseAssert") {
    // Preconditions.
    bool thrown = false;

    // Under Test.
    try {
      ASSERT(false);
    } catch (const contract::InvariantError& e) {
      thrown = true;
      CHECK(static_cast<std::string>(e.origin.file_name()).size());
    }

    // Postconditions.
    REQUIRE(thrown);
  }

  SECTION("ShouldNotThrowGivenTrueExpect") {
    // Postconditions.
    REQUIRE_NOTHROW(EXPECT(true));
  }

  SECTION("ShouldNotThrowGivenTrueEnsure") {
    // Postconditions.
    REQUIRE_NOTHROW(ENSURE(true));
  }

  SECTION("ShouldNotThrowGivenTrueAssert") {
    // Postconditions.
    REQUIRE_NOTHROW(ASSERT(true));
  }
}

}  // namespace lib
