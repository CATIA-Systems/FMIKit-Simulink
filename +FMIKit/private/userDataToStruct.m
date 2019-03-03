function ud = userDataToStruct(userData)

ud = struct('fmiKitVersion', [], ...
            'fmuFile', [], ...
            'fmuLastModified', [], ...
            'unzipDirectory', [], ...
            'runAsKind', [], ...
            'sampleTime', [], ...
            'inputPorts', struct('label', [], 'variables', {}), ...
            'outputPorts', struct('label', [], 'variables', {}), ...
            'startValues', containers.Map, ...
            'debugLogging', [], ...
            'errorDiagnostics', [], ...
            'useSourceCode', [], ...
            'setBlockName', [], ...
            'functionName', [], ...
            'parameters', [], ...
            'directInput', []);

% TODO: check version

ud.fmiKitVersion = char(userData.fmiKitVersion);
ud.fmuFile = char(userData.fmuFile);
ud.fmuLastModified = userData.fmuLastModified;
ud.unzipDirectory = char(userData.unzipDirectory);
ud.runAsKind = userData.runAsKind;
ud.sampleTime = char(userData.sampleTime);

for i = 1:userData.inputPorts.size()
    port = userData.inputPorts.get(i-1);
    ud.inputPorts(i).label = char(port.label);
    for j = 1:port.variables.size()
        ud.inputPorts(i).variables{end+1} = char(port.variables.get(j-1));
    end
end

for i = 1:userData.outputPorts.size()
    port = userData.outputPorts.get(i-1);
    ud.outputPorts(i).label = char(port.label);
    for j = 1:port.variables.size()
        ud.outputPorts(i).variables{end+1} = char(port.variables.get(j-1));
    end
end

it = userData.startValues.entrySet().iterator();
while it.hasNext()
    entry = it.next();
    ud.startValues(entry.getKey()) = entry.getValue();
end

ud.debugLogging = userData.debugLogging;
ud.errorDiagnostics = char(userData.errorDiagnostics);
ud.useSourceCode = userData.useSourceCode;
ud.setBlockName = userData.setBlockName;
ud.functionName = char(userData.functionName);
ud.parameters = char(userData.parameters);
ud.directInput = userData.directInput;

end