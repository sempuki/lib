// Copyright 2022 -- CONTRIBUTORS. See LICENSE.

#include "base/core.hpp"

#include <cstdlib>
#include <format>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>

#if defined(__GNUC__)  // Clang also supports this header.
#include <cxxabi.h>
#endif

namespace lib {

namespace internal {

void do_contract_failure(const char* kind, const char* condition,
                         std::source_location location) {
  throw std::logic_error(
      std::format("[{}] {} Failed {}: {}", location.file_name(),
                  location.function_name(), kind, condition));
}

}  // namespace internal

#if defined(__GNUC__)
std::string demangle(const std::string& name) {
  // Let __cxa_demangle allocate, since it may realloc() any buffer it is given.
  int out_status = 0;
  std::unique_ptr<char, decltype(&std::free)> demangled{
      abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &out_status),
      &std::free};
  return (out_status == 0 && demangled) ? std::string{demangled.get()} : name;
}
#else
std::string demangle(const std::string& name) { return name; }
#endif

}  // namespace lib
