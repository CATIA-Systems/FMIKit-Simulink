function generator = grtfmi_cmake_default_generator

if ispc
    % detect the platform
    if strcmp(computer('arch'), 'win64')
        platform = ' Win64';
    else
        platform = '';
    end
    
    % find the newest installed Visual Studio
    vswhere = [getenv('ProgramFiles(x86)') '\Microsoft Visual Studio\Installer\vswhere.exe'];
    vswhere_command = ['"' vswhere '" -latest -products * -requires Microsoft.Component.MSBuild'];
    [status, productName] = system([vswhere_command, ' -property productName']);
    
    if status ~= 0 || ~isempty(productName)
        generator = ['Visual Studio 14 2015' platform];
    else
        [~, installationVersion] = system([vswhere_command, ' -property installationVersion']);
        [~, productLineVersion] = system([vswhere_command, ' -property productLineVersion']);
        
        generator = [strtrim(productName), ' ', ...
            regexp(installationVersion, '^[0-9]*', 'match', 'once'), ' ', ...
            strtrim(productLineVersion), platform];
    end
else
    generator = 'Unix Makefiles';
end

end
