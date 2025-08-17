import os
import subprocess
import sys
from pathlib import Path
from generate_config import generate_config
Import("env")

generate_config('simucore_config.schema.json', 'test.h')

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

print("SimuCore SRC_FILTER:", env['SRC_FILTER'])
