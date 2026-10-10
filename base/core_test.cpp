// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#include "base/core.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <tuple>

#include "base/testing.hpp"

namespace lib {

TEST_CASE("NarrowCast") {
  SECTION("ShouldReturnSameValueGivenValueInRange") {
    // Postconditions.
    REQUIRE(narrow_cast<std::int8_t>(127) == 127);
    REQUIRE(narrow_cast<std::int8_t>(-128) == -128);
    REQUIRE(narrow_cast<std::uint32_t>(std::int64_t{0xFFFFFFFF}) ==
            0xFFFFFFFFu);
  }

  SECTION("ShouldThrowGivenOverflow") {
    // Postconditions.
    REQUIRE_THROWS_AS(narrow_cast<std::int8_t>(128), std::logic_error);
    REQUIRE_THROWS_AS(narrow_cast<std::int32_t>(std::int64_t{1} << 32),
                      std::logic_error);
  }

  SECTION("ShouldThrowGivenSignChange") {
    // Postconditions.
    REQUIRE_THROWS_AS(narrow_cast<std::uint32_t>(-1), std::logic_error);
    REQUIRE_THROWS_AS(
        narrow_cast<std::int32_t>(std::numeric_limits<std::uint32_t>::max()),
        std::logic_error);
  }
}

namespace {

auto halve(int value) -> std::expected<int, std::string> {
  if (value % 2 != 0) {
    return std::unexpected("odd");
  }
  return value / 2;
}

auto boxed(int value) -> std::expected<std::unique_ptr<int>, std::string> {
  if (value < 0) {
    return std::unexpected("negative");
  }
  return std::make_unique<int>(value);
}

// Halves twice, declaring each result.
auto halve_twice(int value) -> std::expected<int, std::string> {
  RETURN_OR_ASSIGN_TO(int half, halve(value));
  RETURN_OR_ASSIGN_TO(int quarter, halve(half));
  return quarter;
}

// Checks without keeping the value, then assigns an existing variable.
auto checked_halve(int value) -> std::expected<int, std::string> {
  RETURN_IF_UNEXPECTED(halve(value));
  int half = 0;
  RETURN_OR_ASSIGN_TO(half, halve(value));
  return half;
}

auto unboxed(int value) -> std::expected<int, std::string> {
  RETURN_OR_ASSIGN_TO(std::unique_ptr<int> box, boxed(value));
  return *box;
}

auto check_even(int value, bool skip) -> std::expected<void, std::string> {
  if (skip)
    RETURN_IF_UNEXPECTED(halve(value))
  else {
    return std::unexpected("skipped");
  }
  return {};
}

}  // namespace

TEST_CASE("PropagateErrors") {
  SECTION("ShouldReturnValueGivenEveryStepSucceeds") {
    // Postconditions.
    CHECK(halve_twice(8) == 2);
    CHECK(checked_halve(6) == 3);
    CHECK(unboxed(5) == 5);
  }

  SECTION("ShouldReturnFirstErrorGivenAStepFails") {
    // Postconditions.
    CHECK(halve_twice(6).error() == "odd");  // 6 halves to 3, which is odd.
    CHECK(halve_twice(5).error() == "odd");
    CHECK(checked_halve(3).error() == "odd");
    CHECK(unboxed(-1).error() == "negative");
  }

  SECTION("ShouldTakeOuterElseGivenFalseOuterIf") {
    // Postconditions.
    CHECK(check_even(4, true).has_value());
    CHECK(check_even(3, true).error() == "odd");
    CHECK(check_even(4, false).error() == "skipped");
  }
}

namespace {
struct Base {
  int base = 1;
};
struct Derived final : Base {};
struct Left {
  int left = 2;
};
struct Both final : Left, Base {};
}  // namespace

TEST_CASE("CheckedPointerConversion") {
  SECTION("ShouldStayUnusedGivenUpcastOfUnusedOut") {
    // Preconditions.
    Out<Derived> derived{unused};

    // Under Test.
    Out<Base> base{derived};

    // Postconditions.
    REQUIRE_FALSE(base);
  }

  SECTION("ShouldPointAtBaseSubobjectGivenUpcast") {
    // Preconditions.
    Both both;
    InOut<Both> whole{both};

    // Under Test.
    InOut<Base> part{whole};

    // Postconditions.
    REQUIRE(&*part == static_cast<Base*>(&both));
    REQUIRE(part->base == 1);
  }
}

TEST_CASE("CheckContract") {
  SECTION("ShouldThrowWithConditionTextGivenFalseCondition") {
    // Postconditions.
    try {
      CHECK_PRECONDITION(1 + 1 == 3)
      FAIL("CHECK_PRECONDITION did not throw");
    } catch (const std::logic_error& e) {
      REQUIRE(std::string_view{e.what()}.contains("Precondition"));
      REQUIRE(std::string_view{e.what()}.contains("1 + 1 == 3"));
    }
  }

  SECTION("ShouldReportCallerLineGivenFalseCondition") {
    // Preconditions.
    std::uint_least32_t line = 0;

    // Postconditions.
    try {
      line = std::source_location::current().line() + 1;
      CHECK_INVARIANT(false);
      FAIL("CHECK_INVARIANT did not throw");
    } catch (const std::logic_error& e) {
      REQUIRE(std::string_view{e.what()}.contains(
          std::format("core_test.cpp:{}", line)));
    }
  }

  SECTION("ShouldReportCallerLocationGivenFalseCondition") {
    // Postconditions.
    try {
      CHECK_INVARIANT(false);
      FAIL("CHECK_INVARIANT did not throw");
    } catch (const std::logic_error& e) {
      REQUIRE(std::string_view{e.what()}.contains("core_test.cpp"));
    }
  }

  SECTION("ShouldTakeOuterElseGivenFalseOuterIf") {
    // Preconditions.
    bool took_else = false;
    const bool outer = false;

    // Under Test.
    // The `else` must bind to the outer `if`, not the one inside the macro.
    if (outer)
      CHECK_PRECONDITION(true)
    else
      took_else = true;

    // Postconditions.
    REQUIRE(took_else);
  }
}

TEST_CASE("CheckedPointer") {
  SECTION("ShouldThrowGivenNullPointer") {
    // Preconditions.
    CheckedPointer<int> pointer;

    // Postconditions.
    REQUIRE_FALSE(pointer);
    REQUIRE_THROWS_AS(*pointer, std::logic_error);
    REQUIRE_THROWS_AS(pointer.get(), std::logic_error);
  }

  SECTION("ShouldWriteThroughGivenBoundValue") {
    // Preconditions.
    int value = 1;
    Out<int> out{value};

    // Under Test.
    *out = 2;

    // Postconditions.
    REQUIRE(value == 2);
  }
}

template <int N>
struct Deep {};

TEST_CASE("Demangle") {
  SECTION("ShouldReturnReadableNameGivenBuiltinType") {
    // Postconditions.
    REQUIRE(to_type_string<int>() == "int");
  }

  SECTION("ShouldReturnFullNameGivenNameLongerThan1024Chars") {
    // Preconditions.
    using Long =
        std::tuple<Deep<0>, Deep<1>, Deep<2>, Deep<3>, Deep<4>, Deep<5>,
                   Deep<6>, Deep<7>, Deep<8>, Deep<9>, Deep<10>, Deep<11>,
                   Deep<12>, Deep<13>, Deep<14>, Deep<15>, Deep<16>, Deep<17>,
                   Deep<18>, Deep<19>, Deep<20>, Deep<21>, Deep<22>, Deep<23>,
                   Deep<24>, Deep<25>, Deep<26>, Deep<27>, Deep<28>, Deep<29>,
                   Deep<30>, Deep<31>, Deep<32>, Deep<33>, Deep<34>, Deep<35>,
                   Deep<36>, Deep<37>, Deep<38>, Deep<39>>;

    // Under Test.
    const std::string name = to_type_string<std::tuple<Long, Long>>();

    // Postconditions.
    REQUIRE(name.size() > 1024u);
    REQUIRE(name.starts_with("std::tuple<std::tuple<lib::Deep<0>"));
    REQUIRE(name.contains("lib::Deep<39>"));
  }
}

TEST_CASE("StableHash") {
  SECTION("ShouldIgnoreBytesPastTheEndGivenLengthNotMultipleOfBlock") {
    // Preconditions.
    // Same 12 characters, followed by different bytes that are not part of
    // the string. A hash that reads past the end would see the difference.
    const std::string first = "abcdefghijklXXXX";
    const std::string second = "abcdefghijklYYYY";

    // Postconditions.
    REQUIRE(stable_hash(std::string_view{first}.substr(0, 12)) ==
            stable_hash(std::string_view{second}.substr(0, 12)));
  }

  SECTION("ShouldDifferGivenDifferentTails") {
    // Postconditions.
    REQUIRE(stable_hash("abcdefghijk1") != stable_hash("abcdefghijk2"));
  }

  SECTION("ShouldMatchKnownValueGivenFixedInputs") {
    // Postconditions.
    // The same on every platform: these were computed with both signed and
    // unsigned char. Blocks are eight bytes; the last check has a byte over
    // 0x7f.
    CHECK(stable_hash("") == 0xeee234c470228e5dULL);
    CHECK(stable_hash("abcdefgh") == 0x2d4c755f42e53ab5ULL);
    CHECK(stable_hash("abcdefghi") == 0xacdf21f24794b3a7ULL);
    CHECK(stable_hash("/world/1/entity/2") == 0x1009c5a7c6933fafULL);
    CHECK(stable_hash("\xff") == 0x0e186611ec97e975ULL);
  }
}

}  // namespace lib
