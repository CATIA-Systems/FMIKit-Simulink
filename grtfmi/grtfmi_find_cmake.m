function command = grtfmi_find_cmake(command)

% try the command
if ~isempty(command)
    [status, ~] = system(command);
    assert(status == 0, ['Failed to run CMake command: ' command '.'])
    return
end

% check for CMake in the FMI Kit folder
info = what('+FMIKit');
[fmikit_folder, ~, ~] = fileparts(info(1).path);
listing = dir(fullfile(fmikit_folder, 'cmake-*', 'bin', 'cmake.exe'));

if ~isempty(listing)
    command = fullfile(listing.folder, listing.name);
    return
end

% try the default command
command = 'cmake';

[status, ~] = system(command);
if status == 0
    return
end

% try the default install locations
if ispc
    command = fullfile(getenv('PROGRAMFILES'), 'CMake', 'bin', 'cmake.exe');
    [status, ~] = system(command);
    if status == 0
        return
    end
    
    command = fullfile(getenv('PROGRAMFILES(X86)'), 'CMake', 'bin', 'cmake.exe');
    [status, ~] = system(command);
    if status == 0
        return
    end
elseif ismac
    command = '/usr/local/bin/cmake';
    [status, ~] = system(command);
    if status == 0
        return
    end
end

error(['Failed to run CMake. ' ...
    'Run <a href="matlab: grtfmi_install_cmake">grtfmi_install_cmake</a> to download and install CMake ' ...
    'into the FMI Kit directory or install from <a href="https://cmake.org/">cmake.org</a> and set the cmake command in ' ...
    'Configuration Parameters > Code Generation > CMake Build > CMake Command ' ...
    'if the cmake executable is not on the path and it is not installed in the default location.'])

end
