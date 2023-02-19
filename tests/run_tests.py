"""

    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/nova-physics


    Nova Physics Engine Unit Tests Runner
    -------------------------------------
    This Python script is used to test various Nova Physics engine
    functions and utilities using `tests.c`

"""

import os
import subprocess
import platform
import pathlib
from time import perf_counter


BASE_PATH = pathlib.Path(os.getcwd())
NOVA_PATH = BASE_PATH.parent
SRC_PATH = NOVA_PATH / "src"
INCLUDE_PATH = NOVA_PATH / "include"


if platform.system() == "Windows":
    binaryfile = BASE_PATH / "tests.exe"
    binaryrun = BASE_PATH / "tests"
else:
    binaryfile = BASE_PATH / "tests"
    binaryrun = BASE_PATH / "./tests"


print("Running tests...")

if os.path.exists(binaryfile): os.remove(binaryfile)

source_files = ["tests.c",]

for *_, files in os.walk(SRC_PATH):
   for name in files:
      source_files.append(SRC_PATH / name)

source_files = [str(f) for f in source_files]

source_files_arg = " ".join(source_files)
args = "-O3"
if platform.system() != "Windows": args += " -lm"


comp = subprocess.run(f"gcc -o tests {source_files_arg} -I{INCLUDE_PATH} {args}")

if comp.returncode != 0:
    print(f"Compilation exited with code {comp.returncode}")
    raise SystemError(0)


start = perf_counter()
out = subprocess.check_output(binaryrun).decode("utf-8").replace("\r", "")
end = perf_counter() - start

outs = out.split("\n")
for i, l in enumerate(outs):

    if l.startswith("[PASSED]"):
        outs[i] = "[\033[92mPASSED\033[0m]" + l[8:]

    elif l.startswith("[FAILED]"):
        outs[i] = "[\033[91mFAILED\033[0m]" + l[8:]

    elif l.startswith("total:"):
        test_count = int(l[6:])

    elif l.startswith("fails:"):
        fail_count = int(l[6:])

outs = outs[:-3]

# Windows issues :(
if platform.system == "Windows": os.system(" ")

first_line = f"Ran {test_count} tests in {round(end, 3)} secs"
print("\033[1F\033[1F")
print(first_line)
print(f"{fail_count} {('fail', 'fails')[bool(fail_count-1)]}")
print("-" * len(first_line))
print("\n".join(outs))