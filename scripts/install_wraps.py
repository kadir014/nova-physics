"""

    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/nova-physics

"""

import subprocess
import os


if not os.path.exists("subprojects"):
    os.mkdir("subprojects")


WRAPS = [
    "sdl2",
]

for wrap in WRAPS:
    print(f"[MESON HELPER] Installing wrap: {wrap}")
    subprocess.run(f"meson wrap install {wrap}", shell=True)