"""

    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/nova-physics

"""

import sys
import re
from pathlib import Path


def change_occurences(version: str) -> None:
    """
    Change occurences of versions in the project directory.

    Parameters
    ----------
    version
        Version string must be in MAJOR.MINOR.PATCH format.
    """

    match = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", version)

    if not match:
        raise ValueError(
            f"Invalid version '{version}'. "
            "Expected format: MAJOR.MINOR.PATCH (e.g. 1.2.3)."
        )

    major, minor, patch = match.groups()
    root = Path.cwd()

    # README.md: shields.io version badge
    readme = root / "README.md"
    text = readme.read_text(encoding="utf-8")
    text, count = re.subn(
        r"(version-)\d+\.\d+\.\d+(-yellow)",
        rf"\g<1>{version}\g<2>",
        text,
    )
    if count == 0:
        raise Exception("Could not find version badge in README.md")

    readme.write_text(text, encoding="utf-8")
    print("Wrote to README.md")

    # meson.build: static library version
    meson = root / "meson.build"
    text = meson.read_text(encoding="utf-8")
    text, count = re.subn(
        r"(\bversion\s*:\s*')[^']*(')",
        rf"\g<1>{version}\g<2>",
        text,
        count=1,
    )
    if count == 0:
        raise Exception("Could not find version declaration in meson.build")

    meson.write_text(text, encoding="utf-8")
    print("Wrote to meson.build")

    # version.h
    header = root / "include" / "novaphysics" / "version.h"
    text = header.read_text(encoding="utf-8")

    for name, value in (
        ("MAJOR", major),
        ("MINOR", minor),
        ("PATCH", patch),
    ):
        text, count = re.subn(
            rf"(^[ \t]*#define[ \t]+NV_VERSION_{name}[ \t]+)\d+",
            rf"\g<1>{value}",
            text,
            count=1,
            flags=re.MULTILINE,
        )
        if count == 0:
            raise Exception(
                f"Could not find NV_VERSION_{name} in version.h"
            )

    header.write_text(text, encoding="utf-8")
    print("Wrote to version.h")

    print(f"Succesfully changed project version to '{version}'.")


if __name__ == "__main__":
    args = sys.argv[1:]

    if len(args) == 0:
        print("No version is given, exiting...")
        sys.exit(0)

    version = args[0]
    print(f"Changing all version occurences with '{version}'...")

    change_occurences(version)