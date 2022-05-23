function generator = grtfmi_cmake_default_generator

if ispc
    % use Visual Studio 2015 as default
    generator = 'Visual Studio 14 2015';

    % find the newest installed Visual Studio
    vswhere = [getenv('ProgramFiles(x86)') '\Microsoft Visual Studio\Installer\vswhere.exe'];
    command = ['"' vswhere '" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath'];
    [status, cmdout] = system(command);
    
    if status == 0
        if ~isempty(strfind(cmdout, '2022'))  %#ok<STREMP>
            generator = 'Visual Studio 17 2022';
        elseif ~isempty(strfind(cmdout, '2019'))  %#ok<STREMP>
            generator = 'Visual Studio 16 2019';
        elseif ~isempty(strfind(cmdout, '2017'))  %#ok<STREMP>
            generator = 'Visual Studio 15 2017';          
        end
    elseif ~isempty(getenv('VS140COMNTOOLS'))
        generator = 'Visual Studio 14 2015';
    elseif ~isempty(getenv('VS120COMNTOOLS'))
        generator = 'Visual Studio 12 2013';
    elseif ~isempty(getenv('VS110COMNTOOLS'))
        generator = 'Visual Studio 11 2012';
    elseif ~isempty(getenv('VS100COMNTOOLS'))
        generator = 'Visual Studio 10 2010';
    elseif ~isempty(getenv('VS90COMNTOOLS'))
        generator = 'Visual Studio 9 2008';
    end
else
    generator = 'Unix Makefiles';
end

end
