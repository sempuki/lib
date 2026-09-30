#!/usr/bin/env python3
# Copyright 2022 -- CONTRIBUTORS. See LICENSE.

"""Writes compile_commands.json for clangd from Bazel's action graph.

Run it from the root of any workspace that uses lib:

    python3 bazel/compile_commands.py                # in lib
    python3 2nd_party/lib/bazel/compile_commands.py  # in simon or volcano

Optional arguments are Bazel target patterns (default //...). It asks Bazel for
every C++ compile action under the Clang toolchain, so clangd sees exactly the
flags and include paths each file builds with, including external libraries
such as Eigen and mp-units. It builds the targets first, so generated headers
exist. Rerun it after adding files, targets or dependencies.
"""

import json
import os
import pathlib
import subprocess
import sys

# Clang's flags, which clangd understands; GCC's can include options it does not.
BAZEL_FLAGS = ["--repo_env=CC=clang"]


def bazel(*arguments: str) -> str:
    return subprocess.check_output(["bazel", *arguments], text=True)


def main() -> None:
    workspace = pathlib.Path(os.environ.get("BUILD_WORKSPACE_DIRECTORY", os.getcwd()))
    targets = sys.argv[1:] or ["//..."]

    subprocess.check_call(["bazel", "build", *BAZEL_FLAGS, *targets])
    execution_root = bazel("info", *BAZEL_FLAGS, "execution_root").strip()
    query = 'mnemonic("CppCompile", {})'.format(" + ".join(targets))
    graph = json.loads(
        bazel("aquery", *BAZEL_FLAGS, query, "--output=jsonproto", "--include_artifacts=false")
    )

    commands = {}
    for action in graph.get("actions", []):
        arguments = action["arguments"]
        source = arguments[arguments.index("-c") + 1]
        if source.startswith(("external/", "bazel-out/")) or source in commands:
            continue
        commands[source] = {
            # Relative paths in the arguments resolve against the execution root,
            # which links to the workspace and every external repository.
            "directory": execution_root,
            # The workspace path, so clangd matches the file the editor opened.
            "file": str(workspace / source),
            "arguments": arguments,
        }

    output = workspace / "compile_commands.json"
    output.write_text(json.dumps(list(commands.values()), indent=2) + "\n")
    print(f"Wrote {len(commands)} compile commands to {output}")


if __name__ == "__main__":
    main()
