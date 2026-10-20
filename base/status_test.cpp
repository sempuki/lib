#include "base/status.hpp"

#include <functional>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "base/testing.hpp"

namespace lib {

constexpr auto StatusCodeShouldHaveSameBits() -> bool {
  StatusCode code;
  code.set_incident_bits(4u);
  code.set_condition_bits(5u);
  code.set_domain_bits(6u);

  return code.incident_bits() == 4u &&   //
         code.condition_bits() == 5u &&  //
         code.domain_bits() == 6u;
}

TEST_CASE("StatusCode") {
  StatusCode code;

  SECTION("ShouldDefaultToZero") {
    // Postconditions.
    REQUIRE(code.domain_bits() == 0u);
    REQUIRE(code.condition_bits() == 0u);
    REQUIRE(code.incident_bits() == 0u);
  }

  SECTION("ShouldConstructWithIncidentConditionDomainBits") {
    // Under Test.
    StatusCode other{4u, 5u, 6u};

    // Postconditions.
    REQUIRE(other.domain_bits() == 4u);
    REQUIRE(other.condition_bits() == 5u);
    REQUIRE(other.incident_bits() == 6u);
  }

  SECTION("ShouldHaveSameDomainBitsWhenSet") {
    // Under Test.
    code.set_domain_bits(5u);

    // Postconditions.
    REQUIRE(code.domain_bits() == 5u);
  }

  SECTION("ShouldHaveSameConditionBitsWhenSet") {
    // Under Test.
    code.set_condition_bits(5u);

    // Postconditions.
    REQUIRE(code.condition_bits() == 5u);
  }

  SECTION("ShouldHaveSameIncidentBitsWhenSet") {
    // Under Test.
    code.set_incident_bits(5u);

    // Postconditions.
    REQUIRE(code.incident_bits() == 5u);
  }

  StatusCode other;
  code.set_incident_bits(4u);
  code.set_condition_bits(5u);
  code.set_domain_bits(6u);

  SECTION("ShouldBeSameKindWithSameConditionAndDomain") {
    // Under Test.
    other.set_incident_bits(7u);
    other.set_condition_bits(5u);
    other.set_domain_bits(6u);

    // Postconditions.
    REQUIRE(code.is_same_kind(other));
  }

  SECTION("ShouldNotBeSameKindWithDifferentCondition") {
    // Under Test.
    other.set_incident_bits(4u);
    other.set_condition_bits(7u);
    other.set_domain_bits(6u);

    // Postconditions.
    REQUIRE(!code.is_same_kind(other));
  }

  SECTION("ShouldNotBeSameKindWithDifferentDomain") {
    // Under Test.
    other.set_incident_bits(4u);
    other.set_condition_bits(5u);
    other.set_domain_bits(7u);

    // Postconditions.
    REQUIRE(!code.is_same_kind(other));
  }

  SECTION("ShouldBeSameCodeWithSameIncidentConditionDomain") {
    // Under Test.
    other.set_incident_bits(4u);
    other.set_condition_bits(5u);
    other.set_domain_bits(6u);

    // Postconditions.
    REQUIRE(code.is_same_code(other));
  }

  SECTION("ShouldNotBeSameCodeWithDifferentIncident") {
    // Under Test.
    other.set_incident_bits(7u);
    other.set_condition_bits(5u);
    other.set_domain_bits(6u);

    // Postconditions.
    REQUIRE(!code.is_same_code(other));
  }

  SECTION("ShouldNotBeSameCodeWithDifferentCondition") {
    // Under Test.
    other.set_incident_bits(4u);
    other.set_condition_bits(7u);
    other.set_domain_bits(6u);

    // Postconditions.
    REQUIRE(!code.is_same_code(other));
  }

  SECTION("ShouldNotBeSameCodeWithDifferentDomain") {
    // Under Test.
    other.set_incident_bits(4u);
    other.set_condition_bits(5u);
    other.set_domain_bits(7u);

    // Postconditions.
    REQUIRE(!code.is_same_code(other));
  }

  SECTION("ShouldCopyOnlyDomainBitsGivenOtherCode") {
    // Under Test.
    code.set_domain_bits(StatusCode{7u, 8u, 9u});

    // Postconditions.
    REQUIRE(code.domain_bits() == 7u);
    REQUIRE(code.condition_bits() == 5u);
    REQUIRE(code.incident_bits() == 4u);
  }

  SECTION("ShouldCopyOnlyConditionBitsGivenOtherCode") {
    // Under Test.
    code.set_condition_bits(StatusCode{7u, 8u, 9u});

    // Postconditions.
    REQUIRE(code.domain_bits() == 6u);
    REQUIRE(code.condition_bits() == 8u);
    REQUIRE(code.incident_bits() == 4u);
  }

  SECTION("ShouldCopyOnlyIncidentBitsGivenOtherCode") {
    // Under Test.
    code.set_incident_bits(StatusCode{7u, 8u, 9u});

    // Postconditions.
    REQUIRE(code.domain_bits() == 6u);
    REQUIRE(code.condition_bits() == 5u);
    REQUIRE(code.incident_bits() == 9u);
  }

  SECTION("ShouldKeepEveryBitGivenSixteenBitFields") {
    // Under Test.
    StatusCode widest{0xFFFFu, 0xFFFFu, 0xFFFFu};

    // Postconditions.
    REQUIRE(widest.domain_bits() == 0xFFFFu);
    REQUIRE(widest.condition_bits() == 0xFFFFu);
    REQUIRE(widest.incident_bits() == 0xFFFFu);
  }

  SECTION("ShouldThrowGivenFieldWiderThanSixteenBits") {
    // Preconditions.
    constexpr std::size_t too_wide = std::size_t{1} << 16;

    // Postconditions.
    REQUIRE_THROWS_AS((StatusCode{too_wide, 0u, 0u}), std::logic_error);
    REQUIRE_THROWS_AS((StatusCode{0u, too_wide, 0u}), std::logic_error);
    REQUIRE_THROWS_AS((StatusCode{0u, 0u, too_wide}), std::logic_error);
  }

  SECTION("ShouldOrderByDomainThenConditionThenIncident") {
    // Postconditions.
    REQUIRE(StatusCode{0u, 0xFFFFu, 0xFFFFu} < StatusCode{1u, 0u, 0u});
    REQUIRE(StatusCode{1u, 0u, 0xFFFFu} < StatusCode{1u, 1u, 0u});
    REQUIRE(StatusCode{1u, 1u, 0u} < StatusCode{1u, 1u, 1u});
    REQUIRE_FALSE(StatusCode{1u, 1u, 1u} < StatusCode{1u, 1u, 1u});
  }

  SECTION("ShouldBeConstexprBits") {
    // Under Test.
    constexpr bool pass = StatusCodeShouldHaveSameBits();

    // Postconditions.
    REQUIRE(pass);
  }

  SECTION("ShouldWriteHexGivenOutputOperator") {
    // Preconditions.
    std::ostringstream out;

    // Under Test.
    out << StatusCode{1u, 2u, 3u};

    // Postconditions.
    REQUIRE(out.str() == "100020003");
  }

  SECTION("ShouldRestoreStreamFlagsGivenOutputOperator") {
    // Preconditions.
    std::ostringstream out;

    // Under Test.
    out << StatusCode{1u, 2u, 3u} << ' ' << 255;

    // Postconditions.
    REQUIRE(out.str().ends_with(" 255"));
  }
}

enum class Quark { UP, DOWN, TOP, BOTTOM, STRANGE, CHARM, COUNT };
constexpr std::size_t QUARK_CONDITION_COUNT =
    static_cast<std::size_t>(Quark::COUNT);

template <>
const std::array<StatusConditionEntry, QUARK_CONDITION_COUNT>
    EnumStatusKindConditionMixin<Quark, QUARK_CONDITION_COUNT>::conditions_ = {
        StatusConditionEntry{"UP"},       //
        StatusConditionEntry{"DOWN"},     //
        StatusConditionEntry{"TOP"},      //
        StatusConditionEntry{"BOTTOM"},   //
        StatusConditionEntry{"STRANGE"},  //
        StatusConditionEntry{"CHARM"},    //
};

class QuarkStatusDomain final
    : public EnumStatusDomain<Quark, QUARK_CONDITION_COUNT> {
  using BaseType = EnumStatusDomain<Quark, QUARK_CONDITION_COUNT>;

 public:
  using BaseType::BaseType;

  auto status_kind_message_of(const StatusKind* kind) const noexcept
      -> std::string_view override {
    message_of_condition++;
    return BaseType::status_kind_message_of(kind);
  }

  auto status_base_message_of(const StatusBase* status) const noexcept
      -> std::string_view override {
    message_of_incident++;
    return BaseType::status_base_message_of(status);
  }

  auto status_base_location_of(const StatusBase* status) const noexcept
      -> std::source_location override {
    location_of_incident++;
    return BaseType::status_base_location_of(status);
  }

  mutable std::size_t message_of_condition = 0;
  mutable std::size_t message_of_incident = 0;
  mutable std::size_t location_of_incident = 0;
};

class QuarkKindDomain final
    : public EnumStatusKindDomain<Quark, QUARK_CONDITION_COUNT> {
  using BaseType = EnumStatusKindDomain<Quark, QUARK_CONDITION_COUNT>;

 public:
  using BaseType::BaseType;
};

// Declares UP and DOWN equivalent conditions.
class EquivalentQuarkDomain final
    : public EnumStatusDomain<Quark, QUARK_CONDITION_COUNT> {
  using BaseType = EnumStatusDomain<Quark, QUARK_CONDITION_COUNT>;

 public:
  using BaseType::BaseType;

  auto has_equivalent_condition_of(StatusBase const* lhs,
                                   StatusBase const* rhs) const noexcept
      -> bool override {
    return is_up_or_down(condition_code_of(lhs)) &&
           is_up_or_down(condition_code_of(rhs));
  }

  auto has_equivalent_condition_of(StatusBase const* lhs,
                                   StatusKind const* rhs) const noexcept
      -> bool override {
    return is_up_or_down(condition_code_of(lhs)) &&
           is_up_or_down(condition_code_of(rhs));
  }

 private:
  static auto is_up_or_down(std::size_t condition_code) noexcept -> bool {
    return condition_code == static_cast<std::size_t>(Quark::UP) ||
           condition_code == static_cast<std::size_t>(Quark::DOWN);
  }
};

constexpr std::size_t SMALL_INCIDENT_COUNT = 4;

QuarkStatusDomain domain{42u, "quark"};
EquivalentQuarkDomain equivalent_domain{43u, "equivalent"};
EnumStatusDomain<Quark, QUARK_CONDITION_COUNT, SMALL_INCIDENT_COUNT>
    small_domain{44u, "small"};
std::source_location location0 = std::source_location::current();
std::source_location location1 = std::source_location::current();
std::string message0 = "mark";
std::string message1 = "bark";
constexpr int platform_error0 = -50;
constexpr int platform_error1 = -42;

TEST_CASE("EnumStatusDomain") {
  SECTION("ShouldNotFetchStatusLocationByDefaultWhenRaiseStatus") {
    // Preconditions.
    const std::size_t before = domain.location_of_incident;

    // Under Test.
    [[maybe_unused]] Status status = domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(domain.location_of_incident == before);
  }

  SECTION("ShouldNotFetchStatusMessageByDefaultWhenRaiseStatus") {
    // Preconditions.
    const std::size_t before = domain.message_of_incident;

    // Under Test.
    [[maybe_unused]] Status status = domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(domain.message_of_incident == before);
  }

  SECTION("ShouldFetchStatusMessageOnDemandWhenRaiseStatus") {
    // Preconditions.
    Status status = domain.raise_status(Quark::UP, message0);

    // Under Test.
    std::string_view message = status.message();

    // Postconditions.
    REQUIRE(domain.message_of_incident > 0u);
    REQUIRE(message.size() > 0u);
  }

  SECTION("ShouldFetchStatusLocationOnDemandWhenRaiseStatusHere") {
    // Preconditions.
    Status status = domain.raise_status_here(Quark::UP);

    // Under Test.
    std::source_location location = status.location();

    // Postconditions.
    REQUIRE(domain.location_of_incident > 0u);
    REQUIRE(location.line() > 0u);
  }

  SECTION("ShouldHaveNameGivenConstruction") {
    // Postconditions.
    REQUIRE(domain.name() == "quark");
    REQUIRE(small_domain.name() == "small");
  }

  SECTION("ShouldEncodeDomainAndConditionGivenRaiseStatus") {
    // Under Test.
    Status status = domain.raise_status(Quark::DOWN);

    // Postconditions.
    REQUIRE(domain_code_of(&status) == 42u);
    REQUIRE(condition_code_of(&status) ==
            static_cast<std::size_t>(Quark::DOWN));
    REQUIRE(incident_code_of(&status) < DEFAULT_INCIDENT_COUNT);
  }

  SECTION("ShouldGiveEachRaiseItsOwnIncidentGivenSameCondition") {
    // Under Test.
    Status first = domain.raise_status(Quark::UP);
    Status second = domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(incident_code_of(&first) != incident_code_of(&second));
    REQUIRE(first != second);
  }

  SECTION("ShouldReuseIncidentSlotsGivenMoreRaisesThanIncidentCountMax") {
    // Preconditions.
    std::vector<Status> raised;
    for (std::size_t i = 0; i <= SMALL_INCIDENT_COUNT; ++i) {
      raised.push_back(small_domain.raise_status(Quark::UP, std::to_string(i)));
    }

    // Under Test.
    Status& first = raised.front();
    Status& wrapped = raised.back();

    // Postconditions.
    // The last raise took the first raise's slot, so the first Status now
    // reads the last message and compares equal to the last Status.
    REQUIRE(incident_code_of(&wrapped) == incident_code_of(&first));
    REQUIRE(first.message() == std::to_string(SMALL_INCIDENT_COUNT));
    REQUIRE(wrapped == first);
    REQUIRE(raised[1].message() == "1");
  }

  SECTION("ShouldOverwriteOnlyTheOldestSlotGivenOneRaiseOverCapacity") {
    // Preconditions.
    std::vector<Status> raised;
    for (std::size_t i = 0; i < SMALL_INCIDENT_COUNT; ++i) {
      raised.push_back(small_domain.raise_status(Quark::UP, std::to_string(i)));
    }

    // Under Test.
    Status extra = small_domain.raise_status(Quark::UP, "extra");

    // Postconditions.
    REQUIRE(raised[0].message() == "extra");
    for (std::size_t i = 1; i < SMALL_INCIDENT_COUNT; ++i) {
      REQUIRE(raised[i].message() == std::to_string(i));
    }
    REQUIRE(extra.message() == "extra");
  }
}

TEST_CASE("Status") {
  SECTION("ShouldHaveCurrentLocationAsRaiseStatusHere") {
    // Preconditions.
    auto raised_location = std::source_location::current();
    Status status = domain.raise_status_here(Quark::UP);

    // Under Test.
    std::source_location location = status.location();

    // Postconditions.
    REQUIRE(location.line() - 1 == raised_location.line());
  }

  // Precondition.
  Status status = domain.raise_status_here(Quark::UP, message0, location0);

  SECTION("ShouldHaveSameMessageAsWhenRaiseStatusHere") {
    // Under Test.
    std::string_view status_message = status.message();

    // Postconditions.
    REQUIRE(status_message == message0);
    REQUIRE(status_message != message1);
  }

  SECTION("ShouldHaveSameLocationAsWhenRaiseStatusHere") {
    // Under Test.
    std::source_location location = status.location();

    // Postconditions.
    REQUIRE(location.line() == location0.line());
    REQUIRE(location.line() != location1.line());
  }

  // Precondition.
  Status error_status = domain.raise_error_here(  //
      Quark::UP, platform_error0, message0, location0);

  SECTION("ShouldHaveSameMessageAsWhenRaiseErrorHere") {
    // Under Test.
    std::string_view status_message = error_status.message();

    // Postconditions.
    REQUIRE(status_message == message0);
    REQUIRE(status_message != message1);
  }

  SECTION("ShouldHaveSameLocationAsWhenRaiseErrorHere") {
    // Under Test.
    std::source_location location = error_status.location();

    // Postconditions.
    REQUIRE(location.line() == location0.line());
    REQUIRE(location.line() != location1.line());
  }

  SECTION("ShouldHaveSamePlatformCodeAsWhenRaiseErrorHere") {
    // Under Test.
    int platform_error = error_status.platform_error();

    // Postconditions.
    REQUIRE(platform_error == platform_error0);
    REQUIRE(platform_error != platform_error1);
  }

  SECTION("ShouldHaveEmptyMessageAndLocationGivenRaiseStatusWithoutThem") {
    // Under Test.
    Status bare = domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(bare.message().empty());
    REQUIRE(bare.location().line() == 0u);
    REQUIRE(bare.platform_error() == 0);
  }

  SECTION("ShouldHaveZeroPlatformErrorGivenRaiseStatus") {
    // Postconditions.
    REQUIRE(status.platform_error() == 0);
  }

  SECTION("ShouldHaveEmptyLocationGivenRaiseError") {
    // Under Test.
    Status error = domain.raise_error(Quark::UP, platform_error1, message1);

    // Postconditions.
    REQUIRE(error.platform_error() == platform_error1);
    REQUIRE(error.message() == message1);
    REQUIRE(error.location().line() == 0u);
  }

  SECTION("ShouldOrderStrictlyGivenDistinctIncidents") {
    // Under Test.
    Status other = domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE_FALSE(status < status);
    REQUIRE((status < other) != (other < status));
  }

  SECTION("ShouldOrderByConditionGivenSameDomain") {
    // Preconditions.
    Status up = small_domain.raise_status(Quark::UP);
    Status charm = small_domain.raise_status(Quark::CHARM);

    // Postconditions.
    REQUIRE(up < charm);
    REQUIRE_FALSE(charm < up);
  }

  // Under Test.
  StatusDetached detached = status.detach_copy();

  SECTION("ShouldHaveSameMessageAsDetachedCopy") {
    // Postconditions.
    REQUIRE(detached.message() == status.message());
  }

  SECTION("ShouldHaveSameLocationAsDetachedCopy") {
    // Postconditions.
    REQUIRE(detached.location().line() == status.location().line());
  }

  SECTION("ShouldHaveSamePlatformErrorAsDetachedCopy") {
    // Postconditions.
    REQUIRE(detached.platform_error() == status.platform_error());
  }

  SECTION("ShouldBeEqualToDetachedCopy") {
    // Postconditions.
    REQUIRE(status == detached);
    REQUIRE(detached == status);
    REQUIRE(detached == status.kind());
    REQUIRE(status.kind() == detached);
  }

  SECTION("ShouldBeEqualToStatusWithSameIncident") {
    // Under Test.
    Status copy = status;

    // Postconditions.
    REQUIRE(copy == status);
    REQUIRE(status == copy);
  }

  SECTION("ShouldNotBeEqualToStatusWithDifferentIncident") {
    // Under Test.
    Status other = domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(other != status);
    REQUIRE(status != other);
  }

  SECTION("ShouldNotBeEqualToStatusWithDifferentCondition") {
    // Under Test.
    Status other = domain.raise_status(Quark::DOWN);

    // Postconditions.
    REQUIRE(other != status);
    REQUIRE(status != other);
  }

  SECTION("ShouldBeEqualToKindWithSameConditionAndDomain") {
    // Under Test.
    Status other = domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(status == other.kind());
    REQUIRE(other.kind() == status);
  }

  SECTION("ShouldNotBeEqualToKindWithDifferentCondition") {
    // Under Test.
    Status other = domain.raise_status(Quark::DOWN);

    // Postconditions.
    REQUIRE(status != other.kind());
    REQUIRE(other.kind() != status);
  }

  SECTION("ShouldHaveEqualKindWithSameConditionAndDomain") {
    // Under Test.
    Status other = domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(status.kind() == other.kind());
    REQUIRE(other.kind() == status.kind());
  }

  SECTION("ShouldNotHaveEqualKindWithDifferentCondition") {
    // Under Test.
    Status other = domain.raise_status(Quark::DOWN);

    // Postconditions.
    REQUIRE(status.kind() != other.kind());
    REQUIRE(other.kind() != status.kind());
  }

  SECTION("ShouldBeEquivalentToStatusWithSameCondition") {
    // Under Test.
    Status other = domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(status.has_equivalent_condition_as(other.kind()));
    REQUIRE(other.has_equivalent_condition_as(status.kind()));
  }

  SECTION("ShouldNotBeEquivalentToStatusWithDifferentCondition") {
    // Under Test.
    Status other = domain.raise_status(Quark::DOWN);

    // Postconditions.
    REQUIRE(!status.has_equivalent_condition_as(other.kind()));
    REQUIRE(!other.has_equivalent_condition_as(status.kind()));
  }

  SECTION("ShouldNotBeEquivalentToStatusOfOtherDomainWithSameCondition") {
    // Under Test.
    Status other = small_domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(status != other);
    REQUIRE(status.kind() != other.kind());
    REQUIRE_FALSE(status.has_equivalent_condition_as(other));
    REQUIRE_FALSE(status.has_equivalent_condition_as(other.kind()));
  }

  SECTION("ShouldBeOutStreamable") {
    // Preconditions.
    std::stringstream ss;

    // Under Test.
    ss << status;

    // Postconditions.
    REQUIRE(ss.str().size() > 0u);
    REQUIRE(ss.str() == status.message());
  }

  SECTION("ShouldStreamNothingGivenEmptyMessage") {
    // Preconditions.
    std::stringstream ss;

    // Under Test.
    ss << domain.raise_status(Quark::UP);

    // Postconditions.
    REQUIRE(ss.str().empty());
  }
}

TEST_CASE("StatusDetached") {
  SECTION("ShouldKeepMessageGivenIncidentSlotOverwritten") {
    // Preconditions.
    Status status = small_domain.raise_status(Quark::UP, message0);
    StatusDetached detached = status.detach_copy();

    // Under Test.
    for (std::size_t i = 0; i < SMALL_INCIDENT_COUNT; ++i) {
      auto _ = small_domain.raise_status(Quark::UP, message1);
    }

    // Postconditions.
    REQUIRE(detached.message() == message0);
    REQUIRE(status.message() == message1);
  }

  SECTION("ShouldKeepLocationGivenIncidentSlotOverwritten") {
    // Preconditions.
    Status status = small_domain.raise_status_here(Quark::UP, {}, location0);
    StatusDetached detached = status.detach_copy();

    // Under Test.
    for (std::size_t i = 0; i < SMALL_INCIDENT_COUNT; ++i) {
      auto _ = small_domain.raise_status_here(Quark::UP, {}, location1);
    }

    // Postconditions.
    REQUIRE(detached.location().line() == location0.line());
    REQUIRE(status.location().line() == location1.line());
  }

  SECTION("ShouldKeepPlatformErrorGivenIncidentSlotOverwritten") {
    // Preconditions.
    Status status = small_domain.raise_error(Quark::UP, platform_error0);
    StatusDetached detached = status.detach_copy();

    // Under Test.
    for (std::size_t i = 0; i < SMALL_INCIDENT_COUNT; ++i) {
      auto _ = small_domain.raise_error(Quark::UP, platform_error1);
    }

    // Postconditions.
    REQUIRE(detached.platform_error() == platform_error0);
    REQUIRE(status.platform_error() == platform_error1);
  }

  SECTION("ShouldKeepMessageGivenRaisingThreadEnded") {
    // Preconditions.
    std::optional<StatusDetached> detached;

    // Under Test.
    std::thread raiser{[&] {
      detached = raise_thread_local(Quark::TOP, message0).detach_copy();
    }};
    raiser.join();

    // Postconditions.
    REQUIRE(detached.has_value());
    REQUIRE(detached->message() == message0);
    REQUIRE(*detached == watch_thread_local(Quark::TOP));
  }

  SECTION("ShouldBeOutStreamable") {
    // Preconditions.
    std::stringstream ss;
    StatusDetached detached =
        domain.raise_status(Quark::UP, message0).detach_copy();

    // Under Test.
    ss << detached;

    // Postconditions.
    REQUIRE(ss.str() == message0);
  }
}

TEST_CASE("StatusKind") {
  StatusKind down = domain.watch_kind(Quark::DOWN);
  StatusKind up = domain.watch_kind(Quark::UP);
  Status status = domain.raise_status(Quark::UP);

  SECTION("ShouldHaveMessageDescribingCondition") {
    // Under Test.
    std::string_view message0 = up.message();
    std::string_view message1 = down.message();

    // Postconditions.
    REQUIRE(message0 == "UP");
    REQUIRE(message1 == "DOWN");
  }

  SECTION("ShouldBeTrueWhenComparedWithIncidentOfSameCondition") {
    // Postconditions.
    REQUIRE(status == up);
    REQUIRE(up == status);
  }

  SECTION("ShouldBeFalseWhenComparedWithIncidentOfDifferentCondition") {
    // Postconditions.
    REQUIRE(status != down);
    REQUIRE(down != status);
  }

  SECTION("ShouldBeEquivalentToIncidentOfSameCondition") {
    // Postconditions.
    REQUIRE(status.has_equivalent_condition_as(up));
  }

  SECTION("ShouldNotBeEquivalentToIncidentOfDifferentCondition") {
    // Postconditions.
    REQUIRE(!status.has_equivalent_condition_as(down));
  }

  SECTION("ShouldOrderByConditionGivenSameDomain") {
    // Postconditions.
    REQUIRE(up < down);
    REQUIRE_FALSE(down < up);
    REQUIRE_FALSE(up < up);
  }

  SECTION("ShouldEncodeDomainAndCondition") {
    // Postconditions.
    REQUIRE(domain_code_of(&up) == 42u);
    REQUIRE(condition_code_of(&up) == static_cast<std::size_t>(Quark::UP));
    REQUIRE(condition_code_of(&down) == static_cast<std::size_t>(Quark::DOWN));
  }

  SECTION("ShouldNotEqualKindOfOtherDomainGivenSameCondition") {
    // Under Test.
    StatusKind other = small_domain.watch_kind(Quark::UP);

    // Postconditions.
    REQUIRE(up != other);
    REQUIRE(status != other);
  }

  SECTION("ShouldEqualKindOfOtherDomainObjectGivenSameDomainCode") {
    // Preconditions.
    // Identity is the domain code, not the object, which is what lets every
    // thread's thread_local domain raise comparable statuses.
    QuarkKindDomain same_code{42u, "same"};

    // Under Test.
    StatusKind other = same_code.watch_kind(Quark::UP);

    // Postconditions.
    REQUIRE(up == other);
    REQUIRE(status == other);
  }

  SECTION("ShouldBeOutStreamable") {
    // Preconditions.
    std::stringstream ss;

    // Under Test.
    ss << up;

    // Postconditions.
    REQUIRE(ss.str().size() > 0u);
    REQUIRE(ss.str() == up.message());
  }
}

constexpr QuarkKindDomain quarks{42u, "quark"};

constexpr auto StatusKindShouldHaveSameMessage() -> bool {
  StatusKind kind = quarks.watch_kind(Quark::CHARM);
  return kind.message() == "CHARM";
}
constexpr auto StatusKindShouldCompareSame() -> bool {
  StatusKind kind_a = quarks.watch_kind(Quark::CHARM);
  StatusKind kind_b = quarks.watch_kind(Quark::CHARM);
  return kind_a == kind_b;
}
constexpr auto StatusKindShouldCompareDifferent() -> bool {
  StatusKind kind_a = quarks.watch_kind(Quark::CHARM);
  StatusKind kind_b = quarks.watch_kind(Quark::STRANGE);
  return kind_a != kind_b;
}

TEST_CASE("EnumStatusKindDomain") {
  // watch_kind goes through a virtual call, so these run at runtime.
  SECTION("ShouldHaveSameMessageAndCompareGivenWatchedKinds") {
    // Postconditions.
    REQUIRE(StatusKindShouldHaveSameMessage());
    REQUIRE(StatusKindShouldCompareSame());
    REQUIRE(StatusKindShouldCompareDifferent());
  }

  SECTION("ShouldHaveConstexprName") {
    // Under Test.
    constexpr std::string_view name = quarks.name();

    // Postconditions.
    REQUIRE(name == "quark");
  }
}

TEST_CASE("StatusEquivalence") {
  SECTION("ShouldBeEquivalentGivenStatusWithSameConditionDifferentIncident") {
    // Under Test.
    Status first = raise(Quark::TOP);
    Status second = raise(Quark::TOP);

    // Postconditions.
    REQUIRE(first.has_equivalent_condition_as(second));
    REQUIRE_FALSE(first.has_equivalent_condition_as(raise(Quark::CHARM)));
  }

  SECTION("ShouldBeEquivalentGivenDomainDeclaresConditionsEquivalent") {
    // Preconditions.
    Status up = equivalent_domain.raise_status(Quark::UP);
    Status down = equivalent_domain.raise_status(Quark::DOWN);

    // Postconditions.
    REQUIRE(up.has_equivalent_condition_as(down));
    REQUIRE(down.has_equivalent_condition_as(up));
    REQUIRE(up.has_equivalent_condition_as(down.kind()));
    REQUIRE(up.has_equivalent_condition_as(
        equivalent_domain.watch_kind(Quark::DOWN)));
  }

  SECTION("ShouldStayUnequalGivenDomainDeclaresConditionsEquivalent") {
    // Preconditions.
    Status up = equivalent_domain.raise_status(Quark::UP);
    Status down = equivalent_domain.raise_status(Quark::DOWN);

    // Postconditions.
    REQUIRE(up != down);
    REQUIRE(up.kind() != down.kind());
    REQUIRE(up != down.kind());
  }

  SECTION("ShouldNotBeEquivalentGivenConditionOutsideDomainRule") {
    // Preconditions.
    Status up = equivalent_domain.raise_status(Quark::UP);
    Status top = equivalent_domain.raise_status(Quark::TOP);

    // Postconditions.
    REQUIRE_FALSE(up.has_equivalent_condition_as(top));
    REQUIRE_FALSE(top.has_equivalent_condition_as(up));
    REQUIRE_FALSE(up.has_equivalent_condition_as(top.kind()));
  }
}

TEST_CASE("StaticEnumStatusDomain") {
  SECTION("ShouldReturnOneDomainGivenSameTemplateArguments") {
    // Under Test.
    auto& first = static_enum_status_domain<Quark, QUARK_CONDITION_COUNT>();
    auto& second = static_enum_status_domain<Quark, QUARK_CONDITION_COUNT>();

    // Postconditions.
    REQUIRE(&first == &second);
    REQUIRE(first.name().starts_with("static_enum_status_domain_"));
  }

  SECTION("ShouldReturnAnotherDomainGivenDifferentIncidentCountMax") {
    // Under Test.
    auto& other = static_enum_status_domain<Quark, QUARK_CONDITION_COUNT, 4>();

    // Postconditions.
    REQUIRE(other.watch_kind(Quark::TOP) != watch(Quark::TOP));
  }

  SECTION("ShouldRaiseStatusFromDifferentDomain") {
    // Preconditions.
    Status other = domain.raise_status(Quark::TOP);

    // Under Test.
    Status status0 = raise(Quark::TOP);
    Status status1 = raise(Quark::TOP);

    // Postconditions.
    REQUIRE(status0.kind() != other.kind());
    REQUIRE(status0.kind() == status1.kind());
  }

  SECTION("ShouldRaiseMultipleStatusWithDifferentMessages") {
    // Under Test.
    Status status0 = raise(Quark::TOP, message0);
    Status status1 = raise(Quark::TOP, message1);

    // Postconditions.
    REQUIRE(status0.message() == message0);
    REQUIRE(status1.message() == message1);
  }

  SECTION("ShouldRaiseMultipleStatusHereWithDifferentLocations") {
    // Under Test.
    Status status0 = raise_here(Quark::TOP, message0, location0);
    Status status1 = raise_here(Quark::TOP, message1, location1);

    // Postconditions.
    REQUIRE(status0.location().line() == location0.line());
    REQUIRE(status1.location().line() == location1.line());
  }

  SECTION("ShouldRaiseStatusHereWithLocationByDefault") {
    // Under Test.
    Status status = raise_here(Quark::TOP);

    // Postconditions.
    REQUIRE(status.location().line() > 0u);
  }

  SECTION("ShouldWatchForCondition") {
    // Preconditions.
    Status status = raise(Quark::TOP);
    StatusKind kind = watch(Quark::TOP);

    // Under Test.
    bool found = false;
    if (status == kind || kind == status) {
      found = true;
    }

    // Postconditions.
    REQUIRE(found);
  }
}

TEST_CASE("ThreadLocalEnumStatusDomain") {
  SECTION("ShouldMatchKindGivenStatusRaisedOnAnotherThread") {
    // Preconditions.
    // The domain's identity is shared; only its incident storage is per
    // thread.
    StatusKind watched = watch_thread_local(Quark::TOP);
    StatusKind other_condition = watch_thread_local(Quark::CHARM);
    bool same_condition_matches = false;
    bool other_condition_differs = false;

    // Under Test.
    std::thread raiser{[&] {
      Status raised = raise_thread_local(Quark::TOP);
      same_condition_matches = raised.kind() == watched;
      other_condition_differs = raised.kind() != other_condition;
    }};
    raiser.join();

    // Postconditions.
    REQUIRE(same_condition_matches);
    REQUIRE(other_condition_differs);
  }

  SECTION("ShouldRaiseStatusFromDifferentDomain") {
    // Preconditions.
    Status other = domain.raise_status(Quark::TOP);

    // Under Test.
    Status status = raise_thread_local(Quark::TOP);

    // Postconditions.
    REQUIRE(status.kind() != other.kind());
  }

  SECTION("ShouldKeepThisThreadsMessageGivenRaiseOnAnotherThread") {
    // Preconditions.
    Status status = raise_thread_local(Quark::TOP, message0);

    // Under Test.
    // Each thread's storage holds one incident, so a shared one would
    // overwrite this thread's message.
    std::thread raiser{
        [] { auto _ = raise_thread_local(Quark::TOP, message1); }};
    raiser.join();

    // Postconditions.
    REQUIRE(status.message() == message0);
  }

  SECTION("ShouldRaiseMultipleStatusWithSingleMessagePerThread") {
    // Under Test.
    Status status0 = raise_thread_local(Quark::TOP, message0);
    Status status1 = raise_thread_local(Quark::TOP, message1);

    // Postconditions.
    REQUIRE(status0.message() != message0);
    REQUIRE(status0.message() == message1);
    REQUIRE(status1.message() == message1);
  }

  SECTION("ShouldRaiseMultipleStatusHereWithSingleLocationPerThread") {
    // Under Test.
    Status status0 = raise_here_thread_local(Quark::TOP, message0, location0);
    Status status1 = raise_here_thread_local(Quark::TOP, message1, location1);

    // Postconditions.
    REQUIRE(status0.location().line() != location0.line());
    REQUIRE(status0.location().line() == location1.line());
    REQUIRE(status1.location().line() == location1.line());
  }

  SECTION("ShouldRaiseStatusHereWithLocationByDefault") {
    // Under Test.
    Status status = raise_here_thread_local(Quark::TOP);

    // Postconditions.
    REQUIRE(status.location().line() > 0u);
  }

  SECTION("ShouldWatchForCondition") {
    // Preconditions.
    Status status = raise_thread_local(Quark::TOP);
    StatusKind kind = watch_thread_local(Quark::TOP);

    // Under Test.
    bool found = false;
    if (status == kind || kind == status) {
      found = true;
    }

    // Postconditions.
    REQUIRE(found);
  }
}

TEST_CASE("StaticEnumStatusDomainIncidentCount") {
  SECTION("ShouldRaiseStatusGivenNonDefaultIncidentCountMax") {
    // Under Test.
    auto& small = static_enum_status_domain<Quark, QUARK_CONDITION_COUNT, 4>();
    Status status = small.raise_status(Quark::CHARM, message0);

    // Postconditions.
    REQUIRE(status.message() == message0);
    REQUIRE(status.kind().message() == "CHARM");
  }
}

TEST_CASE("StatusKindDefault") {
  SECTION("ShouldNotEqualAnyRaisedStatusGivenDefaultKind") {
    // Postconditions.
    REQUIRE(StatusKind{} != raise(Quark::TOP).kind());
  }

  SECTION("ShouldHaveEmptyMessageGivenDefaultConstruction") {
    // Under Test.
    StatusKind kind;

    // Postconditions.
    REQUIRE(kind.message().empty());
  }
}

}  // namespace lib
