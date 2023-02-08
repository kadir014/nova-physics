"""

    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/Nova-Physics


    Nova Physics Engine Example Runner Script
    --------------------------------------
    This Python script is used to run example demos.
    Downloads dependencies like SDL2 and SDL2_ttf if
    not found in the working directory.

    Usage: python run_example <example> [-f]

    example : Name of the example C file to run
    -f      : Force download all dependencies

"""

import sys
import subprocess
import platform
import os
import tarfile
import io
import shutil
import stat
import requests


"""
    TODO: Rewrite this script using pathlib
"""


if len(sys.argv) == 1:
    print("Usage: python run_example <example> [-f]\n" + \
          "example : Name of the example C file to run\n" + \
          "-f      : Force download all dependencies")
    raise SystemExit(1)

if len(sys.argv) == 2:
    example = sys.argv[1]

    if not os.path.exists(f"{example}.c"):
        print(f"{example}.c is not found in the examples directory.\n")
        print("make sure you are running the script in examples directory.")
        raise SystemExit(1)

force_download = False
if len(sys.argv) == 3:
    if sys.argv[2] == "-f":
        force_download = True
    else:
        print("Unknown flag 'f'")
        raise SystemExit(1)


# Overwrites READ-ONLY files as WRITE-ONLY and removes them
def del_rw(action, name, exc):
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

# Copies directory to anotther existing directory
def copytree(src, dst, symlinks=False, ignore=None):
    for item in os.listdir(src):
        try:
            s = os.path.join(src, item)
            d = os.path.join(dst, item)
            if os.path.isdir(s):
                shutil.copytree(s, d, symlinks, ignore)
            else:
                shutil.copy2(s, d)
        except:
            pass


print("Checking dependencies")

# Flags to determine missing dependencies
# TODO: Handle controlling dependencies better
download_includes = False
download_libs = False
download_dlls = False

# Check SDL2 headers
if not os.path.exists("include/SDL2"):
    download_includes = True

# Check SDL2 libraries
if not os.path.exists("lib"):
    download_libs = True

# Check SDL2 DLLs
if not os.path.exists("SDL2.dll") or not os.path.exists("SDL2_ttf.dll"):
    download_dlls = True

print(f"Dependencies -> includes:{not download_includes}, libs:{not download_libs}, dlls: {not download_dlls}")

if download_includes or download_libs or download_dlls:
    # TODO: Handle downloading, version & platform control better
    SDL2_VER = "2.26.3"
    SDL2_URL = f"https://github.com/libsdl-org/SDL/releases/download/release-{SDL2_VER}/SDL2-devel-{SDL2_VER}-mingw.tar.gz"

    print(f"Downloading SDL2-{SDL2_VER}")

    if os.path.exists("sdl"): shutil.rmtree("sdl", onerror=del_rw)
    if os.path.exists("ttf"): shutil.rmtree("ttf", onerror=del_rw)

    res = requests.get(SDL2_URL)

    with io.BytesIO(res.content) as fileobj:
        with tarfile.open(mode="r:gz", fileobj=fileobj) as tarred:
            tarred.extractall("sdl")

    TTF_VER = "2.20.2"
    TTF_URL = f"https://github.com/libsdl-org/SDL_ttf/releases/download/release-{TTF_VER}/SDL2_ttf-devel-{TTF_VER}-mingw.tar.gz"

    print(f"Downloading SDL2_ttf-{TTF_VER}")

    res = requests.get(TTF_URL)

    with io.BytesIO(res.content) as fileobj:
        with tarfile.open(mode="r:gz", fileobj=fileobj) as tarred:
            tarred.extractall("ttf")

    sdl_dir = f"sdl/SDL2-{SDL2_VER}/"
    ttf_dir = f"ttf/SDL2_ttf-{TTF_VER}/"

    if platform.machine().endswith("64"):
        sdl_dir += "x86_64-w64-mingw32/"
        ttf_dir += "x86_64-w64-mingw32/"
    else:
        sdl_dir += "i686-w64-mingw32/"
        ttf_dir += "i686-w64-mingw32/"

    # Extract include headers to script directory
    if download_includes:
        print(f"Extracting SDL2-{SDL2_VER}/include/SDL2/")
        shutil.copytree(sdl_dir + "include", "include")

        print(f"Extracting SDL2_ttf-{TTF_VER}/include/SDL2/SDL_ttf.h")
        shutil.copyfile(ttf_dir + "include/SDL2/SDL_ttf.h", "include/SDL2/SDL_ttf.h")

    # Extract libraries to script directory
    if download_libs:
        print(f"Extracting SDL2-{SDL2_VER}/lib/")
        shutil.copytree(sdl_dir + "lib", "lib")

        print(f"Extracting SDL2_ttf-{TTF_VER}/lib/")
        copytree(ttf_dir + "lib", "lib")

    # Extract DLLs to script directory
    if download_dlls:
        print(f"Extracting SDL2-{SDL2_VER}/bin/SDL2.dll")
        shutil.copyfile(sdl_dir + "bin/SDL2.dll", "SDL2.dll")

        print(f"Extracting SDL2_ttf-{TTF_VER}/bin/SDL2_ttf.dll")
        shutil.copyfile(ttf_dir + "bin/SDL2_ttf.dll", "SDL2_ttf.dll")


print("Cleaning up")
if os.path.exists("sdl"): shutil.rmtree("sdl", onerror=del_rw)
if os.path.exists("ttf"): shutil.rmtree("ttf", onerror=del_rw)


if platform.system() == "Windows": BINARY = "nova_example.exe"
else: BINARY = "nova_example"

if os.path.exists(BINARY):
    os.remove(BINARY)

source_files = (
    f"{example}.c",
    "../src/vector.c",
    "../src/body.c",
    "../src/math.c",
    "../src/resolution.c",
    "../src/space.c",
    "../src/collision.c",
    "../src/contact.c",
    "../src/solver.c",
)

source_files_arg = " ".join(source_files)
includes = "-I../include/ -I./include/"
libs = "-L../lib/ -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf"
args = "-O3"

print("Compilation started")

out = subprocess.run(f"gcc -o nova_example.exe {source_files_arg} {includes} {libs} {args}")

print("Compilation return code:", out.returncode)

if os.path.exists(BINARY):
    out = subprocess.run(BINARY)
    print("Run return code:", out.returncode)
    os.remove(BINARY)