// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#include "base/math.hpp"

#include <type_traits>

#include "base/testing.hpp"

namespace lib {

constexpr size_t N = 3;
constexpr size_t M = 3;

TEST_CASE("Matrix") {
  SECTION("IsEigenMatrix") {
    // Postconditions.
    REQUIRE(std::is_same_v<Matrix<N, M>, Eigen::Matrix<double, N, M>>);
  }
}

TEST_CASE("Vector") {
  SECTION("IsEigenMatrix") {
    // Postconditions.
    REQUIRE(std::is_same_v<Vector<N>, Eigen::Matrix<double, N, 1>>);
  }
}

}  // namespace lib
