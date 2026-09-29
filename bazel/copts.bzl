# Copyright 2022 -- CONTRIBUTORS. See LICENSE.

"""Compiler flags shared by lib and the projects that depend on it.

Every first-party cc_* target passes `copts = COPTS`. The language standard is
set here rather than with a global --cxxopt so that third-party modules keep
their own defaults: a global --cxxopt also leaks into C probes (e.g.
rules_cc_autoconf), where Clang rejects a C++ -std outright.
"""

_MSVC = [
    "/std:c++latest",
    "/Zc:__cplusplus",
    "/Zc:preprocessor",
    "/utf-8",
    "/permissive-",
    "/W4",
]

# C++26 is still spelled c++2c by some shipping compilers (Apple Clang).
_GNU = ["-std=c++2c", "-Wall", "-Wextra"]

COPTS = select({
    "@rules_cc//cc/compiler:msvc-cl": _MSVC,
    "@rules_cc//cc/compiler:clang-cl": _MSVC,
    # Contract checks throw from noexcept functions by design: the throw is the
    # terminate, and -Wterminate (GCC) / -Wexceptions (Clang) flag every one.
    # GCC also flags partial designated initializers (`.sType = ...`), which
    # are the idiomatic way to fill C API structs.
    "@rules_cc//cc/compiler:gcc": _GNU + [
        "-Wno-terminate",
        "-Wno-missing-field-initializers",
    ],
    "@rules_cc//cc/compiler:clang": _GNU + ["-Wno-exceptions"],
    "//conditions:default": _GNU,
})
