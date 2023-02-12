"""

    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/nova-physics


    Nova Physics Engine Build Script
    --------------------------------
    This Python script is used to build Nova Physics Engine
    library files ready to release
    
    Script assumes MinGW binaries are on PATH for Windows.
    Just simply run the script with 'python build.py', (or 'py',
    'python3' depending on the platform.)

"""

import os
import subprocess
import pathlib
import shutil


def remove_object_files():
    for object_file in object_files:
        if os.path.exists(object_file):
            os.remove(object_file)


BASE_PATH = pathlib.Path(os.getcwd())
SRC_PATH = BASE_PATH / "src"
INCLUDE_PATH = BASE_PATH / "include"

GCC_ARGS = [
    "-O3",
]

# Get version numbers
with open(INCLUDE_PATH / "novaphysics" / "novaphysics.h", "r") as header_file:
    content = header_file.readlines()
    MAJOR, MINOR, PATCH = 0, 0, 0

    for line in content:
        if line.startswith("#define NV_VERSION_MAJOR"):
            MAJOR = int(line[24:].strip())

        elif line.startswith("#define NV_VERSION_MINOR"):
            MINOR = int(line[24:].strip())

        elif line.startswith("#define NV_VERSION_PATCH"):
            PATCH = int(line[24:].strip())
        
VER_STR = f"{MAJOR}.{MINOR}.{PATCH}"


source_files = []
object_files = []

for *_, files in os.walk(SRC_PATH):
   for name in files:
      source_files.append(SRC_PATH / name)
      object_files.append(BASE_PATH / (name[:-2] + ".o"))

source_files = [str(f) for f in source_files]
object_files = [str(f) for f in object_files]


if __name__ == "__main__":
    print("Generating library for x86")
    # Generate object files for x86
    out = subprocess.run(f"gcc -c -m32 {' '.join(source_files)} {' '.join(GCC_ARGS)} -I{INCLUDE_PATH}")
    if out.returncode != 0:
        print("Compiling object files for x86 exited with code:", out.returncode)
        raise SystemExit(out.returncode)

    # Generate library file from object files for x86
    out = subprocess.run(f"ar rc libnova_x86.a {' '.join(object_files)}")
    if out.returncode != 0:
        print("Generating library for x86 exited with code:", out.returncode)
        raise SystemExit(out.returncode)

    remove_object_files()

    print("Generating library for x86_64")
    # Generate object files for x86_64
    out = subprocess.run(f"gcc -c {' '.join(source_files)} {' '.join(GCC_ARGS)} -I{INCLUDE_PATH}")
    if out.returncode != 0:
        print("Compiling object files for x86_64 exited with code:", out.returncode)
        raise SystemExit(out.returncode)

    # Generate library file from object files for x86_64
    out = subprocess.run(f"ar rc libnova_x86_64.a {' '.join(object_files)}")
    if out.returncode != 0:
        print("Generating library for x86_64 exited with code:", out.returncode)
        raise SystemExit(out.returncode)

    ZIP_DIR = BASE_PATH / "zip_dir"
    TARGZ_DIR = BASE_PATH / "targz_dir"

    print("Building temporary archive directories")

    os.mkdir(ZIP_DIR)
    os.mkdir(TARGZ_DIR)
    os.mkdir(ZIP_DIR / "lib")
    os.mkdir(TARGZ_DIR / "lib")
    os.mkdir(ZIP_DIR / "lib" / "x86")
    os.mkdir(TARGZ_DIR / "lib" / "x86")
    os.mkdir(ZIP_DIR / "lib" / "x86_64")
    os.mkdir(TARGZ_DIR / "lib" / "x86_64")

    # Copy include directory
    shutil.copytree(INCLUDE_PATH, ZIP_DIR / "include")
    shutil.copytree(INCLUDE_PATH, TARGZ_DIR / "include")

    # Copy library files
    shutil.copyfile(BASE_PATH / "libnova_x86.a", ZIP_DIR / "lib" / "x86" / "libnova.a")
    shutil.copyfile(BASE_PATH / "libnova_x86.a", TARGZ_DIR / "lib" / "x86" / "libnova.a")
    shutil.copyfile(BASE_PATH / "libnova_x86_64.a", ZIP_DIR / "lib" / "x86_64" / "libnova.a")
    shutil.copyfile(BASE_PATH / "libnova_x86_64.a", TARGZ_DIR / "lib" / "x86_64" / "libnova.a")

    print("Building archives")
    shutil.make_archive(f"nova-physics-{VER_STR}-devel", "zip", ZIP_DIR)
    shutil.make_archive(f"nova-physics-{VER_STR}-devel", "gztar", TARGZ_DIR)

    print("Cleaning")
    # Cleaning
    remove_object_files()

    shutil.rmtree(ZIP_DIR)
    shutil.rmtree(TARGZ_DIR)

    os.remove("libnova_x86.a")
    os.remove("libnova_x86_64.a")