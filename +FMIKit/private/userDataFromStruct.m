function userData = userDataFromStruct(ud)
% Convert a UserData MATLAB struct to a Java object

userData = fmikit.ui.UserData;

userData.fmiKitVersion     = ud.fmiKitVersion;
userData.fmuFile           = ud.fmuFile;
userData.fmuLastModified   = ud.fmuLastModified;
userData.runAsKind         = ud.runAsKind;
userData.unzipDirectory    = ud.unzipDirectory;
userData.debugLogging      = ud.debugLogging;
userData.logFMICalls       = ud.logFMICalls;
userData.logLevel          = ud.logLevel;
userData.logFile           = ud.logFile;
userData.logToFile         = ud.logToFile;
userData.relativeTolerance = ud.relativeTolerance;
userData.sampleTime        = ud.sampleTime;

for i = 1:numel(ud.inputPorts)
    p = ud.inputPorts(i);
    port = javaObject('fmikit.ui.UserData$Port');
    port.label = p.label;
    for j = 1:numel(p.variables)
        variable = java.lang.String(p.variables{j});
        port.variables.add(variable);
    end
    userData.inputPorts.add(port);
end

for i = 1:numel(ud.outputPorts)
    p = ud.outputPorts(i);
    port = javaObject('fmikit.ui.UserData$Port');
    port.label = p.label;
    for j = 1:numel(p.variables)
        variable = java.lang.String(p.variables{j});
        port.variables.add(variable);
    end
    userData.outputPorts.add(port);
end

for key = keys(ud.startValues)
    userData.startValues.put(...
      java.lang.String(key{1}), ...
      java.lang.String(ud.startValues(key{1})));
end

userData.useSourceCode    = ud.useSourceCode;
userData.functionName     = ud.functionName;
userData.parameters       = ud.parameters;

end
