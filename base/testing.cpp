// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#include "base/testing.hpp"

#include <iomanip>
#include <iostream>

namespace lib {

constexpr const std::string_view test_marker{"\033[1;34m=\033[0m"};
constexpr const std::string_view section_marker{"\033[1;33m-\033[0m"};
constexpr const std::string_view result_marker{"\033[1;33m*\033[0m"};
constexpr const std::string_view passed{"\033[1;32mPASSED\033[0m."};
constexpr const std::string_view failed{"\033[1;31mFAILED\033[0m."};

auto SummaryReporter::getDescription() -> std::string {
  return "Summary reporter";
}

auto SummaryReporter::testRunStarting(Catch::TestRunInfo const& info) -> void {
  report_.emplace_back();
  report_.back() << test_marker << ' ' << info.name;
}

auto SummaryReporter::testRunEnded(Catch::TestRunStats const& /*stats*/)
    -> void {
  std::cout << "\n\n**** Test Run Results. ****\n\n";
  for (auto&& line : report_) {
    std::cout << line.str() << "\n";
  }
}

auto SummaryReporter::sectionStarting(Catch::SectionInfo const& info) -> void {
  depth_++;
  switch (depth_) {
    case 1:
      report_.emplace_back();
      report_.back() << section_marker;
      [[fallthrough]];
    case 2:  //
      report_.back() << ' ' << info.name;
      break;
    default:  //
      report_.emplace_back();
      report_.back() << std::setw(depth_ * 2) << info.name;
      break;
  }
}

auto SummaryReporter::sectionEnded(Catch::SectionStats const& stats) -> void {
  depth_--;

  auto& result = stats.assertions.allPassed() ? passed : failed;
  switch (depth_) {
    case 1:  //
      report_.back() << ' ' << result;
      break;
    case 2:
      report_.emplace_back();
      report_.back() << result_marker;
      break;
    default:  //
      break;
  }
}

auto SummaryReporter::assertionStarting(Catch::AssertionInfo const& /*info*/)
    -> void {}
auto SummaryReporter::assertionEnded(Catch::AssertionStats const& /*stats*/)
    -> void {}

CATCH_REGISTER_LISTENER(SummaryReporter)

}  // namespace lib
