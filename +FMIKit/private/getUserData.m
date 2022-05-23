function userData = getUserData(block)

userData = get_param(block, 'UserData');

if isempty(userData)
    return
end

mask = Simulink.Mask.get(block);

userData.startValues = containers.Map;

for i = 1:numel(mask.Parameters)
    parameter = mask.Parameters(i);
    userData.startValues(parameter.Prompt) = parameter.Value;
end

if ~strcmp(userData.fmiKitVersion, FMIKit.version)
    error([getfullname(block) ' was imported with an incompatible version of FMI Kit. Please re-import the FMU.']);
end

end
