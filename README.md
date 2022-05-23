![FMI Kit banner](docs/images/banner.png)

# FMI Kit for Simulink

[![Build Status](https://dev.azure.com/CATIA-Systems/FMIKit-Simulink/_apis/build/status/CATIA-Systems.FMIKit-Simulink?branchName=master)](https://dev.azure.com/CATIA-Systems/FMIKit-Simulink/_build/latest?definitionId=4&branchName=master)

A Simulink library to import and export [Functional Mock-up Units](https://fmi-standard.org/) that supports...

- FMI 1.0, 2.0, and 3.0
- Model Exchange and Co-Simulation
- MATLAB R2016a - R2022a

## Quick start

To get started with FMI Kit run the following commands in the MATLAB Command Window

```matlab
% download and extract the distribution archive to the current folder
unzip(['https://github.com/CATIA-Systems/FMIKit-Simulink/releases/' ...
  'download/v3.0/FMIKit-Simulink-3.0.zip'], 'FMIKit-Simulink-3.0')

% add the folder to the MATLAB path
addpath(fullfile(pwd, 'FMIKit-Simulink-3.0'))

% initialize FMI Kit
FMIKit.initialize()

% open the Bouncing Ball demo
fmikit_demo_BouncingBall

% open the documentation
web('FMIKit-Simulink-3.0/html/index.html')
```

See the documentation to learn how to [import](docs/fmu_import.md) and [export](docs/fmu_export.md) FMUs.

## Commercial Support

You're starting a project, need training or professional support?
Our partners at [Claytex](https://www.claytex.com/about-us/contact-us/) are ready to help you.

## License

Copyright &copy; 2022 Dassault Syst&egrave;mes.
The code is released under the [2-Clause BSD license](LICENSE.txt).
