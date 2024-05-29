# Starlight
Starlight is a galaxy simulator designed for visual pleasure.

## Build instructions
### Requirements
1. meson

### Dependencies
1. glfw3
1. vulkan

### Build all
To build all, run the following command in the root directory of this project.
```bash
./build.py [BuildType=[debug|debugoptimized|release|minsize]] [WarnLevel=[0|1|2|3]] [RunTest=[true|false]] [Doxygen=[true|false]]
```
1. BuildType(default: release)  
Select the build type. For details, refer to meson build types.
1. WarnLevel(default: 3)  
Select the warn level. For details, refer to meson warn levels.
1. RunTest(default: false)  
Run unit tests post build if RunTest is true.
1. Doxygen(default: false)  
`doc/html/index.html` will be generated if Doxygen is true and doxygen is installed.

### Clean all
To clean all, run the following command in the root directory of this project.
```bash
./clean.py
```
