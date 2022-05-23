# Changelog

## v3.0

- FMI 3.0 support for FMU import and FMU export with grtfmi.tlc
- New FMU import framework and improved FMU block dialog
- Improved CMake based FMU export for both grtfmi.tlc and rtwfscnfmi.tlc

## v2.9

This release resolves issues with the FMU export:

- Fix memory allocation and fmi2Reset() in rtwsfcnfmi.tlc (#31)
- Initialize model variables in fmi2Reset() in grtfmi.tlc (#211)
- Use 0-based indexing when exporting FMI 2.0 arrays in grtfmi.tlc and rtwsfcnfmi.tlc (#208)
- Add sources of referenced models in grtfmi.tlc
- Evaluate nextEventTimeDefined in Model Exchange (#209)
- Skip duplicate CGType indices in rtwsfcnfmi.tlc (#205)
- Escape special characters in input and output variables in grtfmi.tlc (#206)
- Fix loading of MEX S-functions in rtwsfcnfmi.tlc (#203)

## v2.8

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

## v2.7

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
