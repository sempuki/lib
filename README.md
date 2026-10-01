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

## Editor setup

clangd needs a `compile_commands.json`, and the headers it names must stay put.
Bazel's execution root does not: every build relinks it to only the external
repositories that build needed, and switching compilers reconfigures it.
`bazel/mirror.py` builds in an output base of its own, copies the headers
clangd reads into `.lsp/mirror/`, and writes `compile_commands.json` against
that mirror, so builds and compiler switches never disturb the editor. Ignore
`.lsp/` in both `.gitignore` and `.bazelignore`, then from the workspace root:

```sh
python3 bazel/mirror.py                  # in lib; 2nd_party/lib/bazel/mirror.py elsewhere
python3 bazel/mirror.py --if-stale       # only if files or targets changed
python3 bazel/mirror.py --watch 60       # check every minute, e.g. in a tmux pane
python3 bazel/mirror.py --install-hooks  # refresh after checkout, merge, rebase
```

`--if-stale` takes a fraction of a second when nothing changed, so it is cheap
to run often. Restart clangd after the first build.
