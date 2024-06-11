import os
import shutil


if not os.path.exists("build/assets"):
    print("Copying assets directory.")
    shutil.copytree("examples/assets", "build/assets", dirs_exist_ok=True)