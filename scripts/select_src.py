import os
import subprocess
import sys
from pathlib import Path
from generate_cpp_config import generate_cpp_config
from generate_config_struct import generate_header_config
Import("env")


generate_header_config()
generate_cpp_config(env)

platform = env.GetProjectOption("platform")
frameworks = env.GetProjectOption("framework")

src_dirs = []

# Always include generic
src_dirs.append("+<generic/>")

if platform == "native":
    src_dirs.append("+<native/>")
elif isinstance(frameworks, list) and "arduino" in frameworks:
    src_dirs.append("+<arduino/>")
elif isinstance(frameworks, str) and "arduino" in frameworks:
    src_dirs.append("+<arduino/>")

env.Replace(SRC_FILTER=" ".join(src_dirs))
cxxflags = " ".join(env.get("CXXFLAGS", []))

if "-std=gnu++17" not in cxxflags and "-std=c++17" not in cxxflags:
    print("\n\n[ERROR] This library requires C++17.")
    print("Please add the following to your platformio.ini:\n")
    print("    build_unflags = -std=gnu++11 -std=gnu++14")
    print("    build_flags = -std=gnu++17\n")
    exit(1)
print("SimuCore SRC_FILTER:", env['SRC_FILTER'])

