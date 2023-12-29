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
import json
import tarfile
import zipfile
import io
import urllib.error
import urllib.request
import multiprocessing
from abc import ABC, abstractmethod
from pathlib import Path
from enum import Enum
from time import perf_counter, time, gmtime


IS_WIN = platform.system() == "Windows"


# Fix windows terminal
if IS_WIN:
    subprocess.run("", shell=True)
    # This import won't be used anywhere else
    from ctypes import windll
    k = windll.kernel32
    k.SetConsoleMode(k.GetStdHandle(-11), 7) # -11 = stdout


SEGFAULT_CODES = (
    3221225477, # This can also mean a network error, but irrevelant in our case
    11,
    -11,
    134,
    139
)


class FG:
    """
    ANSI escape codes for terminal foreground colors.
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
    ANSI escape codes for terminal background colors.
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
    """
    Other ANSI escape codes for graphic modes.
    Some doesn't work on some terminals.
    """

    bold = "\033[01m"
    underline = "\033[04m"
    reverse = "\033[07m"
    strike = "\033[09m"

RESET = "\033[0m"

def format_colors(string: str, no_ansi: bool = False) -> str:
    """ Format string with color codes. """

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


class ProgressBar:
    """
    Simplistic progress bar.
    """
    
    def __init__(self,
            template: str,
            minvalue: Optional[float] = 0.0,
            maxvalue: Optional[float] = 1.0,
            value: Optional[float] = None
            ) -> None:
        self.template = template
        self.minvalue = minvalue
        self.maxvalue = maxvalue
        self.value = self.minvalue if value is None else value
        self.start_time = time()
        self.bar_length = 30
        self.mbps = 0.0

    def _render(self) -> str:
        """ Render the progress bar string. """

        render = self.template

        normval = self.minvalue + self.value * (self.maxvalue - self.minvalue)
        fpercent = normval * 100
        percent = round(fpercent)

        elapsed = gmtime(time() - self.start_time)

        remaining = gmtime(((time() - self.start_time) / self.value) * self.maxvalue)
        
        render = render.replace("{fpercent}", f"{fpercent}")
        render = render.replace("{percent}", f"{percent}")

        render = render.replace("{mbps}", f"{round(self.mbps, 2)}")

        render = render.replace("{ela.hours}", f"{elapsed.tm_hour}")
        render = render.replace("{ela.mins}", f"{elapsed.tm_min}")
        render = render.replace("{ela.secs}", f"{elapsed.tm_sec}")

        render = render.replace("{rem.hours}", f"{remaining.tm_hour}")
        render = render.replace("{rem.mins}", f"{remaining.tm_min}")
        render = render.replace("{rem.secs}", f"{remaining.tm_sec}")

        render = render.replace("{progress}", f"[{'='*int(normval * self.bar_length): <{self.bar_length}}]")

        return render

    def progress(self, increment: float) -> None:
        """ Advance the progress bar. """

        self.value += increment

        print(f"{self._render()}\033[1G\033[1A")


def error(messages: Union[list, str], no_color: bool = False) -> None:
    """ Log error message and abort. """

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
    
def success(messages: Union[list, str], no_color: bool = False) -> None:
    """ Log success message. """

    if isinstance(messages, str):
        msg = f"{{FG.lightgreen}}{{Style.reverse}} DONE {{RESET}} {messages}\n"
        print(format_colors(msg, no_color))
    
    else:
        msg = f"{{FG.lightgreen}}{{Style.reverse}} DONE {{RESET}} {messages[0]}\n"

        for line in messages[1:]:
            msg += f"       {line}\n"

        print(format_colors(msg, no_color))

def info(messages: Union[list, str], no_color: bool = False) -> None:
    """ Log information message. """

    if isinstance(messages, str):
        msg = f"{{FG.cyan}}{{Style.reverse}} INFO {{RESET}} {messages}"
        print(format_colors(msg, no_color))

    else:
        msg = f"{{FG.cyan}}{{Style.reverse}} INFO {{RESET}} {messages[0]}"

        for line in messages[1:]:
            msg += f"       {line}\n"

        print(format_colors(msg, no_color))


def get_output(cmd: str) -> str:
    """ Run check_output, return empty string on error. """

    try:
        return subprocess.check_output(cmd, shell=True).decode("utf-8").strip()
    except:
        return ""
    

class Platform:
    """
    Platform specific information gatherer.

    Attributes
    ----------
    is_64 Whether the system architechure is 64-bit or not.
    system Most basic name that represents the system.
    name Name of the system.
    min_name Shortened name (without release versions, etc..)

    Possible configurations on different platforms:

    Attribute | Windows         | Manjaro    | Ubuntu       | MacOS  | Other Linux
    ----------+-----------------+------------+--------------+--------+------------------
    system    | Windows         | Linux      | Linux        | Darwin | Linux
    name      | Windows 10 Home | Manjaro 23 | Ubuntu 20.04 | ?      | release + version
    min_name  | Windows         | Manjaro    | Ubuntu       | ?      | Linux
    """

    def __init__(self) -> None:
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


BASE_PATH = Path(os.getcwd())

SRC_PATH = BASE_PATH / "src"
INCLUDE_PATH = BASE_PATH / "include"

BUILD_PATH = BASE_PATH / "build"
DEPS_PATH = BASE_PATH / "deps"
CACHE_PATH = BASE_PATH / "cache"

EXAMPLES_PATH = BASE_PATH / "examples"
BENCHS_PATH = BASE_PATH / "benchmarks"
TESTS_PATH = BASE_PATH / "tests"


def get_nova_version() -> str:
    """ Get Nova Phyiscs Engine version number. """
    
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


def remove_readonly(action, name, exc) -> None:
    """ Overwrites READ-ONLY files as WRITE-ONLY and removes them. """
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

def remove_dir(path: Path) -> None:
    """ Remove directory with its contents recursively. """
    shutil.rmtree(path, onerror=remove_readonly)

def copy_dir(
        src_path: Path,
        dst_path: Path,
        symlinks: bool = False,
        ignore: bool = None
        ) -> None:
    """ Copy one directory's content into another one recursively. """
    
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

def copy_dlls(src_dir: Path, dst_dir: Path) -> None:
    """ Copy DLL files from one directory to another. """

    for item in os.listdir(src_dir):
        s = os.path.join(src_dir, item)
        if os.path.isfile(s):
            if item.endswith(".dll"):
                shutil.copyfile(s, dst_dir / item)


class CLI:
    """
    Class that parses and handles command line arguments and commands.
    """

    def __init__(self) -> None:
        self.commands = {}
        self.arguments = {}
        self.extra_arguments = []

    @property
    def command_count(self) -> int:
        return sum(int(self.check_command(command)) for command in self.commands)
    
    @property
    def argument_count(self) -> int:
        return sum(int(self.check_argument(argument)) for argument in self.arguments)

    def add_command(self, command: str, doc: str) -> bool:
        """ Add a command. """
        self.commands[command] = {
            "doc": doc,
            "passed": False
        }

    def add_argument(self,
            argument: Union[str, tuple[str, str]],
            doc: str,
            accepts_value: bool = False,
            accepts_suffix: bool = False,
            value: Union[None, str, int] = None
            ) -> None:
        """ Add an argument. """

        self.arguments[argument] = {
            "doc": doc,
            "passed": False,
            "accepts_value": accepts_value,
            "accepts_suffix": accepts_suffix,
            "value": value
        }

    def check_command(self, command: str) -> bool:
        """ Check if the command is passed. """
        return self.commands[command]["passed"]

    def check_argument(self, argument: Union[str, tuple[str, str]]) -> bool:
        """ Check if the argument is passed. """
        
        for arg in self.arguments:
            if isinstance(arg, str):
                if arg == argument:
                    return self.arguments[arg]["passed"]
                
            elif isinstance(arg, tuple):
                if argument in arg or argument == arg:
                    return self.arguments[arg]["passed"]

    def get_argument(self,
            argument: Union[str, tuple[str, str]]
            ) -> Union[None, str, int]:
        """ Get the value of argument. """
        
        for arg in self.arguments:
            if isinstance(arg, str):
                if arg == argument:
                    return self.arguments[arg]["value"]
                
            elif isinstance(arg, tuple):
                if argument in arg or argument == arg:
                    return self.arguments[arg]["value"]

    def parse(self) -> None:
        """ Parse command line. """

        for arg in sys.argv[1:]:
            if self._set_argument(arg):
                continue

            if self._set_command(arg):
                continue

            self.extra_arguments.append(arg)

    def usage(self) -> str:
        """ Generate usage manual. """

        script_name = sys.argv[0].replace(".py", "")
        man =                                                                                        \
        f"{{FG.orange}}Usage:{{RESET}} {script_name} [arguments] <command> [extra arguments...]\n" + \
        "\n"                                                                                       + \
        "{FG.orange}Commands:{RESET}\n"

        GAP = 20

        for command in self.commands:
            command_l = command.ljust(GAP)
            man += f"  {{FG.lightcyan}}{command_l}{{RESET}} {self.commands[command]['doc']}\n"

        man += "\n{FG.orange}Arguments:{RESET}\n"

        for argument in self.arguments:
            if isinstance(argument, tuple):
                option_l = f"{{FG.lightcyan}}{argument[0]} {{FG.darkgray}}| {{FG.lightcyan}}{argument[1]}{{RESET}}"
                option_l = option_l.ljust(GAP + 48)

            else:
                option_l = f"{{FG.lightcyan}}{argument.ljust(GAP)}{{RESET}}"

            man += f"  {option_l} {self.arguments[argument]['doc']}\n"

        return man

    def _set_command(self, raw: str) -> bool:
        """ Check if the command exists and set it. """

        for cmd in self.commands:
            if raw == cmd:
                self.commands[cmd]["passed"] = True
                return True
                
        return False
    
    def _set_argument(self, raw: str) -> bool:
        """ Check if the argument exists and set it. """

        for arg in self.arguments:
            if isinstance(arg, str):
                if raw.startswith(arg):
                    parsed = self._parse_argument(raw, arg, self.arguments[arg])
                    if not parsed["valid"]: continue
                    self.arguments[arg]["passed"] = True
                    self.arguments[arg]["value"] = parsed["value"]
                    return True

            elif isinstance(arg, tuple):
                if raw.startswith(arg[0]) or raw.startswith(arg[1]):
                    parsed = self._parse_argument(raw, arg, self.arguments[arg])
                    if not parsed["valid"]: continue
                    self.arguments[arg]["passed"] = True
                    self.arguments[arg]["value"] = parsed["value"]
                    return True
                
        return False

    def _parse_argument(self,
            raw: str,
            argument: Union[str, tuple[str, str]],
            context: dict
            ) -> dict:

        dash = 0
        if raw.startswith("-"): dash = 1
        elif raw.startswith("--"): dash = 2
        
        if context["accepts_value"]:
            if "=" in raw:
                s = raw.split("=")
                arg = s[0]
                val = s[1]

                return {"valid": self._is_valid(arg, argument), "value": val}
            
            else:
                return {"valid": self._is_valid(raw, argument), "value": None}
            
        elif context["accepts_suffix"]:
            raw_ = raw[dash:]
            # Do you have a better solution to parse suffix values? Cuz I don't
            if any(char.isdigit() for char in raw_):
                letters = []
                digits = []

                for char in raw_:
                    if char.isalpha(): letters.append(char)
                    elif char.isdigit(): digits.append(char)

                letters = "".join(letters)
                digits = int("".join(digits))

                return {
                    "valid": self._is_valid(f"{'-'*dash}{letters}", argument),
                    "value": digits
                }

            else:
                return {"valid": self._is_valid(raw, argument), "value": None}

        else:
            return {"valid": self._is_valid(raw, argument), "value": None}
        
    def _is_valid(self,
            argument: str,
            original: Union[str, tuple[str, str]]
            ) -> bool:
        if isinstance(original, str):
            return argument == original
        
        elif isinstance(original, tuple):
            if argument == original[0]: return True
            elif argument == original[1]: return True


class DependencyManager:
    """
    Class to download & manage build dependencies.
    """

    def __init__(self, cli: CLI) -> None:
        self.cli = cli
        self.no_color = not self.cli.check_argument("-n")

        self.chunk_size = 4 * 1024

        # Don't forget to update versions
        SDL2_VER = "2.28.5"
        TTF_VER = "2.20.2"

        # Library and DLL files are Windows only
        self.dependencies = {
            "SDL2": {
                "url": f"https://github.com/libsdl-org/SDL/releases/download/release-{SDL2_VER}/SDL2-devel-{SDL2_VER}-mingw.tar.gz",

                "include": {
                    "satisfied": False,
                    "path": DEPS_PATH / "include" / "SDL2",
                    "archive-path": f"SDL2-{SDL2_VER}/x86_64-w64-mingw32/include/SDL2"
                },

                "lib-x64": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "lib-x64" / "SDL2",
                    "archive-path": f"SDL2-{SDL2_VER}/x86_64-w64-mingw32/lib"
                },

                "lib-x86": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "lib-x86" / "SDL2",
                    "archive-path": f"SDL2-{SDL2_VER}/i686-w64-mingw32/lib"
                },

                "bin-x64": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "bin-x64" / "SDL2",
                    "archive-path": f"SDL2-{SDL2_VER}/x86_64-w64-mingw32/bin"
                },

                "bin-x86": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "bin-x86" / "SDL2",
                    "archive-path": f"SDL2-{SDL2_VER}/i686-w64-mingw32/bin"
                }
            },

            "SDL2-MSVC": {
                "url": f"https://github.com/libsdl-org/SDL/releases/download/release-{SDL2_VER}/SDL2-devel-{SDL2_VER}-VC.zip",

                "include": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "include" / "SDL2-MSVC",
                    "archive-path": f"SDL2-{SDL2_VER}/include"
                },

                "lib-x64": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "lib-x64" / "SDL2-MSVC",
                    "archive-path": f"SDL2-{SDL2_VER}/lib/x64"
                },

                "lib-x86": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "lib-x86" / "SDL2-MSVC",
                    "archive-path": f"SDL2-{SDL2_VER}/lib/x86"
                },

                "bin-x64": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "bin-x64" / "SDL2-MSVC",
                    "archive-path": f"SDL2-{SDL2_VER}/lib/x64"
                },

                "bin-x86": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "bin-x86" / "SDL2-MSVC",
                    "archive-path": f"SDL2-{SDL2_VER}/lib/x86"
                }
            },

            "SDL2_ttf": {
                "url": f"https://github.com/libsdl-org/SDL_ttf/releases/download/release-{TTF_VER}/SDL2_ttf-devel-{TTF_VER}-mingw.tar.gz",

                "include": {
                    "satisfied": False,
                    "path": DEPS_PATH / "include" / "SDL2",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/x86_64-w64-mingw32/include/SDL2"
                },

                "lib-x64": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "lib-x64" / "SDL2_ttf",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/x86_64-w64-mingw32/lib"
                },

                "lib-x86": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "lib-x86" / "SDL2_ttf",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/i686-w64-mingw32/lib"
                },

                "bin-x64": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "bin-x64" / "SDL2_ttf",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/x86_64-w64-mingw32/bin"
                },

                "bin-x86": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "bin-x86" / "SDL2_ttf",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/i686-w64-mingw32/bin"
                }
            },

            "SDL2_ttf-MSVC": {
                "url": f"https://github.com/libsdl-org/SDL_ttf/releases/download/release-{TTF_VER}/SDL2_ttf-devel-{TTF_VER}-VC.zip",

                "include": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "include" / "SDL2-MSVC",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/include"
                },

                "lib-x64": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "lib-x64" / "SDL2_ttf-MSVC",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/lib/x64"
                },

                "lib-x86": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "lib-x86" / "SDL2_ttf-MSVC",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/lib/x86"
                },

                "bin-x64": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "bin-x64" / "SDL2_ttf-MSVC",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/lib/x64"
                },

                "bin-x86": {
                    "satisfied": not IS_WIN,
                    "path": DEPS_PATH / "bin-x86" / "SDL2_ttf-MSVC",
                    "archive-path": f"SDL2_ttf-{TTF_VER}/lib/x86"
                }
            }
        }

    def download(self, url: str) -> bytes:
        """ Download & return the data. """
        
        try:
            response = urllib.request.urlopen(url)
        
        except urllib.error.HTTPError as e:
            error(
                f"{{FG.lightcyan}}{url}{{RESET}} responsed with code {{FG.lightred}}{e.code}{{RESET}}.",
                self.no_color
            )

        size = int(response.info()["Content-Length"])

        template = f"{{progress}} {{FG.yellow}}{{percent}}%{{RESET}} {{FG.magenta}}{{mbps}}Mbps{{RESET}} ETA {{FG.lightblue}}{{ela.hours}}:{{ela.mins}}:{{ela.secs}}{{RESET}} RTA {{FG.lightblue}}{{rem.hours}}:{{rem.mins}}:{{rem.secs}}{{RESET}}"
        template = format_colors(template, self.no_color)
        bar = ProgressBar(template)

        fp = io.BytesIO()
        dl = 0
        start = perf_counter()
        while True:
            chunk = response.read(self.chunk_size)
            dl += len(chunk)
            if not chunk: break
            fp.write(chunk)

            mbps = dl / (perf_counter() - start) / (2 ** 17)

            bar.mbps = mbps
            bar.progress(self.chunk_size / size)

        fp.seek(0)
        return fp.read()
    
    def extract(self, data: bytes, path: Path) -> None:
        """ Extract archive data to path. """

        try:
            with tarfile.open(mode="r:gz", fileobj=io.BytesIO(data)) as tar:
                tar.extractall(path)

        except tarfile.ReadError:
            with zipfile.ZipFile(io.BytesIO(data)) as zip:
                zip.extractall(path)

        except Exception as e:
            raise e

    def check(self) -> None:
        """ Check if dependencies satisfy. """

        for dep in self.dependencies:
            for pack in self.dependencies[dep]:
                if pack == "url": continue

                if os.path.exists(self.dependencies[dep][pack]["path"]):
                    self.dependencies[dep][pack]["satisfied"] = True
        
    def missing(self, dep_: Optional[str] = None) -> int:
        """ Return number of missing dependencies. """

        deps = 0

        for dep in self.dependencies:
            if dep_ is not None and dep != dep_: continue 

            for pack in self.dependencies[dep]:
                if pack == "url": continue

                if not self.dependencies[dep][pack]["satisfied"]:
                    deps += 1

        return deps
    
    def clean(self) -> None:
        """ Clean temporary repositories. """

        for dep_name in self.dependencies:
            temp_dir = DEPS_PATH / f"_{dep_name}"
            if os.path.exists(temp_dir):
                remove_dir(temp_dir)
    
    def satisfy(self) -> None:
        """ Download, extract and organize all dependencies. """

        # Don't need to continue if all deps are satisfied
        if self.missing() == 0:
            return

        if not os.path.exists(DEPS_PATH):
            os.mkdir(DEPS_PATH)

        if not os.path.exists(DEPS_PATH / "include"):
            os.mkdir(DEPS_PATH / "include")
        
        if not os.path.exists(DEPS_PATH / "lib-x64"):
            os.mkdir(DEPS_PATH / "lib-x64")

        if not os.path.exists(DEPS_PATH / "lib-x86"):
            os.mkdir(DEPS_PATH / "lib-x86")

        if not os.path.exists(DEPS_PATH / "bin-x64"):
            os.mkdir(DEPS_PATH / "bin-x64")

        if not os.path.exists(DEPS_PATH / "bin-x86"):
            os.mkdir(DEPS_PATH / "bin-x86")

        start = time()

        self.clean()

        for dep_name in self.dependencies:
            temp_dir = DEPS_PATH / f"_{dep_name}"

            dep = self.dependencies[dep_name]

            if self.missing(dep_name):
                info(f"Downloading {{FG.lightcyan}}{dep['url']}{{RESET}}", self.no_color)

                self.extract(
                    self.download(dep["url"]),
                    temp_dir
                )

            if not dep["include"]["satisfied"]:
                info(
                    f"Extracting {{FG.yellow}}{dep['include']['archive-path']}{{RESET}}",
                    self.no_color
                )

                if not os.path.exists(dep["include"]["path"]):
                    os.mkdir(dep["include"]["path"])

                copy_dir(
                    temp_dir / dep["include"]["archive-path"],
                    dep["include"]["path"]
                )

            if IS_WIN:
                if not dep["lib-x64"]["satisfied"]:
                    info(
                        f"Extracting {{FG.yellow}}{dep['lib-x64']['archive-path']}{{RESET}}",
                        self.no_color
                    )

                    if not os.path.exists(dep["lib-x64"]["path"]):
                        os.mkdir(dep["lib-x64"]["path"])

                    copy_dir(
                        temp_dir / dep["lib-x64"]["archive-path"],
                        dep["lib-x64"]["path"]
                    )

                if not dep["lib-x86"]["satisfied"]:
                    info(
                        f"Extracting {{FG.yellow}}{dep['lib-x86']['archive-path']}{{RESET}}",
                        self.no_color
                    )

                    if not os.path.exists(dep["lib-x86"]["path"]):
                        os.mkdir(dep["lib-x86"]["path"])

                    copy_dir(
                        temp_dir / dep["lib-x86"]["archive-path"],
                        dep["lib-x86"]["path"]
                    )

                if not dep["bin-x64"]["satisfied"]:
                    info(
                        f"Extracting {{FG.yellow}}{dep['bin-x64']['archive-path']}{{RESET}}",
                        self.no_color
                    )

                    if not os.path.exists(dep["bin-x64"]["path"]):
                        os.mkdir(dep["bin-x64"]["path"])

                    copy_dir(
                        temp_dir / dep["bin-x64"]["archive-path"],
                        dep["bin-x64"]["path"]
                    )

                if not dep["bin-x86"]["satisfied"]:
                    info(
                        f"Extracting {{FG.yellow}}{dep['bin-x86']['archive-path']}{{RESET}}",
                        self.no_color
                    )

                    if not os.path.exists(dep["bin-x86"]["path"]):
                        os.mkdir(dep["bin-x86"]["path"])

                    copy_dir(
                        temp_dir / dep["bin-x86"]["archive-path"],
                        dep["bin-x86"]["path"]
                    )

        self.clean()

        end = time() - start

        success(
            f"Downloaded & extracted all dependencies in {{FG.lightblue}}{round(end, 3)}{{RESET}} seconds.\n",
            self.no_color
        )


class CompilerType(Enum):
    """
    Supported compilers.
    """

    GCC = 0
    MSVC = 1
    CLANG = 2


COMMON_MSVC_DEV_PROMPTS = (
    "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2022/Community/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2022/Professional/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2022/Professional/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2022/Enterprise/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2022/BuildTools/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2019/Community/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2019/Professional/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2019/Professional/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2019/Enterprise/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2019/BuildTools/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2017/Community/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2017/Professional/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2017/Enterprise/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2017/Enterprise/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2017/BuildTools/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2017/BuildTools/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2015/Community/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2015/Community/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2015/Professional/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2015/Professional/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2015/Enterprise/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2015/Enterprise/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files/Microsoft Visual Studio/2015/BuildTools/Common7/Tools/VsDevCmd.bat",
    "C:/Program Files (x86)/Microsoft Visual Studio/2015/BuildTools/Common7/Tools/VsDevCmd.bat",
)

MSVC_DEV_PROMPT = None


def detect_compilers() -> dict[CompilerType, str]:
    """ Detect available compilers on the system. """

    global MSVC_DEV_PROMPT
    
    gcc_path = shutil.which("gcc")
    gcc_invoker = None
    if gcc_path is not None:
        gcc_invoker = f"\"{gcc_path}\""

    for common_path in COMMON_MSVC_DEV_PROMPTS:
        if os.path.exists(common_path):
            MSVC_DEV_PROMPT = common_path
            break

    msvc_invoker = None
    if MSVC_DEV_PROMPT is not None:
        msvc_invoker = "cl.exe"

    return {
        CompilerType.GCC: gcc_invoker,
        CompilerType.MSVC: msvc_invoker,
        CompilerType.CLANG: None
    }


COMPILER_ARGS = {
    CompilerType.GCC: {
        "debug": "-g",
        "optimization": (
            "-O1", "-O2", "-O3"
        ),
        "warnings": "-Wall",
        "define": "-D",
        "include": "-I",
        "library": "-L",
        "link": "-l",
        "invoke-avx": "-march=native"
    },

    CompilerType.MSVC: {
        "debug": "/Zi",
        "optimization": (
            "/O1", "/O2", "/Ox"
        ),
        "warnings": "/W3",
        "define": "/D",
        "include": "/I",
        "library": "/LIBPATH:",
        "link": "",
        "invoke-avx": "/arch:AVX"
    }
}


class Compiler(ABC):
    """
    Base compiler class.
    """

    def __init__(self,
            type_: CompilerType,
            invoker: str,
            no_color: bool = False
            ) -> None:
        self.type = type_
        self.invoker = invoker
        self.no_color = no_color
        self.fetch_version()

    def build_cache(self) -> None:
        """ Build object file cache. """

        self.cached_sources = {}
        if os.path.exists(CACHE_PATH / "cached_sources.json"):
            with open(CACHE_PATH / "cached_sources.json", "r", encoding="utf-8") as file:
                self.cached_sources = json.load(file)

    @abstractmethod
    def fetch_version(self) -> str:
        """ Fetch compiler version on the system. """
        ...

    @abstractmethod
    def _build_compile_command(self,
            sources_arg: str,
            include_arg: str,
            args_arg: str,
            define_arg: str
            ) -> str:
        """ Generate compilation command. """
        ...
    
    @abstractmethod
    def _build_linkage_command(self,
            binary_path: str,
            objects_arg: str,
            library_arg: str,
            linkage_arg: str,
            args_arg: str
            ) -> str:
        """ Generate linkage command. """
        ...

    def compile(self,
            source_paths: list[Path] = [],
            include_paths: list[Path] = [],
            library_paths: list[Path] = [],
            linkage_args: list[str] = [],
            defines: list[str] = [],
            compile_args: list[str] = [],
            link_args: list[str] = [],
            binary: Optional[str] = None,
            process_count: int = 1,
            verbose: bool = False
            ) -> None:
        """ Compile. """

        if os.path.exists(BUILD_PATH):
            shutil.rmtree(BUILD_PATH)

        os.mkdir(BUILD_PATH)

        if not os.path.exists(CACHE_PATH):
            os.mkdir(CACHE_PATH)

        os.chdir(CACHE_PATH)

        if self.type == CompilerType.MSVC:
            for i in range(len(linkage_args)):
                linkage_args[i] = linkage_args[i] + ".lib"

        if not IS_WIN:
            linkage_args.append("m") # Required on Linux for math.h

        include_arg = f" {COMPILER_ARGS[self.type]['include']}".join([str(include_path) for include_path in include_paths])
        if len(include_arg) > 0: include_arg = COMPILER_ARGS[self.type]['include'] + include_arg

        library_arg = f" {COMPILER_ARGS[self.type]['library']}".join([str(library_path) for library_path in library_paths])
        if len(library_arg) > 0: library_arg = COMPILER_ARGS[self.type]['library'] + library_arg

        linkage_arg = f" {COMPILER_ARGS[self.type]['link']}".join(linkage_args)
        if len(linkage_arg) > 0: linkage_arg = COMPILER_ARGS[self.type]['link'] + linkage_arg

        define_arg = f" {COMPILER_ARGS[self.type]['define']}".join(defines)
        if len(define_arg) > 0: define_arg = COMPILER_ARGS[self.type]['define'] + define_arg

        compile_args_arg = " ".join(compile_args)
        link_args_arg = " ".join(link_args)

        # Detect modified source files
        new_source_paths = []
        for source_path in source_paths:
            src = str(source_path)

            if src in self.cached_sources:
                mtime = os.path.getmtime(src)

                if self.cached_sources[src] != mtime:
                    self.cached_sources[src] = mtime
                    new_source_paths.append(source_path)

            else:
                self.cached_sources[src] = os.path.getmtime(src)
                new_source_paths.append(source_path)

        # Compile if there is any changed source file
        start = perf_counter()

        if len(new_source_paths) == 0:
            if verbose:
                info(f"No source to compile.", self.no_color)

        else:
            if verbose:
                info(f"There are {len(new_source_paths)} changed source files to compile.", self.no_color)

            processes = []
            targets = [[] for _ in range(process_count)]

            # Distribute sources across multiple processes evenly
            i = 0
            for source_path in new_source_paths:
                targets[i].append(source_path)
                i += 1
                if i > len(targets) - 1: i = 0

            for sub_sources in targets:
                if len(sub_sources) == 0: continue
                
                sources_arg = " ".join([str(source_path) for source_path in sub_sources])
                compile_command = self._build_compile_command(
                    sources_arg,
                    include_arg,
                    define_arg,
                    compile_args_arg
                )

                if verbose: print(compile_command, "\n")
                processes.append(subprocess.Popen(compile_command, shell=True))

            for process in processes:
                process.communicate()

            for process in processes:
                if process.returncode != 0:
                    error(f"Compilation failed with return code {process.returncode}.", self.no_color)

        # Link object files if binary name is given
        if binary is not None:
            if IS_WIN:
                binary_path = f"{binary}.exe"
            else:
                binary_path = f"./{binary}"

            object_paths = self.get_object_paths()

            objects_arg = " ".join([str(object_path) for object_path in object_paths])
            linkage_command = self._build_linkage_command(
                binary_path,
                objects_arg,
                library_arg,
                linkage_arg,
                link_args_arg
            )
            
            if verbose: print(linkage_command, "\n")
            out = subprocess.run(linkage_command, shell=True)
            if out.returncode != 0:
                error(f"Linkage failed with return code {out.returncode}.", self.no_color)

        end = perf_counter()
        comp_time = end - start

        # Update cache data if the compilation is sucessful
        with open(CACHE_PATH / "cached_sources.json", "w", encoding="utf-8") as file:
            file.write(json.dumps(self.cached_sources))

        success(f"Compilation is done in {{FG.blue}}{round(comp_time, 3)}{{RESET}} seconds.", self.no_color)

        os.chdir(BASE_PATH)

    @abstractmethod
    def generate_library(self, library_path: Path) -> None:
        """ Generate static library from object files. """
        ...

    def get_object_paths(self) -> list[Path]:
        """ Gather cached object files. """

        object_ext = ".o" if self.type in (CompilerType.GCC, CompilerType.CLANG) else ".obj"

        object_paths = []

        for name in os.listdir(CACHE_PATH):
            file = CACHE_PATH / name
            if os.path.isfile(file) and name.endswith(object_ext):
                object_paths.append(file)

        return object_paths


class CompilerGCC(Compiler):
    def __init__(self, invoker: str, no_color: bool = False) -> None:
        super().__init__(CompilerType.GCC, invoker, no_color)

    def fetch_version(self) -> str:
        version = get_output(f"{self.invoker} -dumpfullversion -dumpversion")
        if version != "": self.version = version

    def _build_compile_command(self,
            sources_arg: str,
            include_arg: str,
            args_arg: str,
            define_arg: str
            ) -> str:
        
        compile_command = f"{self.invoker} -c {sources_arg} {include_arg} {args_arg} {define_arg}"
        return " ".join(compile_command.split())
    
    def _build_linkage_command(self,
            binary_path: str,
            objects_arg: str,
            library_arg: str,
            linkage_arg: str,
            args_arg: str
            ) -> str:
        
        linkage_command = f"{self.invoker} -o {binary_path} {objects_arg} {library_arg} {linkage_arg} {args_arg}"
        return " ".join(linkage_command.split())

    def generate_library(self, library_path: Path) -> None:
        ...


class CompilerMSVC(Compiler):
    def __init__(self, invoker: str, no_color: bool = False) -> None:
        super().__init__(CompilerType.MSVC, invoker, no_color)

    def fetch_version(self) -> str:
        if "2022" in MSVC_DEV_PROMPT: self.compiler_version = "2022"
        elif "2019" in MSVC_DEV_PROMPT: self.compiler_version = "2019"
        elif "2017" in MSVC_DEV_PROMPT: self.compiler_version = "2017"
        elif "2015" in MSVC_DEV_PROMPT: self.compiler_version = "2015"

    def _build_compile_command(self,
            sources_arg: str,
            include_arg: str,
            args_arg: str,
            define_arg: str
            ) -> str:
        
        compile_command = f"\"{MSVC_DEV_PROMPT}\" & {self.invoker} /nologo /c {sources_arg} {include_arg} {args_arg} {define_arg}"
        return " ".join(compile_command.split())
    
    def _build_linkage_command(self,
            binary_path: str,
            objects_arg: str,
            library_arg: str,
            linkage_arg: str,
            args_arg: str
            ) -> str:
        
        linkage_command = f"\"{MSVC_DEV_PROMPT}\" & link.exe /OUT:{binary_path} {objects_arg} {library_arg} {linkage_arg} {args_arg}"
        return " ".join(linkage_command.split())

    def generate_library(self, library_path: Path) -> None:
        ...
    

def main():
    """ Entry point of the CLI. """

    if not os.path.exists(CACHE_PATH):
        os.mkdir(CACHE_PATH)

    cli = CLI()

    cli.add_command("build", "Build static library")
    cli.add_command("examples", "Run example demos")
    cli.add_command("bench", "Run a benchmark from benchmarks directory")
    cli.add_command("tests", "Run unit tests")

    cli.add_argument(("-h", "--help"), "Print usage manual")
    cli.add_argument(("-q", "--quiet"), "Do not log any build logs")
    cli.add_argument(("-v", "--verbose"), "Get build logs as verbose as possible")
    cli.add_argument(("-f", "--float"), "Use single-precision floating point numbers")
    cli.add_argument("--no-color", "Disable coloring with ANSI escape codes")
    cli.add_argument("--clear", "Clear cached code and configuration")
    cli.add_argument(
        "--target",
        "Specify a target compiler instead of detecting one",
        accepts_value=True
    )
    cli.add_argument("--force-deps", "Force download all dependencies (for example demos)")
    cli.add_argument("--enable-tracy", "Enable Tracy profiler")
    cli.add_argument("--no-profiler", "Disable built-in profiler")
    cli.add_argument("--no-simd", "Disable SIMD vectorization")
    cli.add_argument("--m32", "Build 32-bit executable (If the compiler is 64-bit)")
    cli.add_argument("-g", "Compile for debugging")
    cli.add_argument(
        "-O",
        "Set optimization level (default is 3)",
        accepts_suffix=True,
        value=3
    )
    cli.add_argument(
        "-j",
        "Parallel compilation on multiple processes (defaults to CPU count)",
        accepts_suffix=True,
        value=multiprocessing.cpu_count()
    )
    cli.add_argument("-w", "Enable all warnings")

    cli.parse()

    NO_COLOR = not cli.get_argument("-n")

    print(format_colors("{FG.magenta}Nova Physics Engine Build System{RESET}", NO_COLOR))
    print(format_colors("Manual: {FG.cyan}https://github.com/kadir014/nova-physics/blob/main/BUILDING.md{RESET}", NO_COLOR))
    print()

    if not BASE_PATH.name.startswith("nova-physics"):
        error(
            [
                "Make sure you are in the Nova Physics directory!",
                f"This script is ran at {{FG.yellow}}{BASE_PATH.absolute()}{{RESET}}"
            ], NO_COLOR
        )

    builder_command = None

    if cli.check_command("build"):
        builder_command = "build"
    
    elif cli.check_command("examples"):
        builder_command = "examples"

    elif cli.check_command("bench"):
        builder_command = "bench"

    elif cli.check_command("tests"):
        builder_command = "tests"

    if cli.check_argument("--clear"):
        remove_dir(CACHE_PATH)
        os.mkdir(CACHE_PATH)

    if cli.check_argument("-O"):
        optimization = cli.get_argument("-O")

        if optimization < 1 or optimization > 3:
            error("Optimization value must be in range [1, 3].", NO_COLOR)

    if cli.check_argument("-j"):
        parallel_comp = cli.get_argument("-j")

        if parallel_comp <= 0:
            error("-j argument value can't be smaller than 1.", NO_COLOR)

    help_arg = cli.check_argument("-h")

    if cli.command_count == 0 or help_arg:
        if not help_arg and len(cli.extra_arguments) > 0:
            error(
                (
                    f"Unknown command: {cli.extra_arguments[0]}",
                    "Run 'python nova_builder.py' without any arguments to see usage manual."
                ),
                NO_COLOR
            )

        print(format_colors(cli.usage(), NO_COLOR))

    else:
        detected = detect_compilers()
        target = cli.get_argument("--target")

        if target is None:
            compiler = CompilerGCC(detected[CompilerType.GCC], not cli.check_argument("-n"))

        elif target.lower() == "gcc":
            if detected[CompilerType.GCC] is None:
                error("Targeted compiler is not available on the system.", NO_COLOR)

            compiler = CompilerGCC(detected[CompilerType.GCC], not cli.check_argument("-n"))

        elif target.lower() == "msvc":
            if detected[CompilerType.MSVC] is None:
                error("Targeted compiler is not available on the system.", NO_COLOR)
                
            compiler = CompilerMSVC(detected[CompilerType.MSVC], not cli.check_argument("-n"))

        else:
            error(f"Unknown compiler target: '{target}'", NO_COLOR)

        info(
            f"Compiler: {{FG.yellow}}{compiler.type.name}{{RESET}} {{FG.lightcyan}}{compiler.fetch_version()}{{RESET}}",
            NO_COLOR
        )
        info(
            f"Platform: {{FG.yellow}}{PLATFORM.name}{{RESET}}, {('32-bit', '64-bit')[PLATFORM.is_64]}\n",
            NO_COLOR
        )

        clear_cache = False

        current_config = {
            "compiler": str(compiler.type),
            "debug": cli.check_argument("-g"),
            "enable-tracy": cli.check_argument("--enable-tracy"),
            "no-profiler": cli.check_argument("--no-profiler"),
            "no-simd": cli.check_argument("--no-simd"),
            "command": builder_command
        }

        # Config was never cached, cache now
        if not os.path.exists(CACHE_PATH / "cached_config.json"):
            with open(CACHE_PATH / "cached_config.json", "w+") as file:
                file.write(json.dumps(current_config))

        # Config was cached, check if the current config is the same
        # if not, clear all the cache and update the config
        else:
            with open(CACHE_PATH / "cached_config.json", "r", encoding="utf-8") as file:
                cached_config = json.load(file)

            clear_cache = cached_config != current_config

        if clear_cache:
            if cli.check_argument("-v"):
                info("Compilaton configuration has been changed. Clearing & updating cache.", NO_COLOR)

            remove_dir(CACHE_PATH)
            os.mkdir(CACHE_PATH)

            with open(CACHE_PATH / "cached_config.json", "w+") as file:
                file.write(json.dumps(current_config))

        elif cli.check_argument("-v"):
            info("Compilaton configuration is the same.", NO_COLOR)

        compiler.build_cache()

        if cli.check_command("build"):
            build(cli, compiler)
        
        elif cli.check_command("examples"):
            examples(cli, compiler)

        elif cli.check_command("bench"):
            benchmark(cli, compiler)

        elif cli.check_command("tests"):
            tests(cli, compiler)

def build(cli: CLI, compiler: Compiler):
    NO_COLOR = not cli.get_argument("-n")

    source_paths = []
    include_paths = [INCLUDE_PATH]
    linkage_args = []
    defines = []
    compile_args = []
    link_args = []

    if cli.check_argument("--enable-tracy"):
        TRACY_PATH = SRC_PATH / "tracy"
        source_paths.append(TRACY_PATH / "TracyClient.cpp")
        include_paths.append(TRACY_PATH)
        defines.append("TRACY_ENABLE")
        # Tracy needs all this libraries
        linkage_args += ["stdc++", "ws2_32", "wsock32", "dbghelp"]

    for name in os.listdir(SRC_PATH):
        if os.path.isfile(SRC_PATH / name):
            source_paths.append(SRC_PATH / name)

    if cli.check_argument("-f"):
        defines.append("NV_FLOAT")

    if not cli.check_argument("--no-profile"):
        defines.append("NV_PROFILE")

    if not cli.check_argument("--no-simd"):
        defines.append("NV_USE_SIMD")

    if cli.check_argument("-g"):
        compile_args.append(COMPILER_ARGS[compiler.type]["debug"])

    else:
        compile_args.append(f"{COMPILER_ARGS[compiler.type]['optimization'][cli.get_argument('-O')-1]}")

    if cli.check_argument("-w"):
        compile_args.append(COMPILER_ARGS[compiler.type]["warnings"])

    compile_args.append(COMPILER_ARGS[compiler.type]["invoke-avx"])

    info("Compilation started", NO_COLOR)

    compiler.compile(
        source_paths=source_paths,
        include_paths=include_paths,
        linkage_args=linkage_args,
        defines=defines,
        compile_args=compile_args,
        link_args=link_args,
        process_count=cli.get_argument("-j"),
        verbose=cli.check_argument("-v")
    )

def examples(cli: CLI, compiler: Compiler):
    NO_COLOR = not cli.get_argument("-n")

    if cli.check_argument("--force-deps"):
        if os.path.exists(BASE_PATH / "deps"):
            remove_dir(BASE_PATH / "deps")

    dm = DependencyManager(cli)

    info("Checking dependencies.", NO_COLOR)

    dm.check()

    deps = dm.missing()
    if (deps == 0):
        success("All dependencies are satisfied.", NO_COLOR)

    else:
        info(f"Missing {deps} dependency files.", NO_COLOR)

    dm.satisfy()

    source_paths = [EXAMPLES_PATH / "example.c"]
    include_paths = [INCLUDE_PATH, DEPS_PATH / "include"]
    linkage_args = ["SDL2main", "SDL2", "SDL2_ttf"]
    defines = []
    compile_args = []
    link_args = []

    if PLATFORM.is_64:
        dep_lib = "lib-x64"
        dep_bin = "bin-x64"

    else:
        dep_lib = "lib-x86"
        dep_bin = "bin-x32"

    if compiler.type == CompilerType.GCC:
        library_paths = [DEPS_PATH / dep_lib / "SDL2", DEPS_PATH / dep_lib / "SDL2_ttf"]

    elif compiler.type == CompilerType.MSVC:
        library_paths = [DEPS_PATH / dep_lib / "SDL2-MSVC", DEPS_PATH / dep_lib / "SDL2_ttf-MSVC"]

    if IS_WIN and compiler.type == CompilerType.GCC:
        linkage_args.insert(0, "mingw32")

    if compiler.type == CompilerType.MSVC:
        defines.append("SDL_MAIN_HANDLED")
        defines.append("_CRT_SECURE_NO_WARNINGS") # Disable security warnings for sprintf
        link_args.append("/SUBSYSTEM:CONSOLE")

    if cli.check_argument("--enable-tracy"):
        TRACY_PATH = SRC_PATH / "tracy"
        source_paths.append(TRACY_PATH / "TracyClient.cpp")
        include_paths.append(TRACY_PATH)
        defines.append("TRACY_ENABLE")
        # Tracy needs all this libraries
        linkage_args += ["stdc++", "ws2_32", "wsock32", "dbghelp"]

    for name in os.listdir(SRC_PATH):
        if os.path.isfile(SRC_PATH / name):
            source_paths.append(SRC_PATH / name)

    if cli.check_argument("-f"):
        defines.append("NV_FLOAT")

    if not cli.check_argument("--no-profile"):
        defines.append("NV_PROFILE")

    if not cli.check_argument("--no-simd"):
        defines.append("NV_USE_SIMD")

    if cli.check_argument("-g"):
        compile_args.append(COMPILER_ARGS[compiler.type]["debug"])

    else:
        compile_args.append(f"{COMPILER_ARGS[compiler.type]['optimization'][cli.get_argument('-O')-1]}")

    if cli.check_argument("-w"):
        compile_args.append(COMPILER_ARGS[compiler.type]["warnings"])

    compile_args.append(COMPILER_ARGS[compiler.type]["invoke-avx"])

    info("Compilation started", NO_COLOR)

    compiler.compile(
        source_paths=source_paths,
        include_paths=include_paths,
        library_paths=library_paths,
        linkage_args=linkage_args,
        defines=defines,
        compile_args=compile_args,
        link_args=link_args,
        process_count=cli.get_argument("-j"),
        binary="nova",
        verbose=cli.check_argument("-v"),
    )

    os.mkdir(BUILD_PATH / "assets")
    for *_, files in os.walk(EXAMPLES_PATH / "assets"):
        for file in files:
            if not file.startswith("example"):
                shutil.copyfile(
                    EXAMPLES_PATH / "assets" / file,
                    BUILD_PATH / "assets" / file
                )

    if IS_WIN:
        binary = "nova.exe"
    
    else:
        binary = "nova"

    os.replace(CACHE_PATH / binary, BUILD_PATH / binary)

    if IS_WIN:
        if compiler.type == CompilerType.GCC:
            copy_dlls(DEPS_PATH / dep_bin / "SDL2", BUILD_PATH)
            copy_dlls(DEPS_PATH / dep_bin / "SDL2_ttf", BUILD_PATH)

        elif compiler.type in (CompilerType.MSVC, CompilerType.CLANG):
            copy_dlls(DEPS_PATH / dep_bin / "SDL2-MSVC", BUILD_PATH)
            copy_dlls(DEPS_PATH / dep_bin / "SDL2_ttf-MSVC", BUILD_PATH)

    # Run the example
    # We have to change directory to get assets working 
    info("Running the example demos", NO_COLOR)

    os.chdir(BUILD_PATH)

    out = subprocess.run(binary, shell=True)

    if out.returncode == 0:
        success(f"Example demos exited with code {out.returncode}.", NO_COLOR)
    
    elif out.returncode in SEGFAULT_CODES:
        error(
            [
                f"Segmentation fault occured in the example demos. Exit code: {out.returncode}",
                f"Please report this at {{FG.lightcyan}}https://github.com/kadir014/nova-physics/issues{{RESET}}"
            ],
            NO_COLOR
        )

    else:
        error(f"Example demos exited with code {out.returncode}", NO_COLOR)

def benchmark(cli: CLI, compiler: Compiler):
    NO_COLOR = not cli.get_argument("-n")

    if len(cli.extra_arguments) == 0:
        cmd_example = "{FG.darkgray}(eg. {FG.magenta}nova_builder {FG.yellow}bench {RESET}boxes{FG.darkgray})"
        error(
            f"You have to enter a benchmark name. {cmd_example}{{RESET}}",
            NO_COLOR
        )

    if cli.extra_arguments[0].endswith(".c"):
        bench = BENCHS_PATH / cli.extra_arguments[0]
    else:
        bench = BENCHS_PATH / (cli.extra_arguments[0] + ".c")

    if not os.path.exists(bench):
        error(
            [
                f"Benchmark file {{FG.lightblue}}{bench}{{RESET}} is not found.",
                "Make sure you are in the Nova Physics directory!"
            ],
            NO_COLOR
        )

    source_paths = [bench]
    include_paths = [INCLUDE_PATH]
    library_paths = []
    linkage_args = []
    defines = []
    compile_args = []
    link_args = []

    if cli.check_argument("--enable-tracy"):
        TRACY_PATH = SRC_PATH / "tracy"
        source_paths.append(TRACY_PATH / "TracyClient.cpp")
        include_paths.append(TRACY_PATH)
        defines.append("TRACY_ENABLE")
        # Tracy needs all this libraries
        linkage_args += ["stdc++", "ws2_32", "wsock32", "dbghelp"]

    for name in os.listdir(SRC_PATH):
        if os.path.isfile(SRC_PATH / name):
            source_paths.append(SRC_PATH / name)

    if cli.check_argument("-f"):
        defines.append("NV_FLOAT")

    if not cli.check_argument("--no-profile"):
        defines.append("NV_PROFILE")

    if not cli.check_argument("--no-simd"):
        defines.append("NV_USE_SIMD")

    if cli.check_argument("-g"):
        compile_args.append(COMPILER_ARGS[compiler.type]["debug"])

    else:
        compile_args.append(f"{COMPILER_ARGS[compiler.type]['optimization'][cli.get_argument('-O')-1]}")

    if cli.check_argument("-w"):
        compile_args.append(COMPILER_ARGS[compiler.type]["warnings"])

    compile_args.append(COMPILER_ARGS[compiler.type]["invoke-avx"])

    info("Compilation started", NO_COLOR)

    compiler.compile(
        source_paths=source_paths,
        include_paths=include_paths,
        library_paths=library_paths,
        linkage_args=linkage_args,
        defines=defines,
        compile_args=compile_args,
        link_args=link_args,
        process_count=cli.get_argument("-j"),
        binary="nova",
        verbose=cli.check_argument("-v"),
    )

    if IS_WIN:
        binary = "nova.exe"
    
    else:
        binary = "nova"

    os.replace(CACHE_PATH / binary, BUILD_PATH / binary)

    info("Running the benchmarks", NO_COLOR)

    os.chdir(BUILD_PATH)

    out = subprocess.run(binary, shell=True)

    if out.returncode == 0:
        success(f"Benchmark exited with code {out.returncode}.", NO_COLOR)
    
    elif out.returncode in SEGFAULT_CODES:
        error(
            [
                f"Segmentation fault occured in the benchmark. Exit code: {out.returncode}",
                f"Please report this at {{FG.lightcyan}}https://github.com/kadir014/nova-physics/issues{{RESET}}"
            ],
            NO_COLOR
        )

    else:
        error(f"Benchmark exited with code {out.returncode}", NO_COLOR)

def tests(cli: CLI, compiler: Compiler):
    NO_COLOR = not cli.get_argument("-n")

    source_paths = [TESTS_PATH / "tests.c"]
    include_paths = [INCLUDE_PATH]
    library_paths = []
    linkage_args = []
    defines = []
    compile_args = []
    link_args = []

    if cli.check_argument("--enable-tracy"):
        TRACY_PATH = SRC_PATH / "tracy"
        source_paths.append(TRACY_PATH / "TracyClient.cpp")
        include_paths.append(TRACY_PATH)
        defines.append("TRACY_ENABLE")
        # Tracy needs all this libraries
        linkage_args += ["stdc++", "ws2_32", "wsock32", "dbghelp"]

    for name in os.listdir(SRC_PATH):
        if os.path.isfile(SRC_PATH / name):
            source_paths.append(SRC_PATH / name)

    if cli.check_argument("-f"):
        defines.append("NV_FLOAT")

    if not cli.check_argument("--no-profile"):
        defines.append("NV_PROFILE")

    if not cli.check_argument("--no-simd"):
        defines.append("NV_USE_SIMD")

    if cli.check_argument("-g"):
        compile_args.append(COMPILER_ARGS[compiler.type]["debug"])

    else:
        compile_args.append(f"{COMPILER_ARGS[compiler.type]['optimization'][cli.get_argument('-O')-1]}")

    if cli.check_argument("-w"):
        compile_args.append(COMPILER_ARGS[compiler.type]["warnings"])

    compile_args.append(COMPILER_ARGS[compiler.type]["invoke-avx"])

    info("Compilation started", NO_COLOR)

    compiler.compile(
        source_paths=source_paths,
        include_paths=include_paths,
        library_paths=library_paths,
        linkage_args=linkage_args,
        defines=defines,
        compile_args=compile_args,
        link_args=link_args,
        process_count=cli.get_argument("-j"),
        binary="nova",
        verbose=cli.check_argument("-v"),
    )

    if IS_WIN:
        binary = "nova.exe"
    
    else:
        binary = "nova"

    os.replace(CACHE_PATH / binary, BUILD_PATH / binary)

    info("Running the tests", NO_COLOR)

    os.chdir(BUILD_PATH)

    try:
        start = perf_counter()
        out = subprocess.check_output(binary, shell=True)
        elapsed = perf_counter() - start
        
        outs = out.decode("utf-8").split("\n")
        for i, line in enumerate(outs):

            if line.startswith("[PASSED]"):
                outs[i] = format_colors("[{FG.lightgreen}PASSED{RESET}]", NO_COLOR) + line[8:]

            elif line.startswith("[FAILED]"):
                outs[i] = format_colors("[{FG.lightred}FAILED{RESET}]", NO_COLOR) + line[8:]

            elif line.startswith("total:"):
                test_count = int(line[6:])

            elif line.startswith("fails:"):
                fail_count = int(line[6:])

        outs = outs[:-3]

        success_msg = f"Ran {{FG.yellow}}{test_count}{{RESET}} tests in {{FG.blue}}{round(elapsed, 3)}{{RESET}} seconds."
        success(success_msg, NO_COLOR)
        if fail_count == 0:
            info(f"{{FG.lightgreen}}{fail_count}{{RESET}} failed tests.", NO_COLOR)
        else:
            info(f"{{FG.lightred}}{fail_count}{{RESET}} failed tests.", NO_COLOR)

        print()
        print("\n".join(outs))

    except subprocess.CalledProcessError as e:
        if e.returncode in SEGFAULT_CODES:
            error(
                [
                    f"Segmentation fault occured in the tests. Exit code: {e.returncode}",
                    f"Please report this at {{FG.lightcyan}}https://github.com/kadir014/nova-physics/issues{{RESET}}"
                ],
                NO_COLOR
            )

        else:
            error(f"Tests exited with code {e.returncode}", NO_COLOR)


if __name__ == "__main__":
    main()