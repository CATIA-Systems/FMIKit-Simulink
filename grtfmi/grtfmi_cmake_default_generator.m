function generator = grtfmi_cmake_default_generator

if ispc
    % detect the platform
    if strcmp(computer('arch'), 'win64')
        platform = ' Win64';
    else
        platform = '';
    end
    
    % use Visual Studio 2015 as default
    generator = ['Visual Studio 14 2015' platform];

    % find the newest installed Visual Studio
    vswhere = [getenv('ProgramFiles(x86)') '\Microsoft Visual Studio\Installer\vswhere.exe'];
    command = ['"' vswhere '" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath'];
    [status, cmdout] = system(command);
    
    if status == 0
        if ~isempty(strfind(cmdout, '2019'))  %#ok<STREMP>
            generator = 'Visual Studio 16 2019';
        elseif ~isempty(strfind(cmdout, '2017'))  %#ok<STREMP>
            generator = ['Visual Studio 15 2017' platform];          
        end
    elseif ~isempty(getenv('VS140COMNTOOLS'))
        generator = ['Visual Studio 14 2015' platform];
    elseif ~isempty(getenv('VS120COMNTOOLS'))
        generator = ['Visual Studio 12 2013' platform];
    elseif ~isempty(getenv('VS110COMNTOOLS'))
        generator = ['Visual Studio 11 2012' platform];
    elseif ~isempty(getenv('VS100COMNTOOLS'))
        generator = ['Visual Studio 10 2010' platform];
    elseif ~isempty(getenv('VS90COMNTOOLS'))
        generator = ['Visual Studio 9 2008' platform];
    end
else
    generator = 'Unix Makefiles';
end

end
