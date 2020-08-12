![FMI Kit banner](docs/images/banner.png)

# FMI Kit for Simulink

[![Build Status](https://dev.azure.com/CATIA-Systems/FMIKit-Simulink/_apis/build/status/CATIA-Systems.FMIKit-Simulink?branchName=master)](https://dev.azure.com/CATIA-Systems/FMIKit-Simulink/_build/latest?definitionId=4&branchName=master)

A Simulink library to import and export [Functional Mock-up Units](https://fmi-standard.org/) that supports...

- FMI 1.0 and 2.0
- Model Exchange and Co-Simulation
- Rapid Accelerator mode
- import of C code FMUs
- MATLAB R2012b - R2020a

## Quick start

To get started with FMI Kit run the following commands in the MATLAB Command Window

```matlab
% download and extract the distribution archive to the current folder
unzip(['https://github.com/CATIA-Systems/FMIKit-Simulink/releases/' ...
  'download/v2.8/FMIKit-Simulink-2.8.zip'], 'FMIKit-Simulink-2.8')

% add the folder to the MATLAB path
addpath(fullfile(pwd, 'FMIKit-Simulink-2.8'))

% initialize FMI Kit
FMIKit.initialize()

% open the Bouncing Ball demo
fmikit_demo_BouncingBall

% open the documentation
web('FMIKit-Simulink-2.8/html/index.html')
```

See the documentation to learn how to [import](docs/fmu_import.md) and [export](docs/fmu_export.md) FMUs.

## Commercial Support

You're starting a project, need training or professional support?
Our partners at [Claytex](https://www.claytex.com/about-us/contact-us/) are ready to help you.

## License

Copyright &copy; 2019 Dassault Syst&egrave;mes.
The code is released under the [2-Clause BSD license](LICENSE.txt).
