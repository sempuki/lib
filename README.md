# lib

Core C++ building blocks shared by [volcano](https://github.com/sempuki/volcano)
and [simon](https://github.com/sempuki/simon). Everything lives in `namespace lib`.

| Target               | What                                                         |
| -------------------- | ------------------------------------------------------------ |
| `@lib//base:core`     | Class-declaration macros, contract checks, `Out`/`InOut`/`Depend` argument wrappers, `narrow_cast`, `Overloaded`, `stable_hash`. |
| `@lib//base:contract` | `EXPECT`/`ENSURE`/`ASSERT` macros throwing typed contract errors. |
| `@lib//base:status`   | Allocation-free, domain-extensible status/error codes.       |
| `@lib//base:math`     | Eigen matrix/vector aliases.                                 |
| `@lib//base:time`     | `SimClock` simulation clock, `Duration`, `TimePoint`.        |
| `@lib//base:testing`  | Catch2 main plus a summary reporter; depend on it from `cc_test`. |

## Build

Requires [Bazelisk](https://github.com/bazelbuild/bazelisk) (installed as
`bazel`). The Bazel release is pinned in `.bazelversion`.

```sh
bazel test //...
bazel test //... --repo_env=CC=clang   # or pick a compiler explicitly
```

The language standard is C++26 (`/std:c++latest` on MSVC), applied per target
through `COPTS` in `bazel/copts.bzl`. Put machine-local flags in an untracked
`user.bazelrc`.

## Using from another repo

Add lib as a git submodule and point Bazel at it:

```sh
git submodule add git@github.com:sempuki/lib.git 2nd_party/lib
echo 2nd_party/lib >> .bazelignore
```

```starlark
# MODULE.bazel
bazel_dep(name = "lib")
local_path_override(module_name = "lib", path = "2nd_party/lib")
```

Then `#include "base/core.hpp"`, depend on `@lib//base:core`, and give each
first-party target `copts = COPTS` from `load("@lib//bazel:copts.bzl", "COPTS")`
so it builds with the same standard and warnings as lib.
