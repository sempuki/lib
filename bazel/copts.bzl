# Copyright 2022 -- CONTRIBUTORS. See LICENSE.

"""Warning flags shared by lib and the projects that depend on it.

The language standard is set per-platform in .bazelrc so that every target,
including external dependencies, agrees on one standard.
"""

COPTS = select({
    "@rules_cc//cc/compiler:msvc-cl": ["/W4", "/permissive-"],
    "@rules_cc//cc/compiler:clang-cl": ["/W4", "/permissive-"],
    # Contract checks throw from noexcept functions by design: the throw is the
    # terminate, and -Wterminate (GCC) / -Wexceptions (Clang) flag every one.
    "@rules_cc//cc/compiler:gcc": ["-Wall", "-Wextra", "-Wno-terminate"],
    "@rules_cc//cc/compiler:clang": ["-Wall", "-Wextra", "-Wno-exceptions"],
    "//conditions:default": ["-Wall", "-Wextra"],
})
