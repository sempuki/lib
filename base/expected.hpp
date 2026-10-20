// Copyright 2026 -- CONTRIBUTORS. See LICENSE.

#pragma once

#include <expected>  // IWYU pragma: keep
#include <utility>   // IWYU pragma: keep

// Propagating errors from std::expected, as a `?` operator would. The enclosing
// function must return a std::expected whose error type the error converts to.
//
//   PROPAGATE_UNEXPECTED(build_world(scenario, Out(world)));
//   PROPAGATE_UNEXPECTED_OR_ASSIGN(Entity asset, build_scenario(InOut(world)));
//   PROPAGATE_UNEXPECTED_OR_ASSIGN(asset_, build_scenario(InOut(world)));

// Returns the error of `expression__`, a std::expected, if it has one. The
// empty then-branch closes the `if`, so a trailing `else` at the call site
// binds to the caller's `if` instead of this one.
#define PROPAGATE_UNEXPECTED(expression__)                      \
  if (auto&& result__ = (expression__); result__.has_value()) { \
  } else [[unlikely]] {                                         \
    return std::unexpected(std::move(result__).error());        \
  }

// Returns the error of `expression__`, a std::expected, if it has one, and
// otherwise moves its value into `target__`, an existing variable or a new
// declaration such as `Entity asset`. It is several statements, so use it only
// where a statement can go, at most once per line.
#define PROPAGATE_UNEXPECTED_OR_ASSIGN(target__, expression__) \
  PROPAGATE_UNEXPECTED_OR_ASSIGN_INNER__(                      \
      CONCATENATE__(result_on_line_, __LINE__), target__, expression__)

#define PROPAGATE_UNEXPECTED_OR_ASSIGN_INNER__(result__, target__, \
                                               expression__)       \
  auto&& result__ = (expression__);                                \
  if (!result__.has_value()) [[unlikely]] {                        \
    return std::unexpected(std::move(result__).error());           \
  }                                                                \
  target__ = *std::move(result__)

#define CONCATENATE__(first__, second__) CONCATENATE_INNER__(first__, second__)
#define CONCATENATE_INNER__(first__, second__) first__##second__
