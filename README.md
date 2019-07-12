
<p align="center">
  <img src="docs/images/fmikit-banner.svg" alt="FMIKit banner" width="65%">
</p>

# FMIKit for Simulink

A Simulink library to import and export [Functional Mock-up Units](https://fmi-standard.org/) that supports...

- FMI 1.0 and 2.0
- Model Exchange and Co-Simulation
- Rapid Accelerator mode
- import of C code FMUs
- MATLAB R2012b - R2019b (rtwsfcnfmi.tlc R2012b - R2018b)

## Quick start

- download and run the [MATLAB App installer](https://github.com/CATIA-Systems/FMIKit-Simulink/releases)
- click on the app in the MATLAB app tool strip to initialize FMI Kit
- learn how to [import](docs/fmu_import.md) and [export](docs/fmu_export.md) FMUs

To run the FMI Kit app automatically, add the following lines to your [startup script](https://mathworks.com/help/matlab/ref/startup.html).

```matlab
% start FMI Kit
apps = matlab.apputil.getInstalledAppInfo();

for i = 1:numel(apps)
    app = apps(i);
    if strcmp(app.name, 'FMI Kit')
        matlab.apputil.run(app.id);
        return
    end
end
```

## License

Copyright &copy; 2019 Dassault Syst&egrave;mes.
The code is released under the [2-Clause BSD license](LICENSE.txt).
