from pathlib import Path
Import("env")

env.Execute(f"$PYTHONEXE -m pip install -r requirements.txt")

import os
import subprocess
from generate_struct_from_schema import generate_cpp_and_header_files
from generate_config import generate_config_class
from simulation import start_simulation_framework
from SCons.Script import Import, DefaultEnvironment





env.AddCustomTarget(
    'simulation',
    None,
    start_simulation_framework,
    title='simulation',
    description=None,
    always_build=True,
)

cxxflags = " ".join(env.get("CXXFLAGS", []))

if "-std=gnu++17" not in cxxflags and "-std=c++17" not in cxxflags:
    print("\n\n[ERROR] This library requires C++17.")
    print("Please add the following to your platformio.ini:\n")
    print("    build_unflags = -std=gnu++11 -std=gnu++14")
    print("    build_flags = -std=gnu++17\n")
    exit(1)
print("SimuCore SRC_FILTER:", env['SRC_FILTER'])

generate_cpp_and_header_files(env)
generate_config_class(env)

platform = env.GetProjectOption("platform")
frameworks = env.GetProjectOption("framework")

src_dirs = []

# Always include generic
src_dirs.append("+<generic/>")
src_dirs.append("+<generated/>")


if platform == "native":
    src_dirs.append("+<native/>")
elif isinstance(frameworks, list) and "arduino" in frameworks:
    src_dirs.append("+<arduino/>")
elif isinstance(frameworks, str) and "arduino" in frameworks:
    src_dirs.append("+<arduino/>")

env.Replace(SRC_FILTER=" ".join(src_dirs))
