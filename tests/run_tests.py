"""

    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/Nova-Physics


    Nova Physics Engine Unit Tests Runner
    -------------------------------------
    This Python script is used to test various Nova Physics engine
    functions and utilities using `tests.c`

"""

import os
import subprocess
import platform
from time import perf_counter


if platform.system() == "Windows":
    binaryfile = "tests.exe"
    binaryrun = "tests"
else:
    binaryfile = "tests"
    binaryrun = "./tests"


print("Running tests...")

if os.path.exists(binaryfile): os.remove(binaryfile)

source_files = (
    "tests.c",
    "../src-c/vector.c",
    "../src-c/body.c",
    "../src-c/math.c",
    "../src-c/space.c",
    "../src-c/collision.c",
    "../src-c/contact.c",
    "../src-c/solver.c",
)

if platform.system() == "Windows":
    comp = subprocess.run(f"gcc -o tests {' '.join(source_files)} -I../include/")
else:
    comp = subprocess.run(f"gcc -o tests {' '.join(source_files)} -I../include/ -lm")


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