// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#pragma once

#include <sstream>
#include <vector>

#include "catch2/catch_test_case_info.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/reporters/catch_reporter_event_listener.hpp"
#include "catch2/reporters/catch_reporter_registrars.hpp"

namespace lib {

// See: external/catch2/examples/210-Evt-EventListeners.cpp
struct SummaryReporter : Catch::EventListenerBase {
  using EventListenerBase::EventListenerBase;

  static auto getDescription() -> std::string;

  auto testRunStarting(Catch::TestRunInfo const& info) -> void override;
  auto testRunEnded(Catch::TestRunStats const& stats) -> void override;

  auto sectionStarting(Catch::SectionInfo const& info) -> void override;
  auto sectionEnded(Catch::SectionStats const& stats) -> void override;

  auto assertionStarting(Catch::AssertionInfo const& info) -> void override;
  auto assertionEnded(Catch::AssertionStats const& stats) -> void override;

 private:
  std::size_t depth_ = 0;
  std::vector<std::stringstream> report_;
};

}  // namespace lib
