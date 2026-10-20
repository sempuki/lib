// Copyright 2026 -- CONTRIBUTORS. See LICENSE.

#include "base/expected.hpp"

#include <expected>
#include <memory>
#include <string>

#include "base/testing.hpp"

namespace lib {

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

auto halve_literal(int value) -> std::expected<int, const char*> {
  if (value % 2 != 0) {
    return std::unexpected("odd literal");
  }
  return value / 2;
}

// Halves twice, declaring each result.
auto halve_twice(int value) -> std::expected<int, std::string> {
  PROPAGATE_UNEXPECTED_OR_ASSIGN(int half, halve(value));
  PROPAGATE_UNEXPECTED_OR_ASSIGN(int quarter, halve(half));
  return quarter;
}

// Checks without keeping the value, then assigns an existing variable.
auto checked_halve(int value) -> std::expected<int, std::string> {
  PROPAGATE_UNEXPECTED(halve(value));
  int half = 0;
  PROPAGATE_UNEXPECTED_OR_ASSIGN(half, halve(value));
  return half;
}

auto unboxed(int value) -> std::expected<int, std::string> {
  PROPAGATE_UNEXPECTED_OR_ASSIGN(std::unique_ptr<int> box, boxed(value));
  return *box;
}

// Propagates an error of one type into a function returning another.
auto halve_converted(int value) -> std::expected<int, std::string> {
  PROPAGATE_UNEXPECTED(halve_literal(value));
  PROPAGATE_UNEXPECTED_OR_ASSIGN(int half, halve_literal(value));
  return half;
}

auto check_even(int value, bool skip) -> std::expected<void, std::string> {
  if (skip)
    PROPAGATE_UNEXPECTED(halve(value))
  else {
    return std::unexpected("skipped");
  }
  return {};
}

}  // namespace

TEST_CASE("PropagateUnexpected") {
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

  SECTION("ShouldConvertErrorGivenConvertibleErrorType") {
    // Postconditions.
    CHECK(halve_converted(4) == 2);
    CHECK(halve_converted(3).error() == "odd literal");
  }

  SECTION("ShouldTakeOuterElseGivenFalseOuterIf") {
    // Postconditions.
    CHECK(check_even(4, true).has_value());
    CHECK(check_even(3, true).error() == "odd");
    CHECK(check_even(4, false).error() == "skipped");
  }
}

}  // namespace lib
