import os
import subprocess

here = os.path.dirname(os.path.realpath(__file__))
src = os.path.join(here, "..", "src", "*.cc")

cmd = (
    "cl",
    "/DDBG",
    "/Feayane",
    "/IC:/mpir",
    "/MP",
    "/MTd",
    "/WX",
    "/Zi",
    "/std:c++17",
    src,
    "C:/mpir/debug.lib",
    "dbghelp.lib",
)
subprocess.check_call(cmd)
