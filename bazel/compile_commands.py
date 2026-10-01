#!/usr/bin/env python3
# Copyright 2022 -- CONTRIBUTORS. See LICENSE.

"""Superseded by lsp_mirror.py, which this runs, so old instructions work.

This used to point compile_commands.json into Bazel's execution root, which
every build relinks to only the external repositories it needed; clangd lost
headers whenever a partial build or a compiler switch ran. lsp_mirror.py
writes it against a mirror outside Bazel's reach instead. See README's editor
setup.
"""

import pathlib
import runpy

if __name__ == "__main__":
    runpy.run_path(str(pathlib.Path(__file__).with_name("lsp_mirror.py")), run_name="__main__")
