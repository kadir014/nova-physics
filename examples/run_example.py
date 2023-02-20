"""

    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/nova-physics


    Nova Physics Engine Example Runner Script
    --------------------------------------
    This Python script is used to run example demos.
    Downloads dependencies like SDL2 and SDL2_ttf if
    not found in the working directory.

    Usage: python run_example.py <example> [-f, -g, -r, -b]

    example : Name of the example C file to run
    -f      : Force download all dependencies
    -g      : Compile with -g flag for debugging
    -r      : Don't remove the compiled binary afterwards
    -b      : Benchmark

"""

import sys
import subprocess
import platform
import os
import pathlib
import tarfile
import io
import shutil
import stat
import time
import requests


if len(sys.argv) == 1:
    print("Usage: python run_example.py <example> [-f, -g, -r, -b]\n\n" + \
          "example : Name of the example C file to run\n" + \
          "-f      : Force download all dependencies\n" + \
          "-g      : Compile with -g flag for debugging\n" + \
          "-r      : Don't remove the compiled binary afterwards\n" + \
          "-b      : Benchmark")
    raise SystemExit(1)

if len(sys.argv) >= 2:
    example = sys.argv[1]

    if not os.path.exists(f"{example}.c"):
        print(f"{example}.c is not found in the examples directory.\n")
        print("Make sure you are running the script in the examples directory.")
        raise SystemExit(1)

# Set argument flags
force_download = False
benchmark = False
benchmark_arg = None
debug = False
remove_binary = True
if len(sys.argv) >= 3:
    for arg in sys.argv[2:]:
        if arg == "-f":
            force_download = True

        elif arg == "-g":
            debug = True

        elif arg == "-r":
            remove_binary = False

        elif arg.startswith("-b"):
            if len(arg) > 2:
                benchmark_arg = int(arg[2:])

            benchmark = True

        else:
            print(f"Unknown flag '{arg}'")
            raise SystemExit(1)


def del_rw(action, name, exc):
    """ Overwrites READ-ONLY files as WRITE-ONLY and removes them """
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

def copytree(src, dst, symlinks=False, ignore=None):
    """ Copies directory to another existing directory """
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

def clean():
    """ Clean all dependencies """
    if os.path.exists(INCLUDE_PATH): shutil.rmtree(INCLUDE_PATH, onerror=del_rw)
    if os.path.exists(LIB_PATH): shutil.rmtree(LIB_PATH, onerror=del_rw)
    if os.path.exists(SDL2_DLL_PATH): os.remove(SDL2_DLL_PATH)
    if os.path.exists(TTF_DLL_PATH): os.remove(TTF_DLL_PATH)

def clean_tmp():
    """ Clean temporary dependency directories """
    if os.path.exists(SDL2_TMP_PATH): shutil.rmtree(SDL2_TMP_PATH, onerror=del_rw)
    if os.path.exists(TTF_TMP_PATH): shutil.rmtree(TTF_TMP_PATH, onerror=del_rw)


BASE_PATH = pathlib.Path(os.getcwd())
NOVA_SRC_PATH = BASE_PATH.parent / "src"
INCLUDE_PATH = BASE_PATH / "include"
LIB_PATH = BASE_PATH / "lib"
SDL2_DLL_PATH = BASE_PATH / "SDL2.dll"
TTF_DLL_PATH = BASE_PATH / "SDl2_ttf.dll"

SDL2_TMP_PATH = BASE_PATH / "sdl"
TTF_TMP_PATH = BASE_PATH / "ttf"


if not benchmark:
    print("Checking dependencies")

    # Flags to determine missing dependencies
    download_includes = False
    download_libs = False
    download_dlls = False

    # Check SDL2 headers
    if not os.path.exists(INCLUDE_PATH / "SDL2"):
        download_includes = True

    # Check SDL2 libraries
    if not os.path.exists(LIB_PATH):
        download_libs = True

    # Check SDL2 DLLs
    if not os.path.exists(SDL2_DLL_PATH) or not os.path.exists(TTF_DLL_PATH):
        download_dlls = True

    if platform.system() != "Windows": download_dlls = False

    # Force download dependencies
    if force_download:
        download_includes = True
        download_libs = True
        download_dlls = True
        print("Force download all dependencies")
        print()

        # Remove existing dependencies
        clean()

    else:
        print(f" - SDL2 headers:   {('missing', 'not missing')[not download_includes]}")
        print(f" - SDL2 libraries: {('missing', 'not missing')[not download_libs]}")
        print(f" - SDL2 DLLs:      {('missing', 'not missing')[not download_dlls]}")
        print()

    if download_includes or download_libs or download_dlls:
        SDL2_VER = "2.26.3"
        SDL2_URL = f"https://github.com/libsdl-org/SDL/releases/download/release-{SDL2_VER}/SDL2-devel-{SDL2_VER}-mingw.tar.gz"

        print(f"Downloading SDL2-{SDL2_VER}")

        clean_tmp()

        res = requests.get(SDL2_URL)
        if res.status_code >= 300:
            raise Exception(f"Downloading SDL2 devel package failed. {SDL2_URL} returned code {res.status_code}")

        with io.BytesIO(res.content) as fileobj:
            with tarfile.open(mode="r:gz", fileobj=fileobj) as tarred:
                tarred.extractall(SDL2_TMP_PATH)

        TTF_VER = "2.20.2"
        TTF_URL = f"https://github.com/libsdl-org/SDL_ttf/releases/download/release-{TTF_VER}/SDL2_ttf-devel-{TTF_VER}-mingw.tar.gz"

        print(f"Downloading SDL2_ttf-{TTF_VER}")

        res = requests.get(TTF_URL)
        if res.status_code >= 300:
            raise Exception(f"Downloading SDL2_ttf devel package failed. {TTF_URL} returned code {res.status_code}")

        with io.BytesIO(res.content) as fileobj:
            with tarfile.open(mode="r:gz", fileobj=fileobj) as tarred:
                tarred.extractall(TTF_TMP_PATH)

        SDL_DIR = SDL2_TMP_PATH / f"SDL2-{SDL2_VER}"
        TTF_DIR = TTF_TMP_PATH / f"SDL2_ttf-{TTF_VER}"

        if platform.machine().endswith("64"):
            SDL_DIR /= "x86_64-w64-mingw32"
            TTF_DIR /= "x86_64-w64-mingw32"
        else:
            SDL_DIR /= "i686-w64-mingw32"
            TTF_DIR /= "i686-w64-mingw32"

        # Extract include headers to script directory
        if download_includes:
            print(f"Extracting SDL2-{SDL2_VER}/include/SDL2/")
            shutil.copytree(SDL_DIR / "include", INCLUDE_PATH)

            print(f"Extracting SDL2_ttf-{TTF_VER}/include/SDL2/SDL_ttf.h")
            shutil.copyfile(TTF_DIR / "include" / "SDL2" / "SDL_ttf.h", INCLUDE_PATH / "SDL2" / "SDL_ttf.h")

        # Extract libraries to script directory
        if download_libs:
            print(f"Extracting SDL2-{SDL2_VER}/lib/")
            shutil.copytree(SDL_DIR / "lib", "lib")

            print(f"Extracting SDL2_ttf-{TTF_VER}/lib/")
            copytree(TTF_DIR / "lib", "lib")

        # Extract DLLs to script directory
        if download_dlls:
            print(f"Extracting SDL2-{SDL2_VER}/bin/SDL2.dll")
            shutil.copyfile(SDL_DIR / "bin" / "SDL2.dll", "SDL2.dll")

            print(f"Extracting SDL2_ttf-{TTF_VER}/bin/SDL2_ttf.dll")
            shutil.copyfile(TTF_DIR / "bin" / "SDL2_ttf.dll", "SDL2_ttf.dll")

        print()


    print("Cleaning up\n")
    clean_tmp()


if platform.system() == "Windows": BINARY = "nova_example.exe"
else: BINARY = "nova_example"

if os.path.exists(BINARY):
    os.remove(BINARY)


source_files = [f"{example}.c",]

for *_, files in os.walk(NOVA_SRC_PATH):
    for name in files:
        source_files.append(NOVA_SRC_PATH / name)

source_files = [str(f) for f in source_files]

source_files_arg = " ".join(source_files)
includes = "-I../include/" if benchmark else "-I../include/ -I./include/"
libs = "" if benchmark else "-L./lib/ -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf"
args = "-g" if debug else "-O3"

print("Compilation started")

out = subprocess.run(f"gcc -o {BINARY} {source_files_arg} {includes} {libs} {args}", shell=True)

print("Compilation done with code", out.returncode, "\n")


if os.path.exists(BINARY):
    if benchmark:
        if benchmark_arg is not None:
            entry = f"{BINARY} {benchmark_arg}"
        else:
            entry = BINARY

        start = time.time()
        out = subprocess.run(entry)
        elapsed = time.time() - start
        print(f"\nBenchmark ran in {elapsed} seconds")

        if remove_binary:
            os.remove(BINARY)

    else:
        out = subprocess.run("./" + BINARY)
        print(f"Example exited with code", out.returncode)

        if remove_binary:
            os.remove(BINARY)