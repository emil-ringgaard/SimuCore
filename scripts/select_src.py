
Import("env") # pyright: ignore[reportUndefinedVariable]
import platform

if platform.system() == "Windows":
    # Append the Winsock2 library to the linker
    env.Append(LIBS=["ws2_32"])
    # Append the Winsock2 library to the linker
    env.Append(LIBS=["ws2_32"])
    
    # Disable Link Time Optimization (fixes the 'plugin needed' archiver error)
    env.Append(CCFLAGS=["-fno-lto"])
    env.Append(CXXFLAGS=["-fno-lto"])
    env.Append(LINKFLAGS=["-fno-lto"])
    
    # Force the linker to build a Console app, not a GUI app (fixes the WinMain error)
    env.Append(LINKFLAGS=["-mconsole"])

from generate_config import generate_config_class
from generate_struct_from_schema import generate_cpp_and_header_files
from simulation import start_simulation_framework

env.AddCustomTarget( # pyright: ignore[reportUndefinedVariable]
    "simulation",
    "$BUILD_DIR/program",
    start_simulation_framework,
    title="simulation",
    description=None,
    always_build=True,
)

cxxflags = " ".join(env.get("CXXFLAGS", [])) # pyright: ignore[reportUndefinedVariable]

if "-std=gnu++17" not in cxxflags and "-std=c++17" not in cxxflags:
    print("\n\n[ERROR] This library requires C++17.")
    print("Please add the following to your platformio.ini:\n")
    print("    build_unflags = -std=gnu++11 -std=gnu++14")
    print("    build_flags = -std=gnu++17\n")
    exit(1)
print("SimuCore SRC_FILTER:", env["SRC_FILTER"]) # pyright: ignore[reportUndefinedVariable]


generate_cpp_and_header_files(env) # pyright: ignore[reportUndefinedVariable]
generate_config_class(env) # pyright: ignore[reportUndefinedVariable]

platform_type = env.GetProjectOption("platform") # pyright: ignore[reportUndefinedVariable]
frameworks = env.GetProjectOption("framework") # pyright: ignore[reportUndefinedVariable]



src_dirs = []

# Always include generic and generated
src_dirs.append("+<generic/>")
src_dirs.append("+<generated/>")


if platform_type == "native":
    src_dirs.append("+<native/>")
elif isinstance(frameworks, list) and "arduino" in frameworks or isinstance(frameworks, str) and "arduino" in frameworks:
    src_dirs.append("+<arduino/>")

env.Replace(SRC_FILTER=" ".join(src_dirs)) # pyright: ignore[reportUndefinedVariable]
