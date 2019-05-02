# Build from source

You need Maven, Visual Studio
Requirements: MATLAB, Java JDK 7 or later, Apache Maven

### Clone the repository

```
git clone https://github.com/CATIA-Systems/FMIKit
```

### Build fmikit.jar

Change into the `FMIKit` directory and run

```
mvn -f Java/pom.xml install
```

to build the `fmikit.jar` that contains the block dialog.

## Compile the generic S-function

### Compile with MATLAB

In MATLAB change into the `FMIKit` directory and configure the MEX command:

```
mex -setup C++
```

If you're on Windows make sure that MEX is configured to use `Microsoft Visual C++ 2012` or later for C++ language compilation.

To compile the generic S-function (`sfun_fmurun.mex*`) on Windows run

```
mex sfun_fmurun.cpp src/FMU.cpp src/FMU1.cpp src/FMU2.cpp -Iinclude -lshlwapi
```

On Linux:

```
mex sfun_fmurun.cpp src/FMU.cpp src/FMU1.cpp src/FMU2.cpp -Iinclude -v CXXFLAGS='-std=c++11 -fPIC' -ldl
```

## Debugging the generic S-function

Prerequisites: [CMake](https://cmake.org)

- open the [CMakeGUI](https://cmake.org/runningcmake/)
- click `Browse Source...` and select the `FMIKit` directory (that contains `CMakeLists.txt`)
- click `Browse Build...` and select the folder where you want to create the project files (e.g. `FMIKit/build`)
- click `Configure` and select the generator for your IDE / build tool (e.g. `Visual Studio 14 2015 Win64`)
- set the variable `MATLAB_DIR` to point to your MATLAB installation
- click `Generate` to generate the project files
- click `Open Project` or open the project in your build tool
- build the project
- connect to the MATLAB process and start debugging

## Debugging the block dialog

TODO
