"""
    This file is a part of the Nova Physics Engine
    project and distributed under the MIT license.

    Copyright © Kadir Aksoy
    https://github.com/kadir014/nova-physics


    Nova Physics Engine Build System
    --------------------------------
    Build & package Nova Physics
    Build & run examples
    Build & run benchmarks
    Build & run unit tests

    This script requires Python 3.9+ and is one-file
    with no dependencies for convenience and portability.
"""

from typing import Union, Optional

import sys
import os
import stat
import subprocess
import platform
import shutil
import tarfile
import io
import urllib.error
import urllib.request
from pathlib import Path
from enum import Enum
from time import perf_counter, time


IS_WIN = platform.system() == "Windows"


# Fix windows terminal
if IS_WIN:
    subprocess.run("", shell=True)
    # This import won't be used anywhere else
    from ctypes import windll
    k = windll.kernel32
    k.SetConsoleMode(k.GetStdHandle(-11), 7) # -11 = stdout


# ? Segfault code
# TODO: Make this actual
if IS_WIN:
    SEGFAULT_CODE = 3221225477
else:
    SEGFAULT_CODE = 139


class FG:
    """
    ANSI escape codes for terminal foreground colors
    """

    black = "\033[30m"
    darkgray = "\033[90m"
    lightgray = "\033[37m"
    white = "\033[97m"
    red = "\033[31m"
    orange = "\033[33m"
    yellow = "\033[93m"
    green = "\033[32m"
    blue = "\033[34m"
    cyan = "\033[36m"
    purple = "\033[35m"
    magenta = "\033[95m"
    lightred = "\033[91m"
    lightgreen = "\033[92m"
    lightblue = "\033[94m"
    lightcyan = "\033[96m"

    @staticmethod
    def rgb(r: int, g: int, b: int) -> str:
        return f"\033[38;2;{r};{g};{b}m"

class BG:
    """
    ANSI escape codes for terminal background colors
    """

    black = "\033[40m"
    darkgray = "\033[100m"
    lightgray = "\033[47m"
    white = "\033[107m"
    red = "\033[41m"
    orange = "\033[43m"
    yellow = "\033[103m"
    green = "\033[42m"
    blue = "\033[44m"
    cyan = "\033[46m"
    purple = "\033[45m"
    magenta = "\033[105m"
    lightred = "\033[101m"
    lightgreen = "\033[102m"
    lightblue = "\033[104m"
    lightcyan = "\033[106m"

    @staticmethod
    def rgb(r: int, g: int, b: int) -> str:
        return f"\033[48;2;{r};{g};{b}m"
    
# Other ANSI escape codes for graphic modes
# (some doesn't work on some terminals)
class Style:
    bold = "\033[01m"
    underline = "\033[04m"
    reverse = "\033[07m"
    strike = "\033[09m"

RESET = "\033[0m"

def format_colors(string: str, no_ansi: bool = False) -> str:
    """ Format color codes or remove them """

    # This could be shortened with loops but meh..
    pairs = {
        "{RESET}": RESET,
        "{FG.black}": FG.black,
        "{FG.darkgray}": FG.darkgray,
        "{FG.lightgray}": FG.lightgray,
        "{FG.white}": FG.white,
        "{FG.red}": FG.red,
        "{FG.orange}": FG.orange,
        "{FG.yellow}": FG.yellow,
        "{FG.green}": FG.green,
        "{FG.blue}": FG.blue,
        "{FG.cyan}": FG.cyan,
        "{FG.purple}": FG.purple,
        "{FG.magenta}": FG.magenta,
        "{FG.lightred}": FG.lightred,
        "{FG.lightgreen}": FG.lightgreen,
        "{FG.lightblue}": FG.lightblue,
        "{FG.lightcyan}": FG.lightcyan,
        "{BG.black}": BG.black,
        "{BG.darkgray}": BG.darkgray,
        "{BG.lightgray}": BG.lightgray,
        "{BG.white}": BG.white,
        "{BG.red}": BG.red,
        "{BG.orange}": BG.orange,
        "{BG.yellow}": BG.yellow,
        "{BG.green}": BG.green,
        "{BG.blue}": BG.blue,
        "{BG.cyan}": BG.cyan,
        "{BG.purple}": BG.purple,
        "{BG.magenta}": BG.magenta,
        "{BG.lightred}": BG.lightred,
        "{BG.lightgreen}": BG.lightgreen,
        "{BG.lightblue}": BG.lightblue,
        "{BG.lightcyan}": BG.lightcyan,
        "{Style.reverse}": Style.reverse
    }

    for key in pairs:
        if no_ansi:
            string = string.replace(key, pairs[key])

        else:
            string = string.replace(key, "")

    return string


def error(messages: Union[list, str], no_color: bool = False):
    """ Log error message and abort """

    if isinstance(messages, str):
        msg = f"{{FG.red}}{{Style.reverse}} FAIL {{RESET}} {messages}\n"
        print(format_colors(msg, no_color))
        raise SystemExit(1)
    
    else:
        msg = f"{{FG.red}}{{Style.reverse}} FAIL {{RESET}} {messages[0]}\n"

        for line in messages[1:]:
            msg += f"       {line}\n"

        print(format_colors(msg, no_color))

        raise SystemExit(1)
    
def success(messages: Union[list, str], no_color: bool = False):
    """ Log success message """

    if isinstance(messages, str):
        msg = f"{{FG.lightgreen}}{{Style.reverse}} DONE {{RESET}} {messages}\n"
        print(format_colors(msg, no_color))
    
    else:
        msg = f"{{FG.lightgreen}}{{Style.reverse}} DONE {{RESET}} {messages[0]}\n"

        for line in messages[1:]:
            msg += f"       {line}\n"

        print(format_colors(msg, no_color))

def info(messages: Union[list, str], no_color: bool = False):
    """ Log information message """

    if isinstance(messages, str):
        msg = f"{{FG.cyan}}{{Style.reverse}} INFO {{RESET}} {messages}"
        print(format_colors(msg, no_color))

    else:
        msg = f"{{FG.cyan}}{{Style.reverse}} INFO {{RESET}} {messages[0]}"

        for line in messages[1:]:
            msg += f"       {line}\n"

        print(format_colors(msg, no_color))

# Fancy CLI stuff
# can be made more readable but its just mostly hardcoded strings

nova_logo_title= \
"{RESET}{FG.purple}Nova {FG.magenta}Phys{FG.lightblue}ics E{FG.blue}ngine Build System"

nova_logo_repo = "{FG.cyan}https://github.com/kadir014/nova-physics{RESET}"

nova_logo =                                                               \
"\n{FG.magenta}            ...:::::.{FG.blue}.\n"                       + \
"{FG.magenta}        .:::--------{FG.blue}----:.\n"                     + \
"{FG.magenta}      .:::::-------{FG.blue}------=:\n"                    + \
"{FG.magenta}     ::::::...... {FG.blue}..::---==-\n"                   + \
"{FG.magenta}    .:::: {FG.blue}.:-=====:{FG.blue}  . .-===\n"          + \
"{FG.magenta}.   :::. {FG.blue}-=========:{FG.blue} --. .==    title\n" + \
"{FG.magenta}::   :: {FG.blue}:=========-{FG.blue} .---  .:    repo\n"  + \
"{FG.magenta}:::.  .  {FG.blue}-======-.{FG.blue} :----    \n"          + \
"{FG.magenta}::::::..   ......{FG.blue}:----- \n"                       + \
"{FG.magenta} ::::::::------{FG.blue}------:\n"                         + \
"{FG.magenta}  .::::::---{FG.blue}-------.\n"                           + \
"{FG.magenta}      ..::{FG.blue}:::::..{RESET}\n"
nova_logo = nova_logo                  \
    .replace("title", nova_logo_title) \
    .replace("repo", nova_logo_repo)


def get_output(cmd: str) -> str:
    """ Run check_output, return empty string on error. """
    try:
        return subprocess.check_output(cmd, shell=True).decode("utf-8").strip()
    except:
        return ""


# Define paths

BASE_PATH = Path(os.getcwd())

SRC_PATH = BASE_PATH / "src"
INCLUDE_PATH = BASE_PATH / "include"

BUILD_PATH = BASE_PATH / "build"
DEPS_PATH = BASE_PATH / "deps"

EXAMPLES_PATH = BASE_PATH / "examples"
BENCHS_PATH = BASE_PATH / "benchmarks"
TESTS_PATH = BASE_PATH / "tests"


class Platform:
    """
    Platform specific information.

    Attributes
    ----------
    is_64 Whether the system architechure is 64-bit or not.
    system Most basic name that represents the system
    name Name of the system
    min_name Shortened name (without no release versions, etc..)

    Possible configuration examples on different platforms to see the variation:

    Attribute | Windows         | Manjaro    | Ubuntu       | MacOS  | Other Linux
    ----------+-----------------+------------+--------------+--------+------------------
    system    | Windows         | Linux      | Linux        | Darwin | Linux
    name      | Windows 10 Home | Manjaro 23 | Ubuntu 20.04 | ?      | release + version
    min_name  | Windows         | Manjaro    | Ubuntu       | ?      | Linux
    """

    def __init__(self):
        self.is_64 = "64" in platform.architecture()[0]

        self.system = platform.system()

        if IS_WIN:
            # Some possible edition outputs:
            # Core, CoreSingleLanguage, Ultimate
            edition = platform.win32_edition()

            if "Core" in edition:
                edition = "Home"

            self.name = f"Windows {platform.release()} {edition}"
            self.min_name = "Windows"

        else:
            # These will be overwritten with gathered info
            self.name = platform.release() + platform.version()
            self.min_name = f"Unknown Linux"
            
            lsb_fields = self.parse_lsb_release()

            # Manjaro Linux
            if "manjaro" in platform.platform().lower():
                self.name = "Manjaro"
                if "Release" in lsb_fields: self.name += f" {lsb_fields['Release']}"
                self.min_name = "Manjaro"

            # Ubuntu
            elif "ubuntu" in platform.version().lower():
                self.name = "Ubuntu"
                self.min_name = "Ubuntu"

            else:
                os_release = self.parse_os_release()

                if "ID" in os_release:
                    # Fedora
                    if os_release["ID"].lower() == "fedora":
                        self.min_name = "Fedora"
                        if "PRETTY_NAME" in os_release: self.name = os_release["PRETTY_NAME"]
                        else: self.name = self.min_name

    @staticmethod
    def parse_lsb_release() -> dict:
        """
        Parse "lsb_release -a" command that is found in most Linux distros
        by default to get more detailed system information.
        """
        
        out = get_output("lsb_release -a")
        
        if len(out) == 0: return {}

        fields = {}

        for line in out.split("\n"):
            split = line.split(":")
            if len(split) <= 1: continue
            fields[split[0].strip()] = split[1].strip()
        
        return fields
    
    @staticmethod
    def parse_os_release() -> dict:
        """
        Parse "cat /etc/os_release" command that is found in most Linux distros
        to get more detailed system information, in case lsb_release doesn't exist.
        """

        out = get_output("cat /etc/os-release")
        
        if len(out) == 0: return {}

        fields = {}

        for line in out.split("\n"):
            split = line.split("=")
            if len(split) <= 1: continue
            fields[split[0].strip()] = split[1].strip()
        
        return fields


PLATFORM = Platform()


def get_nova_version() -> str:
    """ Get Nova Phyiscs Engine version number """
    
    with open(INCLUDE_PATH / "novaphysics" / "novaphysics.h", "r") as header_file:
        content = header_file.readlines()
        major, minor, patch = 0, 0, 0

        for line in content:
            if line.startswith("#define NV_VERSION_MAJOR"):
                major = int(line[24:].strip())

            elif line.startswith("#define NV_VERSION_MINOR"):
                minor = int(line[24:].strip())

            elif line.startswith("#define NV_VERSION_PATCH"):
                patch = int(line[24:].strip())
            
    return f"{major}.{minor}.{patch}"


# Useful file & directory methods

def remove_readonly(action, name, exc):
    """ Overwrites READ-ONLY files as WRITE-ONLY and removes them """
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

def remove_dir(path: Path):
    """ Remove directory with its contents recursively """
    shutil.rmtree(path, onerror=remove_readonly)

def copy_dir(
        src_path: Path,
        dst_path: Path,
        symlinks: bool = False,
        ignore: bool = None
        ):
    """ Copy one directory's content into another one recursively """
    
    for item in os.listdir(src_path):
        try:
            s = os.path.join(src_path, item)
            d = os.path.join(dst_path, item)
            if os.path.isdir(s):
                shutil.copytree(s, d, symlinks, ignore)
            else:
                shutil.copy2(s, d)
        except:
            pass


class DependencyManager:
    """
    Class to download & manage build dependencies
    """

    def __init__(self, cli: "CLIHandler"):
        self.cli = cli
        self.no_color = not self.cli.get_option("-n")

        # Library and DLL files are Windows only
        # TODO: Make this system more flexible to allow various dependencies
        self.dependencies = {
            "SDL2": {
                "include": {
                    "satisfied": False,
                    "path": DEPS_PATH / "include" / "SDL2"
                },

                "lib": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "SDL2_lib"
                },

                "dll": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "SDL2.dll"
                }
            },

            "SDL2_ttf": {
                "include": {
                    "satisfied": False,
                    "path": DEPS_PATH / "include" / "SDL2" / "SDL_ttf.h"
                },

                "lib": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "SDL2_ttf_lib"
                },

                "dll": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "SDL2_ttf.dll"
                }
            },

            "SDL2_image": {
                "include": {
                    "satisfied": False,
                    "path": DEPS_PATH / "include" / "SDL2" / "SDL_image.h"
                },

                "lib": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "SDL2_image_lib"
                },

                "dll": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "SDL2_image.dll"
                }
            }
        }

    def download(self, url: str) -> bytes:
        """ Download & return the data"""
        
        try:
            response = urllib.request.urlopen(url)
        
        except urllib.error.HTTPError as e:
            error(
                f"{{FG.lightcyan}}{url}{{RESET}} responsed with code {{FG.lightred}}{e.code}{{RESET}}.",
                self.no_color
            )

        return response.read()
    
    def extract(self, data: bytes, path: Path):
        """ Extract targz data to path """

        with tarfile.open(mode="r:gz", fileobj=io.BytesIO(data)) as tar:
            tar.extractall(path)

    def check(self):
        """ Check if dependencies satisfy """

        for dep in self.dependencies:
            for pack in self.dependencies[dep]:
                if os.path.exists(self.dependencies[dep][pack]["path"]):
                    self.dependencies[dep][pack]["satisfied"] = True
        
    def satisfied(self, dep_: Optional[str] = None) -> int:
        """ Return number of missing dependencies """

        deps = 0

        for dep in self.dependencies:
            if dep_ is not None and dep != dep_: continue 

            for pack in self.dependencies[dep]:
                if not self.dependencies[dep][pack]["satisfied"]:
                    deps += 1

        return deps
    
    def satisfy(self):
        """ Download, extract and organize all dependencies """

        # Don't need to continue if all deps are satisfied
        if self.satisfied() == 0: return

        if not os.path.exists(DEPS_PATH):
            os.mkdir(DEPS_PATH)

        if not os.path.exists(DEPS_PATH / "include"):
            os.mkdir(DEPS_PATH / "include")


        start = time()


        SDL2_TEMP = DEPS_PATH / "_sdl2_temp"
        TTF_TEMP = DEPS_PATH / "_ttf_temp"
        IMAGE_TEMP = DEPS_PATH / "_image_temp"

        # Clear temporary directories
        if os.path.exists(SDL2_TEMP): remove_dir(SDL2_TEMP)
        if os.path.exists(TTF_TEMP): remove_dir(TTF_TEMP)
        if os.path.exists(IMAGE_TEMP): remove_dir(IMAGE_TEMP)


        # Don't forget to update versions
        SDL2_VER = "2.26.3"
        TTF_VER = "2.20.2"
        IMAGE_VER = "2.6.3"


        # Download development packages

        SDL2_DEVEL = f"https://github.com/libsdl-org/SDL/releases/download/release-{SDL2_VER}/SDL2-devel-{SDL2_VER}-mingw.tar.gz"
        TTF_DEVEL = f"https://github.com/libsdl-org/SDL_ttf/releases/download/release-{TTF_VER}/SDL2_ttf-devel-{TTF_VER}-mingw.tar.gz"
        IMAGE_DEVEL = f"https://github.com/libsdl-org/SDL_image/releases/download/release-{IMAGE_VER}/SDL2_image-devel-{IMAGE_VER}-mingw.tar.gz"

        if self.satisfied("SDL2") > 0:
            info(f"Downloading {{FG.lightcyan}}{SDL2_DEVEL}{{RESET}}", self.no_color)

            self.extract(
                self.download(SDL2_DEVEL),
                SDL2_TEMP
            )

        if self.satisfied("SDL2_ttf") > 0:
            info(f"Downloading {{FG.lightcyan}}{TTF_DEVEL}{{RESET}}", self.no_color)

            self.extract(
                self.download(TTF_DEVEL),
                TTF_TEMP
            )

        if self.satisfied("SDL2_image") > 0:
            info(f"Downloading {{FG.lightcyan}}{IMAGE_DEVEL}{{RESET}}", self.no_color)

            self.extract(
                self.download(IMAGE_DEVEL),
                IMAGE_TEMP
            )

        if PLATFORM.is_64:
            SDL2_ARCH = SDL2_TEMP / f"SDL2-{SDL2_VER}" / "x86_64-w64-mingw32"
            TTF_ARCH = TTF_TEMP / f"SDL2_ttf-{TTF_VER}" / "x86_64-w64-mingw32"
            IMAGE_ARCH = IMAGE_TEMP / f"SDL2_image-{IMAGE_VER}" / "x86_64-w64-mingw32"
        else:
            SDL2_ARCH = SDL2_TEMP / f"SDL2-{SDL2_VER}" / "i686-w64-mingw32"
            TTF_ARCH = TTF_TEMP / f"SDL2_ttf-{TTF_VER}" / "i686-w64-mingw32"
            IMAGE_ARCH = TTF_TEMP / f"SDL2_image-{IMAGE_VER}" / "i686-w64-mingw32"

        # Satisfy includes first since they are needed on all platforms

        if not self.dependencies["SDL2"]["include"]["satisfied"]:
            info(
                f"Extracting {{FG.yellow}}{SDL2_ARCH}/include/SDL2/{{RESET}}",
                self.no_color
            )

            if not os.path.exists(self.dependencies["SDL2"]["include"]["path"]):
                os.mkdir(self.dependencies["SDL2"]["include"]["path"])

            copy_dir(
                SDL2_ARCH / "include" / "SDL2",
                self.dependencies["SDL2"]["include"]["path"]
            )

        if not self.dependencies["SDL2_ttf"]["include"]["satisfied"]:
            info(
                f"Extracting {{FG.yellow}}{TTF_ARCH}/include/SDL2/SDL_ttf.h{{RESET}}",
                self.no_color
            )

            if not os.path.exists(self.dependencies["SDL2"]["include"]["path"]):
                os.mkdir(self.dependencies["SDL2"]["include"]["path"])

            shutil.copyfile(
                TTF_ARCH / "include" / "SDL2" / "SDL_ttf.h",
                self.dependencies["SDL2_ttf"]["include"]["path"]
            )

        if not self.dependencies["SDL2_image"]["include"]["satisfied"]:
            info(
                f"Extracting {{FG.yellow}}{IMAGE_ARCH}/include/SDL2/SDL_image.h{{RESET}}",
                self.no_color
            )

            if not os.path.exists(self.dependencies["SDL2"]["include"]["path"]):
                os.mkdir(self.dependencies["SDL2"]["include"]["path"])

            shutil.copyfile(
                IMAGE_ARCH / "include" / "SDL2" / "SDL_image.h",
                self.dependencies["SDL2_image"]["include"]["path"]
            )


        if IS_WIN:
            # Satisfy libraries (Windows-only)

            if not self.dependencies["SDL2"]["lib"]["satisfied"]:
                info(
                    f"Extracting {{FG.yellow}}{SDL2_ARCH}/lib/{{RESET}}",
                    self.no_color
                )

                copy_dir(
                    SDL2_ARCH / "lib",
                    self.dependencies["SDL2"]["lib"]["path"]
                )

            if not self.dependencies["SDL2_ttf"]["lib"]["satisfied"]:
                info(
                    f"Extracting {{FG.yellow}}{TTF_ARCH}/lib/{{RESET}}",
                    self.no_color
                )

                copy_dir(
                    TTF_ARCH / "lib",
                    self.dependencies["SDL2_ttf"]["lib"]["path"]
                )

            if not self.dependencies["SDL2_image"]["lib"]["satisfied"]:
                info(
                    f"Extracting {{FG.yellow}}{IMAGE_ARCH}/lib/{{RESET}}",
                    self.no_color
                )

                copy_dir(
                    IMAGE_ARCH / "lib",
                    self.dependencies["SDL2_image"]["lib"]["path"]
                )

            
            # Satisfy DLLs (Windows only)

            if not self.dependencies["SDL2"]["dll"]["satisfied"]:
                info(
                    f"Extracting {{FG.yellow}}{SDL2_ARCH}/bin/SDL2.dll{{RESET}}",
                    self.no_color
                )

                shutil.copyfile(
                    SDL2_ARCH / "bin" / "SDL2.dll",
                    self.dependencies["SDL2"]["dll"]["path"]
                )

            if not self.dependencies["SDL2_ttf"]["dll"]["satisfied"]:
                info(
                    f"Extracting {{FG.yellow}}{TTF_ARCH}/bin/SDL2_ttf.dll{{RESET}}",
                    self.no_color
                )

                shutil.copyfile(
                    TTF_ARCH / "bin" / "SDL2_ttf.dll",
                    self.dependencies["SDL2_ttf"]["dll"]["path"]
                )

            if not self.dependencies["SDL2_image"]["dll"]["satisfied"]:
                info(
                    f"Extracting {{FG.yellow}}{IMAGE_ARCH}/bin/SDL2_image.dll{{RESET}}",
                    self.no_color
                )

                shutil.copyfile(
                    IMAGE_ARCH / "bin" / "SDL2_image.dll",
                    self.dependencies["SDL2_image"]["dll"]["path"]
                )


        end = time() - start

        print()
        success(
            f"Downloaded & extracted all dependencies in {{FG.lightblue}}{round(end, 3)}{{RESET}} seconds.\n",
            self.no_color
        )

        # Clear temporary directories
        if os.path.exists(SDL2_TEMP): remove_dir(SDL2_TEMP)
        if os.path.exists(TTF_TEMP): remove_dir(TTF_TEMP)
        if os.path.exists(IMAGE_TEMP): remove_dir(IMAGE_TEMP)



class Compiler(Enum):
    """
    Detected compiler on the system
    """

    GCC = 0,
    MSVC = 1


class NovaBuilder:
    """
    Class to compile Nova Physics Engine
    """

    def __init__(self, cli: "CLIHandler"):
        self.cli = cli
        self.no_color = not self.cli.get_option("-n")

        self.detect_compiler()
        self.get_compiler_version()


        if IS_WIN:
            self.binary = "nova.exe"
        else:
            self.binary = "./nova"


        self.source_files = []
        self.object_files = []
        self.msvc_objects = [] # Fuck MSVC

        for *_, files in os.walk(SRC_PATH):
            for name in files:
                self.source_files.append(SRC_PATH / name)

                if self.compiler == Compiler.GCC:
                    self.object_files.append(BUILD_PATH / (name[:-2] + ".o"))

                elif self.compiler == Compiler.MSVC:
                    self.object_files.append(BUILD_PATH / (name[:-2] + ".obj"))
                    self.msvc_objects.append(BASE_PATH / (name[:-2] + ".obj"))

        self.source_files = [str(f) for f in self.source_files]
        self.object_files = [str(f) for f in self.object_files]

    def remove_object_files(self):
        """ Remove object files left after compiling """

        for object_file in self.object_files:
            if os.path.exists(object_file):
                os.remove(object_file)

        if self.compiler == Compiler.MSVC:
            for object_file in self.msvc_objects:
                if os.path.exists(object_file):
                    os.remove(object_file)

    def detect_compiler(self):
        """ Detect which compiler is available on system """

        COMMON_DEV_PROMPT_PATHS = (
            "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2022/Community/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2022/Professional/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2022/Professional/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2022/Enterprise/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2019/Community/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2019/Professional/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2019/Professional/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2019/Enterprise/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2017/Community/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2017/Professional/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2017/Enterprise/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2017/Enterprise/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2015/Community/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2015/Community/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2015/Professional/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2015/Professional/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files/Microsoft Visual Studio/2015/Enterprise/Common7/Tools/VsDevCmd.bat",
            "C:/Program Files (x86)/Microsoft Visual Studio/2015/Enterprise/Common7/Tools/VsDevCmd.bat",
        )

        # Search for dev prompts
        dev_prompt_path = None
        for common_path in COMMON_DEV_PROMPT_PATHS:
            if os.path.exists(common_path):
                dev_prompt_path = common_path
                break

        gcc_path = shutil.which("gcc")

        # Even if MSVC exists, prefer GCC/MinGW because MSVC sucks ass
        if gcc_path is not None:
            self.compiler = Compiler.GCC
            self.compiler_cmd = "gcc"

        elif dev_prompt_path is not None:
            self.compiler = Compiler.MSVC
            self.compiler_cmd = "cl"
            self.msvc_dev_prompt = f"\"{dev_prompt_path}\""

        # If it can't find any supported compilers error and abort
        else:
            error(
                [
                    "Couldn't detect any supported C compilers on your system.",
                    "Check if your compiler is on the system PATH."
                ],
                self.no_color
            )

        self.compiler = Compiler.MSVC
        self.compiler_cmd = "cl"
        self.msvc_dev_prompt = f"\"{dev_prompt_path}\""

    def get_compiler_version(self):
        """ Query compiler version """

        if self.compiler == Compiler.GCC:
            out = subprocess.check_output("gcc -dumpfullversion -dumpversion", shell=True)
            self.compiler_version = out.decode("utf-8").replace("\n", "")

        elif self.compiler == Compiler.MSVC:
            # Bit hacky method to query MSVC version
            # https://stackoverflow.com/a/52089246
            with open("_mvsc_ver_temp.c", "w") as temp:
                temp.write("_MSC_FULL_VER")

            out = subprocess.check_output("cl /nologo /EP _mvsc_ver_temp.c", shell=True)
            out = out.decode("utf-8").replace("_mvsc_ver_temp.c", "").replace("\n", "")

            os.remove("_mvsc_ver_temp.c")

            self.compiler_version = out.strip()

    def _compile_gcc(self,
            sources: list[Path] = [],
            include: Optional[Path] = None,
            libs: Optional[Path] = None,
            links: list[str] = [],
            args: list[str] = [],
            generate_object: bool = False,
            clear: bool = False
            ):
        """ Compile with GCC """

        # Remove / create build directory
        if clear:
            if os.path.exists(BUILD_PATH):
                shutil.rmtree(BUILD_PATH)

            os.mkdir(BUILD_PATH)

        # Binary location
        binary = BUILD_PATH / self.binary

        # Source files argument
        srcs = " ".join([*[str(s) for s in sources], *self.source_files])

        # Include arguments
        inc = f"-I{INCLUDE_PATH}"
        if include is not None:
            for i in include:
                inc += f" -I{i}"

        # Library and linkage arguments
        lib = ""
        if libs is not None:
            for l in libs:
                lib += f" -L{l}"

        if not IS_WIN:
            lib += " -lm" # ? Required on Linux for math.h

        if links is None:
            links = ""
        else:
            links = " ".join(links)

        # Other arguments
        argss = ""

        # Use single-precision float?
        if self.cli.get_option("-f"):
            argss += " -DNV_USE_FLOAT"
        
        # Do not optimize if debug
        if self.cli.get_option("-g"):
            argss += " -g"

        elif self.cli.get_option("-p"):
            # -no-pie is required on some GCC versions?
            argss += " -g -pg -no-pie"

        else:
            # If optimization option doesn't exist, default to 3
            if self.cli.get_option("-O"):
                o = self.cli.get_option_arg("-O")
            else:
                o = 3
            argss += f" -O{o}"

        # Enable warnings
        if self.cli.get_option("-w"):
            argss += " -Wall"

        # Quiet compiling
        if self.cli.get_option("-q"):
            argss += " -s"

        # Show / hide console window
        if not self.cli.get_option("-c"):
            if IS_WIN:
                argss += " -mwindows"

        # Add other arguments
        for arg in args:
            argss += f" {arg}"

        if generate_object: dest = "-c"
        else: dest = f"-o {binary}"
        
        cmd = f"{self.compiler_cmd} {dest} {srcs} {inc} {lib} {links} {argss}"

        # Print the compilation command
        if self.cli.get_option("-b"):
            print(format_colors("{FG.green}Final compilation command: {FG.darkgray}(invoked by -b){RESET}", self.no_color))
            print(cmd, "\n")

        start = perf_counter()
        out = subprocess.run(cmd, shell=True)
        end = perf_counter()

        comp_time = end - start

        if out.returncode == 0:
            success(f"Compilation is done in {{FG.blue}}{round(comp_time, 3)}{{RESET}} seconds.", self.no_color)

        else:
            # Print blank line because of compiler error message
            print()
            error(f"Compilation failed with return code {out.returncode}.", self.no_color)

    def _compile_msvc(self,
            sources: list[Path] = [],
            include: Optional[Path] = None,
            libs: Optional[Path] = None,
            links: list[str] = [],
            args: list[str] = [],
            generate_object: bool = False,
            clear: bool = False
            ):
        """ Compile with MSVC """

        # Remove / create build directory
        if clear:
            if os.path.exists(BUILD_PATH):
                shutil.rmtree(BUILD_PATH)

            os.mkdir(BUILD_PATH)

        # Binary location
        binary = BUILD_PATH / self.binary

        # Source files argument
        srcs = " ".join([*[str(s) for s in sources], *self.source_files])

        # Include arguments
        inc = f"-I{INCLUDE_PATH}"
        if include is not None:
            for i in include:
                inc += f" -I{i}"

        # Library and linkage arguments
        lib = ""
        if libs is not None:
            for l in libs:
                lib += f" /LIBPATH:\"{l}\""

        if not IS_WIN:
            lib += " -lm" # ? Required on Linux for math.h

        if links is None:
            links = ""
        else:
            links = " ".join(links)

        # Other arguments
        argss = ""

        # Use single-precision float?
        if self.cli.get_option("-f"):
            argss += " /DNV_USE_FLOAT"
        
        # Do not optimize if debug
        if self.cli.get_option("-g"):
            # no -g similar option in MSVC
            pass

        elif self.cli.get_option("-p"):
            # no profiling option in MSVC
            pass

        else:
            # If optimization option doesn't exist, default to 3
            if self.cli.get_option("-O"):
                o = self.cli.get_option_arg("-O")
            else:
                o = 3

            if o == 1 or o == 2:
                argss += " /O{o}"
            elif o == 3:
                argss += f" /Ox /GL"

        # Enable warnings
        if self.cli.get_option("-w"):
            argss += " /W3"

        # Quiet compiling
        if self.cli.get_option("-q"):
            # no option for silencing output in MSVC
            pass

        # Show / hide console window
        if not self.cli.get_option("-c"):
            # no option for this in MSVC
            pass

        # Add other arguments
        for arg in args:
            argss += f" {arg}"

        if generate_object: dest = "/c"
        else: dest = f"/Fe{binary}"
        
        cmd = fr"{self.msvc_dev_prompt} & {self.compiler_cmd} /nologo {dest} {srcs} {inc} {argss} /link {lib} {links}"

        # Print the compilation command
        if self.cli.get_option("-b"):
            print(format_colors("{FG.green}Final compilation command: {FG.darkgray}(invoked by -b){RESET}", self.no_color))
            print(cmd, "\n")

        start = perf_counter()
        out = subprocess.run(cmd, shell=True)
        end = perf_counter()

        comp_time = end - start

        self.remove_object_files()

        if out.returncode == 0:
            success(f"Compilation is done in {{FG.blue}}{round(comp_time, 3)}{{RESET}} seconds.", self.no_color)

        else:
            # Print blank line because of compiler error message
            print()
            error(f"Compilation failed with return code {out.returncode}.", self.no_color)

    def compile(self,
            sources: list[Path] = [],
            include: Optional[Path] = None,
            libs: Optional[Path] = None,
            links: list[str] = [],
            args: list[str] = [],
            generate_object: bool = False,
            clear: bool = False
            ):
        """ Compile """

        if self.compiler == Compiler.GCC:
            self._compile_gcc(sources, include, libs, links, args, generate_object, clear)

        elif self.compiler == Compiler.MSVC:
            self._compile_msvc(sources, include, libs, links, args, generate_object, clear)

    def build_library(self, library_path: Path):
        """ Build static library from objects files. """

        if self.compiler == Compiler.GCC:
            out = subprocess.run(f"ar rc {str(library_path) + '.a'} {' '.join(self.object_files)}", shell=True)

        elif self.compiler == Compiler.MSVC:
            out = subprocess.run(f"lib /NOLOGO /OUT:{str(library_path) + '.lib'} {' '.join(self.object_files)}", shell=True)

        return out


class CLIHandler:
    """
    Class to handle command line flags and commands to simplify building
    """

    def __init__(self):
        self.opts = {}
        self.cmds = {}
        self.args = []

    def add_option(
            self,
            option: Union[str, tuple],
            doc: str,
            arg: Optional[str] = None
            ):
        """ Add an option (flag) """

        # 3rd field is option argument
        if isinstance(option, tuple):
            self.opts[option] = [doc, False, arg]
        else:
            self.opts[option[:2]] = [doc, False, arg]

    def get_option(self, option: Union[str, tuple]) -> bool:
        """ Return if the option is used """

        for opt in self.opts:

            # Return option in -f | --flag format
            if isinstance(opt, tuple):
                if option == opt[0] or option == opt[1]:
                    return self.opts[opt][1]

            # Return option only in -f format
            else:
                if opt == option:
                    return self.opts[opt][1]
                
    def get_option_arg(self, option: str) -> str:
        """ Return option argument """

        for opt in self.opts:
            if opt == option:
                return self.opts[opt][2]                

    def _set_option(self, arg: str) -> bool:
        """ Check if there is option and set it """

        for opt in self.opts:
            if isinstance(opt, tuple):
                if arg == opt[0] or arg == opt[1]:
                    self.opts[opt][1] = True
                    return True
                
            else:
                if len(arg) > 2:
                    if opt == arg[:2]:
                        self.opts[opt][1] = True
                        self.opts[opt][2] = arg[2:]
                        return True
                else: 
                    if opt == arg:
                        self.opts[opt][1] = True
                        return True
        
        return False

    def add_command(self, command: str, doc: str):
        """ Add a command """
        self.cmds[command] = [doc, False]

    def get_command(self, command: str) -> bool:
        """ Return if the command is used """
        return self.cmds[command][1]
    
    def _set_command(self, arg: str) -> bool:
        """ Check if there is command and set it """

        for cmd in self.cmds:
                if arg == cmd:
                    self.cmds[cmd][1] = True
                    return True
                
        return False

    def build_usage(self) -> str:
        """ Generate usage manual """

        man =                                                                          \
        "{FG.orange}Usage:{RESET} nova_builder [options] <command> [arguments...]\n" + \
        "\n"                                                                         + \
        "{FG.orange}Commands:{RESET}\n"

        GAP = 20

        for command in self.cmds:
            command_l = command.ljust(GAP)
            man += f"  {{FG.lightcyan}}{command_l}{{RESET}} {self.cmds[command][0]}\n"

        man += "\n{FG.orange}Options:{RESET}\n"

        for option in self.opts:
            # Separate pairs as "-f | --flag"
            if isinstance(option, tuple):
                option_l = f"{{FG.lightcyan}}{option[0]} {{FG.darkgray}}| {{FG.lightcyan}}{option[1]}{{RESET}}"
                option_l = option_l.ljust(GAP + 48)
            else:
                option_l = f"{{FG.lightcyan}}{option.ljust(GAP)}{{RESET}}"

            man += f"  {option_l} {self.opts[option][0]}\n"

        return man
    
    def parse(self):
        """ Parse command line """
        
        # This is a very basic CLI handler so it doesn't check
        # flag arguments or multiple commands, etc..
        for arg in sys.argv[1:]:
            
            # Set option
            success = self._set_option(arg)
            if success: continue

            # Set command
            success = self._set_command(arg)
            if success: continue

            # Add extra arguments to self.args
            # they can be used as filepaths later
            self.args.append(arg)

    @property
    def command_count(self):
        return sum(1 if self.cmds[cmd][1] else 0 for cmd in self.cmds)


def main():
    """ Entry point of the CLI """

    cli = CLIHandler()

    # Add options & commands

    cli.add_command("build", "Build release-ready development library files")
    cli.add_command("example", "Run an example from ./examples/ directory")
    cli.add_command("bench", "Run a benchmark from ./benchmarks/ directory")
    cli.add_command("tests", "Run unit tests at ./tests/ directory")

    cli.add_option(("-v", "--version"), "Print Nova Physics Engine version")
    cli.add_option(("-n", "--no-color"), "Disable coloring with ANSI escape codes")
    cli.add_option(("-q", "--quiet"), "Get compiler logs as silent as possible")
    cli.add_option(("-f", "--float"), "Use single-precision floating point numbers")
    cli.add_option(("-m", "--m32"), "Build library for 32-bit platform as well")
    cli.add_option("-g", "Compile with -g flag for debugging")
    cli.add_option("-p", "Compile with -pg flag for profiling")
    cli.add_option("-b", "Print the compilation command for debugging")
    cli.add_option("-O", "Set optimization level (default is 3)", 3)
    cli.add_option("-w", "Enable all warnings")
    cli.add_option("-d", "Force download all dependencies (for examples)")
    cli.add_option("-c", "Show console window when executable is ran")

    # Parse command line
    cli.parse()

    NO_COLOR = not cli.get_option("-n")

    # Check if the working directory is correct
    if BASE_PATH.name != "nova-physics":
        error([
            "Make sure you are in the Nova Physics directory!",
            f"This script is ran at {{FG.yellow}}{BASE_PATH.absolute()}{{RESET}}"
        ], NO_COLOR
        )

    print(format_colors(nova_logo, NO_COLOR))
    print()

    # After printing version do not continue
    if cli.get_option("-v"):
        print(get_nova_version())
        return

    # If there isn't any commands, print manual and abort
    if cli.command_count == 0:
        print(format_colors(cli.build_usage(), NO_COLOR))
        return
    
    if cli.get_command("build"):
        build(cli)
    
    elif cli.get_command("example"):
        example(cli)

    elif cli.get_command("bench"):
        benchmark(cli)

    elif cli.get_command("tests"):
        print("Not Implemented!")


def build(cli: CLIHandler):
    """ Build & package Nova Physics """
    
    NO_COLOR = not cli.get_option("-n")
    BUILD_FOR_32BIT = cli.get_option("-m")

    # Compile the example

    builder = NovaBuilder(cli)

    info(
        f"Detected compiler: {{FG.yellow}}{builder.compiler.name}{{RESET}} {builder.compiler_version}",
        NO_COLOR
    )
    info(
        f"Platform: {{FG.yellow}}{PLATFORM.name}{{RESET}}, {('32-bit', '64-bit')[PLATFORM.is_64]}\n",
        NO_COLOR
    )

    # winmm is used to set timer resolution
    links = []
    if builder.compiler == Compiler.GCC:
        if IS_WIN: links = ["-lwinmm"]

    elif builder.compiler == Compiler.MSVC:
        links = ["winmm.lib"]

    # Build for x86_64 (64-bit)

    info("Compilation for x86_64 started", NO_COLOR)

    # Bit hacky solution to generate object files in build dir
    if os.path.exists(BUILD_PATH): shutil.rmtree(BUILD_PATH)
    os.mkdir(BUILD_PATH)
    os.chdir(BUILD_PATH)
    builder.compile(generate_object=True, links=links, clear=False)
    os.chdir(BASE_PATH)

    info("Generating library for x86_64", NO_COLOR)

    os.mkdir(BUILD_PATH / "libnova_x86_64")

    start = perf_counter()
    out = builder.build_library(BUILD_PATH / "libnova_x86_64" / "libnova")
    lib_time = perf_counter() - start

    if out.returncode == 0:
        success(
            f"Library generation is done in {{FG.blue}}{round(lib_time, 3)}{{RESET}} seconds.",
            NO_COLOR
        )

    else:
        # Print blank line because of compiler error message
        print()
        error(f"Library generation failed with return code {out.returncode}.", NO_COLOR)

    builder.remove_object_files()

    # Build for x86 (32-bit)

    if BUILD_FOR_32BIT:
        info("Compilation for x86 started", NO_COLOR)

        os.chdir(BUILD_PATH)
        builder.compile(generate_object=True, links=links + ["-m32"], clear=False)
        os.chdir(BASE_PATH)

        info("Generating library for x86", NO_COLOR)

        os.mkdir(BUILD_PATH / "libnova_x86")

        start = perf_counter()
        out = builder.build_library(BUILD_PATH / "libnova_x86" / "libnova.a")
        lib_time = perf_counter() - start

        if out.returncode == 0:
            success(
                f"Library generation is done in {{FG.blue}}{round(lib_time, 3)}{{RESET}} seconds.",
                NO_COLOR
            )

        else:
            # Print blank line because of compiler error message
            print()
            error(f"Library generation failed with return code {out.returncode}.", NO_COLOR)

        builder.remove_object_files()


    # Build archives

    info("Building release-ready archives", NO_COLOR)

    start = time()

    if builder.compiler == Compiler.GCC:
        lib_file = "libnova.a"

    elif builder.compiler == Compiler.MSVC:
        lib_file = "libnova.lib"

    try:
        ZIP_TEMP = BASE_PATH / "_zip_temp"
        TGZ_TEMP = BASE_PATH / "_targz_temp"

        os.mkdir(ZIP_TEMP)
        os.mkdir(TGZ_TEMP)
        os.mkdir(ZIP_TEMP / "lib")
        os.mkdir(TGZ_TEMP / "lib")
        if BUILD_FOR_32BIT:
            os.mkdir(ZIP_TEMP / "lib" / "x86")
            os.mkdir(TGZ_TEMP / "lib" / "x86")
        os.mkdir(ZIP_TEMP / "lib" / "x86_64")
        os.mkdir(TGZ_TEMP / "lib" / "x86_64")

        # Copy include directory
        shutil.copytree(INCLUDE_PATH, ZIP_TEMP / "include")
        shutil.copytree(INCLUDE_PATH, TGZ_TEMP / "include")

        # Copy library files
        if BUILD_FOR_32BIT:
            shutil.copyfile(BUILD_PATH / "libnova_x86" / lib_file, ZIP_TEMP / "lib" / "x86" / lib_file)
            shutil.copyfile(BUILD_PATH / "libnova_x86" / lib_file, TGZ_TEMP / "lib" / "x86" / lib_file)
        shutil.copyfile(BUILD_PATH / "libnova_x86_64" / lib_file, ZIP_TEMP / "lib" / "x86_64" / lib_file)
        shutil.copyfile(BUILD_PATH / "libnova_x86_64" / lib_file, TGZ_TEMP / "lib" / "x86_64" / lib_file)

        ver = get_nova_version()
        shutil.make_archive(BUILD_PATH / f"nova-physics-{ver}-devel", "zip", ZIP_TEMP)
        shutil.make_archive(BUILD_PATH / f"nova-physics-{ver}-devel", "gztar", TGZ_TEMP)

    except Exception as e:
        error(
            [
                f"An exception has occured while building archives:",
                str(e)
            ],
            NO_COLOR
        )

    zip_time = time() - start

    success(
        f"Archives are built in {{FG.blue}}{round(zip_time, 3)}{{RESET}} seconds.",
        NO_COLOR
    )

    # Cleaning
    builder.remove_object_files()
    shutil.rmtree(ZIP_TEMP)
    shutil.rmtree(TGZ_TEMP)
    if os.path.exists(BUILD_PATH / "libnova_x86_64"): shutil.rmtree(BUILD_PATH / "libnova_x86_64")
    if os.path.exists(BUILD_PATH / "libnova_x86"): shutil.rmtree(BUILD_PATH / "libnova_x86")


def example(cli: CLIHandler):
    """ Build & run example """

    NO_COLOR = not cli.get_option("-n")

    # Abort if none example arguments given
    if len(cli.args) == 0:
        cmdeg = "{FG.darkgray}(eg. {FG.magenta}nova_builder {FG.yellow}example {RESET}pool{FG.darkgray})"
        error(
            f"You have to enter an example name. {cmdeg}{{RESET}}",
            NO_COLOR
        )

    # Abort if example argument is not found
    if cli.args[0].endswith(".c"):
        example = EXAMPLES_PATH / cli.args[0]
    else:
        example = EXAMPLES_PATH / (cli.args[0] + ".c")

    if not os.path.exists(example):
        error(
            [
                f"Example file {{FG.lightblue}}{example}{{RESET}} is not found.",
                "Make sure you are in the Nova Physics directory!"
            ],
            NO_COLOR
        )


    # Control & download dependencies

    dm = DependencyManager(cli)

    info("Checking dependencies.", NO_COLOR)

    dm.check()

    deps = dm.satisfied()
    if (deps == 0):
        success("All dependencies are satisfied.\n", NO_COLOR)

    else:
        info(f"Missing {deps} dependencies.\n", NO_COLOR)

    dm.satisfy()


    # Compile the example

    builder = NovaBuilder(cli)

    info(
        f"Detected compiler: {{FG.yellow}}{builder.compiler.name}{{RESET}} {builder.compiler_version}",
        NO_COLOR
    )
    info(
        f"Platform: {{FG.yellow}}{PLATFORM.name}{{RESET}}, {('32-bit', '64-bit')[PLATFORM.is_64]}\n",
        NO_COLOR
    )

    info("Compilation started", NO_COLOR)

    libs = []
    if IS_WIN:
        libs += [
            DEPS_PATH / "SDL2_lib",
            DEPS_PATH / "SDL2_ttf_lib",
            DEPS_PATH / "SDL2_image_lib"
        ]

    # winmm is used to set timer resolution
    if builder.compiler == Compiler.GCC:
        links = ["-lSDL2main", "-lSDL2", "-lSDL2_ttf", "-lSDL2_image"]
        if IS_WIN:
            links.insert(0, "-lmingw32")
            links.append("-lwinmm")
    
    elif builder.compiler == Compiler.MSVC:
        links = ["SDL2main.lib", "SDL2.lib", "SDL2_ttf.lib", "SDL2_image.lib"]
        links.append("winmm.lib")

    builder.compile(
        sources = [example],
        include = [DEPS_PATH / "include"],
        libs = libs,
        links = links,
        clear = True
    )

    # Can't avoid MSVC generating objects...
    if builder.compiler == Compiler.MSVC:
        example_obj = example.stem + ".obj"
        if os.path.exists(example_obj):
            os.remove(example_obj)


    # Copy assets and DLLs to build directory
    os.mkdir(BUILD_PATH / "assets")
    for *_, files in os.walk(EXAMPLES_PATH / "assets"):
            for file in files:
                if not file.startswith("example"):
                    shutil.copyfile(
                        EXAMPLES_PATH / "assets" / file,
                        BUILD_PATH / "assets" / file
                    )

    if IS_WIN:
        # Copy DLLs
        for *_, files in os.walk(DEPS_PATH):
            for file in files:
                if file.endswith(".dll"):
                    shutil.copyfile(DEPS_PATH / file, BUILD_PATH / file)


    # Run the example
    # We have to change directory to get assets working 
    print()
    info("Running the example", NO_COLOR)

    os.chdir(BUILD_PATH)

    out = subprocess.run(builder.binary, shell=True)

    if out.returncode == 0:
        success(f"Example exited with code {out.returncode}.", NO_COLOR)
    
    elif out.returncode == SEGFAULT_CODE:
        error(
            [
                f"Segmentation fault occured in the example demo. Exit code: {out.returncode}",
                f"Please report this at {{FG.lightcyan}}https://github.com/kadir014/nova-physics/issues{{RESET}}"
            ],
            NO_COLOR
        )

    else:
        error(f"Example exited with code {out.returncode}", NO_COLOR)


def benchmark(cli: CLIHandler):
    """ Build & run benchmarks """

    NO_COLOR = not cli.get_option("-n")

    # Abort if none benchmark arguments given
    if len(cli.args) == 0:
        cmdeg = "{FG.darkgray}(eg. {FG.magenta}nova_builder {FG.yellow}bench {RESET}big_pool{FG.darkgray})"
        error(
            f"You have to enter a benchmark name. {cmdeg}{{RESET}}",
            NO_COLOR
        )

    # Abort if benchmark argument is not found
    if cli.args[0].endswith(".c"):
        bench = BENCHS_PATH / cli.args[0]
    else:
        bench = BENCHS_PATH / (cli.args[0] + ".c")

    if not os.path.exists(bench):
        error(
            [
                f"Benchmark file {{FG.lightblue}}{bench}{{RESET}} is not found.",
                "Make sure you are in the Nova Physics directory!"
            ],
            NO_COLOR
        )

    # Output isn't shown unless window option is set
    cli._set_option("-c")

    
    # Compile the benchmark
    builder = NovaBuilder(cli)

    info(
        f"Detected compiler: {{FG.yellow}}{builder.compiler.name}{{RESET}} {builder.compiler_version}",
        NO_COLOR
    )
    info(
        f"Platform: {{FG.yellow}}{PLATFORM.name}{{RESET}}, {('32-bit', '64-bit')[PLATFORM.is_64]}\n",
        NO_COLOR
    )

    info("Compilation started", NO_COLOR)

    # winmm is used to set timer resolution
    links = []
    if builder.compiler == Compiler.GCC:
        if IS_WIN: links = ["-lwinmm"]

    elif builder.compiler == Compiler.MSVC:
        links = ["winmm.lib"]

    builder.compile(
        sources = [bench],
        include = [BENCHS_PATH],
        links = links,
        clear = True
    )

    # Can't avoid MSVC generating objects...
    if builder.compiler == Compiler.MSVC:
        bench_obj = bench.stem + ".obj"
        if os.path.exists(bench_obj):
            os.remove(bench_obj)


    # Run the benchmark
    print()
    info("Running the benchmark", NO_COLOR)

    os.chdir(BUILD_PATH)

    out = subprocess.run(builder.binary, shell=True)

    if out.returncode == 0:
        success(f"Benchmark exited with code {out.returncode}.", NO_COLOR)
    
    elif out.returncode == SEGFAULT_CODE:
        error(
            [
                f"Segmentation fault occured in the benchmark. Exit code: {out.returncode}",
                f"Please report this at {{FG.lightcyan}}https://github.com/kadir014/nova-physics/issues{{RESET}}"
            ],
            NO_COLOR
        )

    else:
        error(f"Benchmark exited with code {out.returncode}", NO_COLOR)



if __name__ == "__main__":
    main()