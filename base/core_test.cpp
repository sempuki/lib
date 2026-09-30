// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#include "base/core.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <tuple>

#include "base/testing.hpp"

namespace lib {

TEST_CASE("NarrowCast") {
  SECTION("ShouldReturnSameValueGivenValueInRange") {
    REQUIRE(narrow_cast<std::int8_t>(127) == 127);
    REQUIRE(narrow_cast<std::int8_t>(-128) == -128);
    REQUIRE(narrow_cast<std::uint32_t>(std::int64_t{0xFFFFFFFF}) == 0xFFFFFFFFu);
  }

  SECTION("ShouldThrowGivenOverflow") {
    REQUIRE_THROWS_AS(narrow_cast<std::int8_t>(128), std::logic_error);
    REQUIRE_THROWS_AS(narrow_cast<std::int32_t>(std::int64_t{1} << 32), std::logic_error);
  }

  SECTION("ShouldThrowGivenSignChange") {
    REQUIRE_THROWS_AS(narrow_cast<std::uint32_t>(-1), std::logic_error);
    REQUIRE_THROWS_AS(narrow_cast<std::int32_t>(std::numeric_limits<std::uint32_t>::max()),
                      std::logic_error);
  }
}

TEST_CASE("CheckContract") {
  SECTION("ShouldThrowWithConditionTextGivenFalseCondition") {
    try {
      CHECK_PRECONDITION(1 + 1 == 3)
      FAIL("CHECK_PRECONDITION did not throw");
    } catch (const std::logic_error& e) {
      REQUIRE(std::string_view{e.what()}.contains("Precondition"));
      REQUIRE(std::string_view{e.what()}.contains("1 + 1 == 3"));
    }
  }

  SECTION("ShouldTakeOuterElseGivenFalseOuterIf") {
    // The `else` must bind to the outer `if`, not the one inside the macro.
    bool took_else = false;
    const bool outer = false;
    if (outer)
      CHECK_PRECONDITION(true)
    else
      took_else = true;
    REQUIRE(took_else);
  }
}

TEST_CASE("CheckedPointer") {
  SECTION("ShouldThrowGivenNullPointer") {
    CheckedPointer<int> pointer;
    REQUIRE_FALSE(pointer);
    REQUIRE_THROWS_AS(*pointer, std::logic_error);
    REQUIRE_THROWS_AS(pointer.get(), std::logic_error);
  }

  SECTION("ShouldWriteThroughGivenBoundValue") {
    int value = 1;
    Out<int> out{value};
    *out = 2;
    REQUIRE(value == 2);
  }
}

template <int N>
struct Deep {};

TEST_CASE("Demangle") {
  SECTION("ShouldReturnReadableNameGivenBuiltinType") {
    REQUIRE(to_type_string<int>() == "int");
  }

  SECTION("ShouldReturnFullNameGivenNameLongerThan1024Chars") {
    using Long = std::tuple<Deep<0>,
                            Deep<1>,
                            Deep<2>,
                            Deep<3>,
                            Deep<4>,
                            Deep<5>,
                            Deep<6>,
                            Deep<7>,
                            Deep<8>,
                            Deep<9>,
                            Deep<10>,
                            Deep<11>,
                            Deep<12>,
                            Deep<13>,
                            Deep<14>,
                            Deep<15>,
                            Deep<16>,
                            Deep<17>,
                            Deep<18>,
                            Deep<19>,
                            Deep<20>,
                            Deep<21>,
                            Deep<22>,
                            Deep<23>,
                            Deep<24>,
                            Deep<25>,
                            Deep<26>,
                            Deep<27>,
                            Deep<28>,
                            Deep<29>,
                            Deep<30>,
                            Deep<31>,
                            Deep<32>,
                            Deep<33>,
                            Deep<34>,
                            Deep<35>,
                            Deep<36>,
                            Deep<37>,
                            Deep<38>,
                            Deep<39>>;
    const std::string name = to_type_string<std::tuple<Long, Long>>();
    REQUIRE(name.size() > 1024u);
    REQUIRE(name.starts_with("std::tuple<std::tuple<lib::Deep<0>"));
    REQUIRE(name.contains("lib::Deep<39>"));
  }
}

TEST_CASE("StableHash") {
  SECTION("ShouldIgnoreBytesPastTheEndGivenLengthNotMultipleOfBlock") {
    // Same 12 characters, followed by different bytes that are not part of
    // the string. A hash that reads past the end would see the difference.
    const std::string first = "abcdefghijklXXXX";
    const std::string second = "abcdefghijklYYYY";
    REQUIRE(stable_hash(std::string_view{first}.substr(0, 12)) ==
            stable_hash(std::string_view{second}.substr(0, 12)));
  }

  SECTION("ShouldDifferGivenDifferentTails") {
    REQUIRE(stable_hash("abcdefghijk1") != stable_hash("abcdefghijk2"));
  }
}

}  // namespace lib
