// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#pragma once

#include <source_location>

namespace lib::contract {
struct ContractError {
  std::source_location origin;
};

struct PreconditionError final : public ContractError {};
struct PostconditionError final : public ContractError {};
struct InvariantError final : public ContractError {};
}  // namespace lib::contract

#define EXPECT(expr)                            \
  [&] {                                         \
    if (!(expr)) [[unlikely]] {                 \
      throw ::lib::contract::PreconditionError{ \
          std::source_location::current()};     \
    }                                           \
  }()

#define ENSURE(expr)                             \
  [&] {                                          \
    if (!(expr)) [[unlikely]] {                  \
      throw ::lib::contract::PostconditionError{ \
          std::source_location::current()};      \
    }                                            \
  }()

#define ASSERT(expr)                                                          \
  [&] {                                                                       \
    if (!(expr)) [[unlikely]] {                                               \
      throw ::lib::contract::InvariantError{std::source_location::current()}; \
    }                                                                         \
  }()
