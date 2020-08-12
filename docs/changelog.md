# Changelog

## 2.8

This release improves the import of source code FMUs and fixes issues with export of FMUs.
The build system for the S-function based target has been changed to CMake for improved flexibility.

### FMU import

- `new` model image preview in block dialog
- `new` improved handling of source code FMUs
- `new` use one FMI Kit installation with multiple MATLAB versions

### FMU export (grtfmi.tlc)

- `new` export of signals with storage class "ExportGlobal"
- `new` model directory is added to include path
- `new` debug builds
- `new` compiler optimization flags
- `new` CMake toolset option (-T)
- `fixed` shared library paths of nested FMUs
- `fixed` handle multiple sample rates

### FMU export (rtwfscnfmi.tlc)

- `new` build system changed to CMake (same as for grtfmi.tlc)

## 2.7

This release brings fixes and improvements for the import and export of FMUs.
It is now distributed as a ZIP archive (instead of a MATLAB App) for easier deployment.

### FMU import

- `changed` the input port direct feedthrough is now automatically derived from the model structure and replaces the "direct input" checkbox.
  This also introduces the expected one step delay for Co-Simulation FMUs and resolves issues with complex models. See the documentation for the exact calling sequence.
- `new` the relative tolerance can now be set for Co-Simulation
- `new` the "Log level" is now adjustable
- `new` FMI calls can now be logged
- `new` the log messages can now be redirected to a file
- `new` the FMIKit.setSampleTime() and FMIKit.setInterfaceType() functions have been added to the API
- `improved` an installed CMake is now detected more reliably
- `improved` long FMU descriptions are now displayed completely
- `removed` the obsolete "Error diagnostics" checkbox has been removed

### FMU export (grtfmi.tlc)

- `fixed` purely discrete models are now synchronized correctly
- `fixed` the indices for array parameters have been fixed
- `new` exported FMUs can now contain FMU blocks
- `new` block outputs, busses and enumerations can now be exported
- `new` a screenshot of the model is added as model.png
- `updated` the FMI headers have been updated to version 2.0.1
- `changed` the "resources" have been replaced with a more flexible template directory
