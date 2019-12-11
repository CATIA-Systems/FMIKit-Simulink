function ud = userDataToStruct(userData)
% Convert a UserData Java object to a MATLAB struct

ud = struct(...
    'fmiKitVersion',     [], ...
    'fmuFile',           [], ...
    'fmuLastModified',   [], ...
    'runAsKind',         [], ...
    'unzipDirectory',    [], ...
    'debugLogging',      [], ...
    'logFMICalls',       [], ...
    'logLevel',          [], ...
    'logFile',           [], ...
    'logToFile',         [], ...
    'relativeTolerance', [], ...
    'sampleTime',        [], ...
    'inputPorts',  struct('label', [], 'variables', {}), ...
    'outputPorts', struct('label', [], 'variables', {}), ...
    'startValues', containers.Map, ...
    'useSourceCode',     [], ...
    'functionName',      [], ...
    'parameters',        [] ...
    );

% TODO: check version

ud.fmiKitVersion     = char(userData.fmiKitVersion);
ud.fmuFile           = char(userData.fmuFile);
ud.fmuLastModified   = userData.fmuLastModified;
ud.runAsKind         = userData.runAsKind;
ud.unzipDirectory    = char(userData.unzipDirectory);
ud.debugLogging      = userData.debugLogging;
ud.logFMICalls       = userData.logFMICalls;
ud.logLevel          = userData.logLevel;
ud.logFile           = char(userData.logFile);
ud.logToFile         = userData.logToFile;
ud.relativeTolerance = char(userData.relativeTolerance);
ud.sampleTime        = char(userData.sampleTime);

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

ud.useSourceCode    = userData.useSourceCode;
ud.functionName     = char(userData.functionName);
ud.parameters       = char(userData.parameters);

end
