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

The language standard is C++26 (`/std:c++latest` on MSVC), set per-platform in
`.bazelrc`. Put machine-local flags in an untracked `user.bazelrc`.

## Using from another repo

Add lib as a git submodule and point Bazel at it:

```sh
git submodule add git@github.com:sempuki/lib.git third_party/lib
echo third_party/lib >> .bazelignore
```

```starlark
# MODULE.bazel
bazel_dep(name = "lib")
local_path_override(module_name = "lib", path = "third_party/lib")
```

Then `#include "base/core.hpp"` and depend on `@lib//base:core`. Consumers must
build with at least the std flags in this repo's `.bazelrc`; shared warning
flags are available via `load("@lib//bazel:copts.bzl", "COPTS")`.
