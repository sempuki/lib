#pragma once

#include <array>
#include <ostream>
#include <source_location>

#include "base/core.hpp"

namespace lib {

class StatusCode;
class StatusBase;
class StatusKind;
class StatusKindDomainInterface;
class StatusDomainInterface;
class StatusDetached;
class Status;

struct StatusConditionEntry final {
  std::string message;
};

struct StatusIncidentEntry final {
  std::string message;
  std::source_location location;
  int platform_error = 0;
};

//==============================================================================
//
class StatusCode final {
 public:
  constexpr explicit StatusCode(std::size_t domain,     //
                                std::size_t condition,  //
                                std::size_t incident)
      : bits_{(least_16_bits_as_uint64(domain) << DOMAIN_SHIFT) |
              (least_16_bits_as_uint64(condition) << CONDITION_SHIFT) |
              (least_16_bits_as_uint64(incident) << INCIDENT_SHIFT)} {
    CHECK_POSTCONDITION(std::cmp_equal(domain, domain_bits()));
    CHECK_POSTCONDITION(std::cmp_equal(condition, condition_bits()));
    CHECK_POSTCONDITION(std::cmp_equal(incident, incident_bits()));
  }

  DECLARE_COPY_DEFAULT_CONSTEXPR(StatusCode);
  DECLARE_MOVE_DEFAULT_CONSTEXPR(StatusCode);

  constexpr StatusCode() = default;
  constexpr ~StatusCode() = default;

  constexpr auto domain_bits() const noexcept -> std::size_t {
    return (bits_ & DOMAIN_MASK) >> DOMAIN_SHIFT;
  }

  constexpr auto set_domain_bits(std::size_t bits) noexcept -> void {
    bits_ &= ~DOMAIN_MASK;
    bits_ |= least_16_bits_as_uint64(bits) << DOMAIN_SHIFT;
    CHECK_POSTCONDITION(std::cmp_equal(bits, domain_bits()));
  }

  constexpr auto set_domain_bits(StatusCode code) noexcept -> void {
    bits_ &= ~DOMAIN_MASK;
    bits_ |= code.bits_ & DOMAIN_MASK;
  }

  constexpr auto condition_bits() const noexcept -> std::size_t {
    return (bits_ & CONDITION_MASK) >> CONDITION_SHIFT;
  }

  constexpr auto set_condition_bits(std::size_t bits) noexcept -> void {
    bits_ &= ~CONDITION_MASK;
    bits_ |= least_16_bits_as_uint64(bits) << CONDITION_SHIFT;
    CHECK_POSTCONDITION(std::cmp_equal(bits, condition_bits()));
  }

  constexpr auto set_condition_bits(StatusCode code) noexcept -> void {
    bits_ &= ~CONDITION_MASK;
    bits_ |= code.bits_ & CONDITION_MASK;
  }

  constexpr auto incident_bits() const noexcept -> std::size_t {
    return (bits_ & INCIDENT_MASK) >> INCIDENT_SHIFT;
  }

  constexpr auto set_incident_bits(std::size_t bits) noexcept -> void {
    bits_ &= ~INCIDENT_MASK;
    bits_ |= least_16_bits_as_uint64(bits) << INCIDENT_SHIFT;
    CHECK_POSTCONDITION(std::cmp_equal(bits, incident_bits()));
  }

  constexpr auto set_incident_bits(StatusCode code) noexcept -> void {
    bits_ &= ~INCIDENT_MASK;
    bits_ |= code.bits_ & INCIDENT_MASK;
  }

  constexpr auto is_same_kind(StatusCode that) const noexcept -> bool {
    return (bits_ & KIND_COMPARE_MASK) == (that.bits_ & KIND_COMPARE_MASK);
  }

  constexpr auto is_same_code(StatusCode that) const noexcept -> bool {
    return (bits_ & CODE_COMPARE_MASK) == (that.bits_ & CODE_COMPARE_MASK);
  }

 private:
  std::uint64_t bits_ = 0;

  constexpr static std::uint64_t DOMAIN_MASK = 0x0000'FFFF'0000'0000;
  constexpr static std::uint64_t DOMAIN_SIZE = 16;
  constexpr static std::uint64_t DOMAIN_SHIFT = 32;

  constexpr static std::uint64_t CONDITION_MASK = 0x0000'0000'FFFF'0000;
  constexpr static std::uint64_t CONDITION_SIZE = 16;
  constexpr static std::uint64_t CONDITION_SHIFT = 16;

  constexpr static std::uint64_t INCIDENT_MASK = 0x0000'0000'0000'FFFF;
  constexpr static std::uint64_t INCIDENT_SIZE = 16;
  constexpr static std::uint64_t INCIDENT_SHIFT = 0;

  constexpr static std::uint64_t KIND_COMPARE_MASK =
      DOMAIN_MASK | CONDITION_MASK;
  constexpr static std::uint64_t CODE_COMPARE_MASK =
      DOMAIN_MASK | CONDITION_MASK | INCIDENT_MASK;

  constexpr static auto least_16_bits_as_uint64(std::size_t bits)
      -> std::uint64_t {
    constexpr std::uint64_t SHIFT = 64 - 16;
    return ((static_cast<std::uint64_t>(bits) << SHIFT) >> SHIFT);
  }

  friend constexpr auto operator<(StatusCode lhs, StatusCode rhs) noexcept
      -> bool {
    return lhs.bits_ < rhs.bits_;
  }

  // In hex, leaving the stream's format as it was.
  friend auto operator<<(std::ostream& out, StatusCode self) noexcept
      -> std::ostream& {
    std::ios_base::fmtflags flags = out.flags();
    out << std::hex << self.bits_;
    out.flags(flags);
    return out;
  }
};

//==============================================================================
//
class StatusKindInspectInterface {
 public:
  constexpr virtual auto status_kind_message_of(
      StatusKind const*) const noexcept -> std::string_view = 0;
};

class StatusKindBuilderInterface {
 public:
  constexpr virtual auto make_status_kind_code(
      std::size_t condition_code) const noexcept -> StatusCode = 0;
};

class StatusBaseInspectInterface {
 public:
  virtual auto status_base_message_of(StatusBase const*) const noexcept
      -> std::string_view = 0;

  virtual auto status_base_location_of(StatusBase const*) const noexcept
      -> std::source_location = 0;

  virtual auto status_base_platform_error_of(StatusBase const*) const noexcept
      -> int = 0;
};

class StatusBaseBuilderInterface {
 public:
  virtual auto make_status_base_code(std::size_t condition_code,
                                     std::size_t incident_code) const noexcept
      -> StatusCode = 0;
};

class StatusConditionEquivalenceInterface {
 public:
  virtual auto has_equivalent_condition_of(StatusBase const*,
                                           StatusBase const*) const noexcept
      -> bool {
    return false;
  }
  virtual auto has_equivalent_condition_of(StatusBase const*,
                                           StatusKind const*) const noexcept
      -> bool {
    return false;
  }
};

class StatusDomainInterface : public StatusBaseInspectInterface,
                              public StatusKindInspectInterface,
                              public StatusConditionEquivalenceInterface {};

class StatusKindBuilderBase : public StatusKindBuilderInterface {
 public:
  DECLARE_COPY_DEFAULT_CONSTEXPR(StatusKindBuilderBase);
  DECLARE_MOVE_DEFAULT_CONSTEXPR(StatusKindBuilderBase);

  constexpr StatusKindBuilderBase() = default;
  constexpr ~StatusKindBuilderBase() = default;

  constexpr explicit StatusKindBuilderBase(std::size_t domain_code,
                                           std::string_view domain_name)
      : domain_code_{domain_code, 0U, 0U}, name_{domain_name} {}

  constexpr auto name() const noexcept -> std::string_view { return name_; }

  constexpr auto make_status_kind_code(
      std::size_t condition_code) const noexcept -> StatusCode final {
    return StatusCode{domain_code_.domain_bits(), condition_code, 0U};
  }

 private:
  StatusCode domain_code_;
  std::string_view name_;
};

class StatusDomainBuilderBase : public StatusKindBuilderInterface,
                                public StatusBaseBuilderInterface {
 public:
  DECLARE_COPY_DEFAULT(StatusDomainBuilderBase);
  DECLARE_MOVE_DEFAULT(StatusDomainBuilderBase);

  StatusDomainBuilderBase() = default;
  ~StatusDomainBuilderBase() = default;

  explicit StatusDomainBuilderBase(std::size_t domain_code,
                                   std::string_view domain_name)
      : domain_code_{domain_code, 0U, 0U}, name_{domain_name} {}

  auto name() const noexcept -> std::string_view { return name_; }

  auto make_status_kind_code(std::size_t condition_code) const noexcept
      -> StatusCode final {
    return StatusCode{domain_code_.domain_bits(),  //
                      condition_code,              //
                      0U};
  }

  auto make_status_base_code(std::size_t condition_code,
                             std::size_t incident_code) const noexcept
      -> StatusCode final {
    return StatusCode{domain_code_.domain_bits(),  //
                      condition_code,              //
                      incident_code};
  }

 private:
  StatusCode domain_code_;
  std::string name_;
};

namespace internal {
auto make_status(  //
    StatusCode, StatusDomainInterface const*) noexcept -> Status;
auto make_status_detached(  //
    StatusCode, StatusDomainInterface const*) noexcept -> StatusDetached;
constexpr auto make_status_kind(  //
    StatusCode, StatusKindInspectInterface const*) noexcept -> StatusKind;
}  // namespace internal

constexpr auto domain_code_of(StatusKind const*) noexcept -> std::size_t;
constexpr auto domain_code_of(StatusBase const*) noexcept -> std::size_t;
constexpr auto condition_code_of(StatusKind const*) noexcept -> std::size_t;
constexpr auto condition_code_of(StatusBase const*) noexcept -> std::size_t;
constexpr auto incident_code_of(StatusBase const*) noexcept -> std::size_t;

//==============================================================================
//
class StatusKind final {
 public:
  DECLARE_COPY_DEFAULT_CONSTEXPR(StatusKind);
  DECLARE_MOVE_DEFAULT_CONSTEXPR(StatusKind);

  constexpr StatusKind() = default;
  constexpr ~StatusKind() = default;

  constexpr auto message() const noexcept -> std::string_view {
    return domain_ ? domain_->status_kind_message_of(this) : std::string_view{};
  }

  constexpr auto operator==(StatusKind that) const noexcept -> bool {
    return kind_code_.is_same_kind(that.kind_code_);
  }

 private:
  constexpr explicit StatusKind(StatusCode code,
                                StatusKindInspectInterface const* domain)
      : kind_code_{code}, domain_{domain} {}

  StatusCode kind_code_;
  StatusKindInspectInterface const* domain_ = nullptr;

  friend class StatusBase;

  friend constexpr auto internal::make_status_kind(
      StatusCode, StatusKindInspectInterface const*) noexcept -> StatusKind;

  friend constexpr auto domain_code_of(StatusKind const*) noexcept
      -> std::size_t;
  friend constexpr auto condition_code_of(StatusKind const*) noexcept
      -> std::size_t;

  friend constexpr auto operator!=(StatusKind a, StatusKind b) noexcept
      -> bool {
    return !a.operator==(b);
  }

  friend constexpr auto operator<(StatusKind lhs, StatusKind rhs) noexcept
      -> bool {
    return lhs.kind_code_ < rhs.kind_code_;
  }

  friend auto operator<<(std::ostream& out, StatusKind self) noexcept
      -> std::ostream& {
    out << self.message();
    return out;
  }
};

//==============================================================================
//
class StatusBase {
 public:
  DECLARE_COPY_DEFAULT(StatusBase);
  DECLARE_MOVE_DEFAULT(StatusBase);

  StatusBase() = delete;
  ~StatusBase() = default;

  auto operator==(StatusBase that) const noexcept -> bool {
    return status_code_.is_same_code(that.status_code_);
  }

  auto operator==(StatusKind that) const noexcept -> bool {
    return status_code_.is_same_kind(that.kind_code_);
  }

 protected:
  explicit StatusBase(StatusCode code) : status_code_{code} {}

  StatusCode status_code_;

  friend constexpr auto domain_code_of(StatusBase const*) noexcept
      -> std::size_t;
  friend constexpr auto condition_code_of(StatusBase const*) noexcept
      -> std::size_t;
  friend constexpr auto incident_code_of(StatusBase const*) noexcept
      -> std::size_t;

  friend auto operator==(StatusKind lhs, StatusBase rhs) noexcept -> bool {
    return rhs.operator==(lhs);
  }

  friend auto operator!=(StatusBase lhs, StatusKind rhs) noexcept -> bool {
    return !lhs.operator==(rhs);
  }

  friend auto operator!=(StatusKind lhs, StatusBase rhs) noexcept -> bool {
    return !rhs.operator==(lhs);
  }

  friend auto operator<(StatusBase lhs, StatusBase rhs) noexcept -> bool {
    return lhs.status_code_ < rhs.status_code_;
  }
};

//==============================================================================
//
class StatusDetached final : public StatusBase {
 public:
  DECLARE_COPY_DEFAULT(StatusDetached);
  DECLARE_MOVE_DEFAULT(StatusDetached);

  StatusDetached() = delete;
  ~StatusDetached() = default;

  auto message() const noexcept -> std::string_view { return entry_.message; }
  auto location() const noexcept -> std::source_location {
    return entry_.location;
  }
  auto platform_error() const noexcept -> int { return entry_.platform_error; }

 private:
  explicit StatusDetached(StatusCode code, StatusDomainInterface const* domain)
      : StatusBase{code} {
    entry_.message = domain->status_base_message_of(this);
    entry_.location = domain->status_base_location_of(this);
    entry_.platform_error = domain->status_base_platform_error_of(this);
  }

  StatusIncidentEntry entry_;

  friend auto internal::make_status_detached(  //
      StatusCode, StatusDomainInterface const*) noexcept -> StatusDetached;

  friend auto operator<<(std::ostream& out, StatusDetached self) noexcept
      -> std::ostream& {
    out << self.message();
    return out;
  }
};

//==============================================================================
//
class Status final : public StatusBase {
 public:
  DECLARE_COPY_DEFAULT(Status);
  DECLARE_MOVE_DEFAULT(Status);

  Status() = delete;
  ~Status() = default;

  auto message() const noexcept -> std::string_view {
    return domain_->status_base_message_of(this);
  }
  auto location() const noexcept -> std::source_location {
    return domain_->status_base_location_of(this);
  }

  auto platform_error() const noexcept -> int {
    return domain_->status_base_platform_error_of(this);
  }

  auto kind() const noexcept -> StatusKind {
    return internal::make_status_kind(status_code_, domain_);
  }

  auto detach_copy() const noexcept -> StatusDetached {
    return internal::make_status_detached(status_code_, domain_);
  }

  // Compares conditions, not incidents: two raises of one condition match.
  auto has_equivalent_condition_as(Status that) const noexcept -> bool {
    return kind() == that.kind() ||
           domain_->has_equivalent_condition_of(this, &that);
  }

  auto has_equivalent_condition_as(StatusKind that) const noexcept -> bool {
    return *this == that || domain_->has_equivalent_condition_of(this, &that);
  }

 private:
  explicit Status(StatusCode code, StatusDomainInterface const* domain)
      : StatusBase{code}, domain_{domain} {}

  StatusDomainInterface const* domain_ = nullptr;

  friend auto internal::make_status(  //
      StatusCode, const StatusDomainInterface*) noexcept -> Status;

  friend auto operator<<(std::ostream& out, Status self) noexcept
      -> std::ostream& {
    out << self.message();
    return out;
  }
};

//------------------------------------------------------------------------------
//
namespace internal {
inline auto make_status(  //
    StatusCode code, const StatusDomainInterface* domain) noexcept -> Status {
  return Status{code, domain};
}

inline auto make_status_detached(  //
    StatusCode code, const StatusDomainInterface* domain) noexcept
    -> StatusDetached {
  return StatusDetached{code, domain};
}

inline constexpr auto make_status_kind(  //
    StatusCode code, StatusKindInspectInterface const* domain) noexcept
    -> StatusKind {
  return StatusKind{code, domain};
}
}  // namespace internal

inline constexpr auto domain_code_of(StatusKind const* self) noexcept
    -> std::size_t {
  return self->kind_code_.domain_bits();
}

inline constexpr auto domain_code_of(StatusBase const* self) noexcept
    -> std::size_t {
  return self->status_code_.domain_bits();
}

inline constexpr auto condition_code_of(StatusKind const* self) noexcept
    -> std::size_t {
  return self->kind_code_.condition_bits();
}

inline constexpr auto condition_code_of(StatusBase const* self) noexcept
    -> std::size_t {
  return self->status_code_.condition_bits();
}

inline constexpr auto incident_code_of(StatusBase const* self) noexcept
    -> std::size_t {
  return self->status_code_.incident_bits();
}

//==============================================================================
//
template <typename ConditionEnumType, std::size_t ConditionCount>
class EnumStatusKindConditionMixin {
 public:
  constexpr auto watch_kind(ConditionEnumType condition) const noexcept
      -> StatusKind {
    return handle_watch_kind(condition);
  }

 protected:
  virtual auto handle_watch_kind(ConditionEnumType condition) const noexcept
      -> StatusKind = 0;

  auto do_watch_kind(                             //
      ConditionEnumType condition,                //
      StatusKindBuilderInterface const* builder,  //
      StatusKindInspectInterface const* domain) const noexcept -> StatusKind {
    auto condition_code = static_cast<std::size_t>(condition);
    return internal::make_status_kind(
        builder->make_status_kind_code(condition_code), domain);
  }

  constexpr auto status_kind_message_of(StatusKind const* kind) const noexcept
      -> std::string_view {
    return conditions_[condition_code_of(kind)].message;
  }

  static const std::array<StatusConditionEntry, ConditionCount> conditions_;
};

//==============================================================================
// NOTE: To avoid the cost of reference counting each return code's incident
// location and message, the domain is limited to tracking `IncidentCountMax`
// number of concurrent incidents, which are stored in a simple array-backed
// ring buffer. If `Status` incident information must be accurately stored for
// longer than the buffer might be expected to overlap, then `StatusDetached`
// should be used instead, which keeps its own local copy.
//
template <typename ConditionEnumType,  //
          std::size_t IncidentCountMax>
class EnumStatusIncidentMixin {
 public:
  auto raise_status(                //
      ConditionEnumType condition,  //
      std::string message = {}) noexcept -> Status {
    std::source_location location{};  // Empty.
    return handle_raise_incident(condition, StatusIncidentEntry{
                                                .message = std::move(message),
                                                .location = std::move(location),
                                                .platform_error = 0,
                                            });
  }

  auto raise_status_here(           //
      ConditionEnumType condition,  //
      std::string message = {},     //
      std::source_location location = std::source_location::current()) noexcept
      -> Status {
    return handle_raise_incident(condition, StatusIncidentEntry{
                                                .message = std::move(message),
                                                .location = std::move(location),
                                                .platform_error = 0,
                                            });
  }

  auto raise_error(                 //
      ConditionEnumType condition,  //
      int platform_error,           //
      std::string message = {}) noexcept -> Status {
    std::source_location location{};  // Empty.
    return handle_raise_incident(condition,
                                 StatusIncidentEntry{
                                     .message = std::move(message),
                                     .location = std::move(location),
                                     .platform_error = platform_error,
                                 });
  }

  auto raise_error_here(            //
      ConditionEnumType condition,  //
      int platform_error,           //
      std::string message = {},
      std::source_location location = std::source_location::current()) noexcept
      -> Status {
    return handle_raise_incident(condition,
                                 StatusIncidentEntry{
                                     .message = std::move(message),
                                     .location = std::move(location),
                                     .platform_error = platform_error,
                                 });
  }

 protected:
  virtual auto handle_raise_incident(ConditionEnumType condition,
                                     StatusIncidentEntry entry) noexcept
      -> Status = 0;

  auto do_raise_incident(                         //
      ConditionEnumType condition,                //
      StatusIncidentEntry entry,                  //
      StatusBaseBuilderInterface const* builder,  //
      StatusDomainInterface const* domain) noexcept -> Status {
    std::size_t current = next_incident_++;
    next_incident_ %= incidents_.size();
    incidents_[current] = std::move(entry);

    auto condition_code = static_cast<std::size_t>(condition);
    return internal::make_status(
        builder->make_status_base_code(condition_code, current), domain);
  }

  auto status_base_message_of(StatusBase const* status) const noexcept
      -> std::string_view {
    return incidents_[incident_code_of(status)].message;
  }

  auto status_base_location_of(StatusBase const* status) const noexcept
      -> std::source_location {
    return incidents_[incident_code_of(status)].location;
  }

  auto status_base_platform_error_of(StatusBase const* status) const noexcept
      -> int {
    return incidents_[incident_code_of(status)].platform_error;
  }

 private:
  std::size_t next_incident_ = 0;
  std::array<StatusIncidentEntry, IncidentCountMax> incidents_;
};

//==============================================================================
// Kind-only enum-based domain type.
//
template <typename ConditionEnumType,  //
          std::size_t ConditionCount>
class EnumStatusKindDomain
    : public EnumStatusKindConditionMixin<ConditionEnumType, ConditionCount>,
      public StatusKindInspectInterface,
      public StatusKindBuilderBase {
 public:
  using StatusKindBuilderBase::StatusKindBuilderBase;

  constexpr auto status_kind_message_of(StatusKind const* kind) const noexcept
      -> std::string_view override {
    return EnumStatusKindConditionMixin<
        ConditionEnumType, ConditionCount>::status_kind_message_of(kind);
  }

 private:
  auto handle_watch_kind(ConditionEnumType condition) const noexcept
      -> StatusKind final {
    return EnumStatusKindConditionMixin<ConditionEnumType,
                                        ConditionCount>::do_watch_kind(  //
        condition, this, this);
  }
};

constexpr std::size_t DEFAULT_INCIDENT_COUNT = 16;

//==============================================================================
// Main enum-based domain type.
//
template <typename ConditionEnumType,  //
          std::size_t ConditionCount,  //
          std::size_t IncidentCountMax = DEFAULT_INCIDENT_COUNT>
class EnumStatusDomain
    : public StatusDomainBuilderBase,
      public StatusDomainInterface,
      public EnumStatusKindConditionMixin<ConditionEnumType, ConditionCount>,
      public EnumStatusIncidentMixin<ConditionEnumType, IncidentCountMax> {
  // StatusCode stores condition and incident codes in 16 bits each.
  static_assert(ConditionCount <= (std::size_t{1} << 16));
  static_assert(IncidentCountMax > 0 &&
                IncidentCountMax <= (std::size_t{1} << 16));

 public:
  using StatusDomainBuilderBase::StatusDomainBuilderBase;

  constexpr auto status_kind_message_of(StatusKind const* kind) const noexcept
      -> std::string_view override {
    return EnumStatusKindConditionMixin<
        ConditionEnumType, ConditionCount>::status_kind_message_of(kind);
  }

  auto status_base_message_of(StatusBase const* status) const noexcept
      -> std::string_view override {
    return EnumStatusIncidentMixin<
        ConditionEnumType, IncidentCountMax>::status_base_message_of(status);
  }

  auto status_base_location_of(StatusBase const* status) const noexcept
      -> std::source_location override {
    return EnumStatusIncidentMixin<
        ConditionEnumType, IncidentCountMax>::status_base_location_of(status);
  }

  auto status_base_platform_error_of(StatusBase const* status) const noexcept
      -> int override {
    return EnumStatusIncidentMixin<ConditionEnumType, IncidentCountMax>::
        status_base_platform_error_of(status);
  }

 private:
  auto handle_watch_kind(ConditionEnumType condition) const noexcept
      -> StatusKind final {
    return EnumStatusKindConditionMixin<ConditionEnumType,
                                        ConditionCount>::do_watch_kind(  //
        condition, this, this);
  }

  auto handle_raise_incident(ConditionEnumType condition,
                             StatusIncidentEntry entry) noexcept
      -> Status final {
    return EnumStatusIncidentMixin<ConditionEnumType, IncidentCountMax>::
        do_raise_incident(condition, std::move(entry), this, this);
  }
};

//==============================================================================
//
template <typename ConditionEnumType,  //
          std::size_t ConditionCount,  //
          std::size_t IncidentCountMax = DEFAULT_INCIDENT_COUNT>
auto static_enum_status_domain() noexcept
    -> EnumStatusDomain<ConditionEnumType, ConditionCount, IncidentCountMax>& {
  static std::size_t domain_code = allocate_static_increment();
  static EnumStatusDomain<ConditionEnumType, ConditionCount, IncidentCountMax>
      domain{domain_code,
             std::format("static_enum_status_domain_{}", domain_code)};
  return domain;
}

// A domain whose identity is shared by every thread, and whose incident
// storage (each raise's message and location) is each thread's own, so raising
// needs no lock. Statuses of one condition compare equal whichever thread
// raised them. A Status reads its message from the raising thread's storage,
// so keep a detach_copy() of any Status that must outlive that thread.
template <typename ConditionEnumType,  //
          std::size_t ConditionCount>
auto thread_local_enum_status_domain() noexcept
    -> EnumStatusDomain<ConditionEnumType, ConditionCount, 1u>& {
  static const std::size_t domain_code = allocate_static_increment();
  thread_local EnumStatusDomain<ConditionEnumType, ConditionCount, 1u> domain{
      domain_code,
      std::format("thread_local_enum_status_domain_{}", domain_code)};
  return domain;
}

//------------------------------------------------------------------------------
//
template <typename ConditionEnumType>
auto raise(ConditionEnumType condition, std::string message = {}) noexcept
    -> Status {
  return static_enum_status_domain<ConditionEnumType,
                                   static_cast<std::size_t>(
                                       ConditionEnumType::COUNT)>()
      .raise_status(condition, std::move(message));
}

template <typename ConditionEnumType>
auto raise_here(ConditionEnumType condition, std::string message = {},
                std::source_location location =
                    std::source_location::current()) noexcept -> Status {
  return static_enum_status_domain<ConditionEnumType,
                                   static_cast<std::size_t>(
                                       ConditionEnumType::COUNT)>()
      .raise_status_here(condition, std::move(message), std::move(location));
}

template <typename ConditionEnumType>
auto watch(ConditionEnumType condition) noexcept -> StatusKind {
  return static_enum_status_domain<ConditionEnumType,
                                   static_cast<std::size_t>(
                                       ConditionEnumType::COUNT)>()
      .watch_kind(condition);
}

//------------------------------------------------------------------------------
//
template <typename ConditionEnumType>
auto raise_thread_local(ConditionEnumType condition,
                        std::string message = {}) noexcept -> Status {
  return thread_local_enum_status_domain<ConditionEnumType,
                                         static_cast<std::size_t>(
                                             ConditionEnumType::COUNT)>()
      .raise_status(condition, std::move(message));
}

template <typename ConditionEnumType>
auto raise_here_thread_local(
    ConditionEnumType condition, std::string message = {},
    std::source_location location = std::source_location::current()) noexcept
    -> Status {
  return thread_local_enum_status_domain<ConditionEnumType,
                                         static_cast<std::size_t>(
                                             ConditionEnumType::COUNT)>()
      .raise_status_here(condition, std::move(message), std::move(location));
}

template <typename ConditionEnumType>
auto watch_thread_local(ConditionEnumType condition) noexcept -> StatusKind {
  return thread_local_enum_status_domain<ConditionEnumType,
                                         static_cast<std::size_t>(
                                             ConditionEnumType::COUNT)>()
      .watch_kind(condition);
}

}  // namespace lib
