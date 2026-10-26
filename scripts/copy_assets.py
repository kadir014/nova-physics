"""

    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/nova-physics

"""

import os
import shutil


VALID_BUILD_DIRS = [
    "build",
    "build_debug",
    "build_web"
]

for build_dir in VALID_BUILD_DIRS:
    assets_dir = f"{build_dir}/assets"
    if os.path.exists(build_dir) and not os.path.exists(assets_dir):
        print(f"[MESON HELPER] Copying assets directory into '{assets_dir}'")
        shutil.copytree("examples/assets", assets_dir, dirs_exist_ok=True)