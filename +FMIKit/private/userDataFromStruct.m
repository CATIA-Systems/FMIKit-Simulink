function userData = userDataFromStruct(ud)
% Convert a UserData MATLAB struct to a Java object

userData = fmikit.ui.UserData;

userData.fmiKitVersion     = ud.fmiKitVersion;
userData.fmuFile           = ud.fmuFile;
userData.fmuLastModified   = ud.fmuLastModified;
userData.unzipDirectory    = ud.unzipDirectory;
userData.runAsKind         = ud.runAsKind;
userData.sampleTime        = ud.sampleTime;
userData.relativeTolerance = ud.relativeTolerance;

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

userData.debugLogging     = ud.debugLogging;
userData.errorDiagnostics = ud.errorDiagnostics;
userData.useSourceCode    = ud.useSourceCode;
userData.setBlockName     = ud.setBlockName;
userData.functionName     = ud.functionName;
userData.parameters       = ud.parameters;
userData.directInput      = ud.directInput;

end
