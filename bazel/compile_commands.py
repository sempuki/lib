#!/usr/bin/env python3
# Copyright 2022 -- CONTRIBUTORS. See LICENSE.

"""Runs lsp_mirror.py, under another name.

lsp_mirror.py writes compile_commands.json against a mirror of the headers
outside Bazel's reach. See README's editor setup.
"""

import pathlib
import runpy

if __name__ == "__main__":
    runpy.run_path(str(pathlib.Path(__file__).with_name("lsp_mirror.py")), run_name="__main__")
