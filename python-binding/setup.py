"""

    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/nova-physics


    Nova Physics Engine Python Binding Builder
    --------------------------------------
    This Python script is used to build Python binding
    of the Nova Physics Engine.

"""


import os
import pathlib
import shutil
from setuptools import setup, Extension


BASE_PATH = pathlib.Path(os.getcwd())
NOVA_PATH = BASE_PATH.parent
SRC_PATH = NOVA_PATH / "src"
INCLUDE_PATH = NOVA_PATH / "include"


if os.path.exists("build"):
    shutil.rmtree("build")


# Gather Nova Physics & Python binding source files
source_files = []

for *_, files in os.walk(SRC_PATH):
    for name in files:
        source_files.append(SRC_PATH / name)

for *_, files in os.walk(BASE_PATH):
    for name in files:
        if name.endswith(".c"):
            source_files.append(BASE_PATH / name)

source_files = [str(f) for f in source_files]


extension = Extension(
    "nova",
    source_files,
    include_dirs = [INCLUDE_PATH]
)


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


setup(
    name = "nova",
    version = VER_STR,
    description = "Nova Physics Engine",
    ext_modules = [extension]
)


shutil.copyfile(BASE_PATH / "build" / "lib.win-amd64-3.10" / "nova.cp310-win_amd64.pyd", BASE_PATH / "nova.pyd")