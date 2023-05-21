#!/usr/bin/python3

import sys
import shutil
import subprocess

# Default options
BuildType = "release"
WarnLevel = "3"
RunTest   = False
Doxygen   = False

# Utility functions
def strtobool(val):
    val = val.lower()
    if val in ('y', 'yes', 't', 'true', 'on', '1'):
        return True
    if val in ('n', 'no', 'f', 'false', 'off', '0'):
        return False
    raise ValueError("invalid truth value %r" % (val,))

# Parse arguments
for arg in sys.argv[1:]:
    opt = arg.split("=")
    if len(opt) < 2:
        continue
    if opt[0] == "BuildType":
        BuildType = opt[1]
    if opt[0] == "WarnLevel":
        WarnLevel = opt[1]
    if opt[0] == "RunTest":
        RunTest = strtobool(opt[1])
    if opt[0] == "Doxygen":
        Doxygen = strtobool(opt[1])

# Output directory
BuildDir = "build/" + BuildType

# Build commands
conf = [
    "meson", "setup",
    "--buildtype", BuildType,
    "--warnlevel", WarnLevel,
    BuildDir
]
make = [
    "meson", "compile",
    "-C", BuildDir,
]
test = [
    "meson", "test",
    "-C", BuildDir
]
docs = [
    "doxygen"
]

# Execute
subprocess.run(conf)
subprocess.run(make)
if RunTest:
    subprocess.run(test)
if Doxygen:
    subprocess.run(docs, cwd=BuildDir)
