// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#pragma once

#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <format>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <print>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>

#ifdef __GNUC__  // Clang also supports this header.
#endif

#define DECLARE_COPY_DEFAULT(class_name__)              \
  class_name__(const class_name__&) noexcept = default; \
  auto operator=(const class_name__&) noexcept -> class_name__& = default;

#define DECLARE_COPY_DELETE(class_name__)     \
  class_name__(const class_name__&) = delete; \
  auto operator=(const class_name__&)->class_name__& = delete;

#define DECLARE_MOVE_DEFAULT(class_name__)         \
  class_name__(class_name__&&) noexcept = default; \
  auto operator=(class_name__&&) noexcept -> class_name__& = default;

#define DECLARE_MOVE_DELETE(class_name__) \
  class_name__(class_name__&&) = delete;  \
  auto operator=(class_name__&&)->class_name__& = delete;

#define DECLARE_COPY_DEFAULT_CONSTEXPR(class_name__)                        \
  constexpr class_name__(const class_name__&) noexcept = default;           \
  constexpr auto operator=(const class_name__&) noexcept -> class_name__& = \
      default;

#define DECLARE_MOVE_DEFAULT_CONSTEXPR(class_name__)         \
  constexpr class_name__(class_name__&&) noexcept = default; \
  constexpr auto operator=(class_name__&&) noexcept -> class_name__& = default;

#define DERIVE_FINAL_WITH_CONSTRUCTORS(derived_name__, base_name__) \
  class derived_name__ final : public base_name__ {                 \
   public:                                                          \
    using base_name__::base_name__;                                 \
  };

#define DECLARE_USED(expression__) ((void)(sizeof(expression__)))
#define DECLARE_UNUSED(expression__) ((void)(sizeof(expression__)))

namespace lib::internal {

// Throws for a failed contract check. Out of line and cold, so each check site
// is only a compare and a call, which keeps small checked functions (such as
// CheckedPointer::operator->) cheap enough for every compiler to inline. The
// default argument captures the caller's location.
[[noreturn, gnu::cold, gnu::noinline]] auto do_contract_failure(
    const char* kind, const char* condition,
    std::source_location location = std::source_location::current()) -> void;

}  // namespace lib::internal

// The empty then-branch closes the `if`, so a trailing `else` at the call site
// binds to the caller's `if` instead of this one.
#define CHECK_CONTRACT__(condition__, kind__)                   \
  if (condition__) {                                            \
  } else [[unlikely]] {                                         \
    ::lib::internal::do_contract_failure(kind__, #condition__); \
  }

#define CHECK_PRECONDITION(precondition__) \
  CHECK_CONTRACT__(precondition__, "Precondition")
#define CHECK_POSTCONDITION(postcondition__) \
  CHECK_CONTRACT__(postcondition__, "Postcondition")
#define CHECK_INVARIANT(invariant__) CHECK_CONTRACT__(invariant__, "Invariant")
#define CHECK_UNREACHABLE() CHECK_CONTRACT__(false, "Unreachable")

// Propagating errors from std::expected, as a `?` operator would. The enclosing
// function must return a std::expected whose error type the error converts to.
//
//   RETURN_IF_UNEXPECTED(build_world(scenario, Out(world)));
//   ASSIGN_OR_RETURN(Entity asset, build_scenario(scenario, InOut(world)));
//   ASSIGN_OR_RETURN(asset_, build_scenario(scenario, InOut(world)));

// Returns the error of `expression__`, a std::expected, if it has one.
#define RETURN_IF_UNEXPECTED(expression__)                      \
  if (auto&& result__ = (expression__); result__.has_value()) { \
  } else [[unlikely]] {                                         \
    return std::unexpected(std::move(result__).error());        \
  }

// Returns the error of `expression__`, a std::expected, if it has one, and
// otherwise moves its value into `target__`, an existing variable or a new
// declaration such as `Entity asset`. It is several statements, so use it only
// where a statement can go, at most once per line.
#define ASSIGN_OR_RETURN(target__, expression__)                               \
  ASSIGN_OR_RETURN_INNER__(CONCATENATE__(result_on_line_, __LINE__), target__, \
                           expression__)

#define ASSIGN_OR_RETURN_INNER__(result__, target__, expression__) \
  auto&& result__ = (expression__);                                \
  if (!result__.has_value()) [[unlikely]] {                        \
    return std::unexpected(std::move(result__).error());           \
  }                                                                \
  target__ = *std::move(result__)

#define CONCATENATE__(first__, second__) CONCATENATE_INNER__(first__, second__)
#define CONCATENATE_INNER__(first__, second__) first__##second__

namespace lib {

inline auto allocate_static_increment() -> std::size_t {
  // Starts at 1, so 0 always means "none", such as a default StatusKind's
  // domain.
  static std::atomic<std::size_t> increment = 1;
  return increment++;
}

template <std::integral ToType, std::integral FromType>
auto narrow_cast(FromType from) -> ToType {
  ToType to = static_cast<ToType>(from);
  CHECK_PRECONDITION(std::cmp_equal(to, from))
  return to;
}

template <typename Type>
class CheckedPointer {
 public:
  DECLARE_COPY_DEFAULT(CheckedPointer);
  DECLARE_MOVE_DEFAULT(CheckedPointer);

  CheckedPointer() = default;
  ~CheckedPointer() = default;

  explicit CheckedPointer(Type& value) : arg_{&value} {}
  template <typename Derived>
    requires(std::is_base_of_v<Type, Derived> && !std::is_same_v<Type, Derived>)
  CheckedPointer(CheckedPointer<Derived> that) : arg_{that.arg_} {}

  explicit operator bool() const { return arg_; }

  auto operator*() const -> Type& {
    CHECK_PRECONDITION(arg_);
    return *arg_;
  }
  auto operator->() const -> Type* {
    CHECK_PRECONDITION(arg_);
    return arg_;
  }
  auto get() const -> Type* {
    CHECK_PRECONDITION(arg_);
    return arg_;
  }

 protected:
  // A null pointer upcasts to null, so an unused Out stays unused.
  template <typename>
  friend class CheckedPointer;

  Type* arg_ = nullptr;
};

struct Unused {};
inline constexpr Unused unused;

// Out argument:
// - May be explicitly initialized.
// - May be implicitly unused.
// - May upcast.
// - May be forwarded by copy/move.
// - May *not* manage lifetimes.
// - May *not* be converted to InOut.
template <typename Type>
class Out : public CheckedPointer<Type> {
  using BaseType = CheckedPointer<Type>;

 public:
  DECLARE_COPY_DEFAULT(Out);
  DECLARE_MOVE_DEFAULT(Out);

  Out() = delete;
  ~Out() = default;

  Out(Unused) {}
  explicit Out(Type& value) : BaseType{value} {}

  template <typename Derived>
    requires(std::is_base_of_v<Type, Derived> && !std::is_same_v<Type, Derived>)
  Out(Out<Derived> that) : BaseType{that} {}
};

// In-out argument:
// - Must be explicitly initialized.
// - May *not* be unused.
// - May upcast.
// - May be forwarded by copy/move.
// - May *not* manage lifetimes.
// - May be converted to Out.
template <typename Type>
class InOut : public Out<Type> {
  using BaseType = Out<Type>;

 public:
  DECLARE_COPY_DEFAULT(InOut);
  DECLARE_MOVE_DEFAULT(InOut);

  InOut() = delete;
  ~InOut() = default;

  explicit InOut(Type& value) : BaseType{value} { CHECK_INVARIANT(this->arg_); }

  template <typename Derived>
    requires(std::is_base_of_v<Type, Derived> && !std::is_same_v<Type, Derived>)
  InOut(InOut<Derived> that) : BaseType{that} {
    CHECK_INVARIANT(this->arg_);
  }
};

// Class member dependency:
// - Must be explicitly initialized.
// - May *not* be unused.
// - May upcast.
// - May be forwarded by copy/move.
// - May *not* manage lifetimes.
// - May be converted to InOut.
template <typename Type>
class Depend final : public InOut<Type> {
  using BaseType = InOut<Type>;

 public:
  using BaseType::BaseType;
};

template <typename T>
Out(T& value) -> Out<T>;
template <typename T>
InOut(T& value) -> InOut<T>;
template <typename T>
Depend(T& value) -> Depend<T>;

template <class... CallableTypes>
struct Overloaded : CallableTypes... {
  using CallableTypes::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

template <typename CallableType, typename... ArgumentTypes,
          typename ContinuationType>
auto invoke_with_continuation(ContinuationType&& continuation,
                              CallableType&& callable,
                              ArgumentTypes&&... arguments) -> void {
  using CallableResultType =
      std::invoke_result_t<CallableType, ArgumentTypes...>;

  if constexpr (std::is_invocable_v<ContinuationType, CallableResultType>) {
    std::invoke(
        continuation,
        std::invoke(callable, std::forward<ArgumentTypes>(arguments)...));
  } else {
    std::invoke(callable, std::forward<ArgumentTypes>(arguments)...);
    std::invoke(continuation);
  }
}

struct Empty final {};

// The readable form of a mangled type name, or the name itself if it cannot be
// demangled.
auto demangle(const std::string& name) -> std::string;

template <typename Type>
auto to_type_string() -> std::string {
  return demangle(typeid(Type).name());
}

template <typename Type>
auto to_type_string(Type&& object) -> std::string {
  return demangle(typeid(decltype(object)).name());
}

template <typename ObjectType>
auto dump_object_bytes(const ObjectType& object) -> void {
  std::print("** {}: ", to_type_string<ObjectType>());
  auto* begin_address = reinterpret_cast<const char*>(std::addressof(object));
  auto* end_address = reinterpret_cast<const char*>(std::addressof(object) + 1);
  for (; begin_address != end_address; ++begin_address) {
    std::print("{:02x} ", static_cast<unsigned char>(*begin_address));
  }
  std::print("\n");
}

inline auto stable_hash(std::string_view str) -> std::size_t {
  static const auto shuffle_ = [](std::uint64_t block) {
    return  // clang-format off
      ((block & 0xFFFF'0000'0000'0000) >> 16) |
      ((block & 0x0000'FFFF'0000'0000) >> 32) |
      ((block & 0x0000'0000'FFFF'0000) << 32) |
      ((block & 0x0000'0000'0000'FFFF) << 16);
    // clang-format on
  };

  static const auto diffuse_ = [](std::uint64_t block, std::uint64_t a,
                                  std::uint64_t b) {
    return (block * a) ^ (~block * b);
  };

  constexpr std::uint64_t m1 = 0xC2B2AE35C2B2AE35;
  constexpr std::uint64_t m2 = 0x42F0E1EBA9EA3693;
  constexpr std::uint64_t m3 = 0xC96C5795D7870F42;
  constexpr std::uint64_t block_size = 8;

  std::size_t i = 0;
  std::uint64_t block;
  std::uint64_t result = diffuse_(str.size(), m1, m2);

  // Bytes are read as unsigned and blocks assembled little-endian, so the
  // hash is the same whether char is signed and whatever the byte order.
  auto byte_at = [&](std::size_t at) {
    return static_cast<std::uint64_t>(static_cast<unsigned char>(str[at]));
  };
  for (; i + block_size <= str.size(); i += block_size) {
    block = 0;
    for (std::size_t b = 0; b < block_size; ++b) {
      block |= byte_at(i + b) << (8 * b);
    }
    result = shuffle_(result) ^ diffuse_(block, ~m2, m3);
  }

  for (; i < str.size(); i++) {
    result = shuffle_(result) ^ diffuse_(byte_at(i), m3, ~m1);
  }

  return diffuse_(result, m2, ~m3);
}

}  // namespace lib
