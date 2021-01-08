# Build from source

You need Maven, Visual Studio
Requirements: MATLAB, Java JDK 7 or later, Apache Maven

### Clone the repository

```
git clone https://github.com/CATIA-Systems/FMIKit
```

### Build fmikit.jar

To build the `fmikit.jar` that contains the block dialog:

- open `Java` in IntelliJ
- in `File > Project Structure` select `Project SDK = 1.6` and `Project language level = 6`
- in `Artifacts` select `Add (+) > JAR > Empty`
- change `Name` to `fmikit` and `Output directory` to the project root
- add `compile output` to `fmikit.jar`
- in `File > Settings > Editor > GUI Designer` set `Generate GUI into = Binary class files` and check `Automatically copy form runtime classes to output directory`
- select `Build > Build Artifacts... > fmikit > Build`

## Compile the generic S-function

### Compile with MATLAB

To compile the generic S-function (`sfun_fmurun.mex*`) change into the `FMIKit` directory.

On Windows run:

```
mex sfun_fmurun.c
```

On Linux:

```
mex sfun_fmurun.c -ldl
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

- start MATLAB with Java debugging (`matlab -jdb`)
- in IntelliJ IDEA select `Run > Attach to Process...` and select the MATLAB process
