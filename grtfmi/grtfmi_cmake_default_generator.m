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
    command = ['"' vswhere '" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath'];
    [status, cmdout] = system(command);
    
    if status == 0 && ~isempty(strfind(cmdout, '2017'))  %#ok<STREMP>
        generator = ['Visual Studio 15 2017' platform];
    elseif system('set VS140COMNTOOLS') == 0
        generator = ['Visual Studio 14 2015' platform];
    elseif system('set VS120COMNTOOLS') == 0
        generator = ['Visual Studio 12 2013' platform];
    elseif system('set VS110COMNTOOLS') == 0
        generator = ['Visual Studio 11 2012' platform];
    elseif system('set VS100COMNTOOLS') == 0
        generator = ['Visual Studio 10 2010' platform];
    elseif system('set VS90COMNTOOLS') == 0
        generator = ['Visual Studio 9 2008' platform];
    else
        generator = ['Visual Studio 14 2015' platform];
    end
else
    generator = 'Unix Makefiles';
end

end
