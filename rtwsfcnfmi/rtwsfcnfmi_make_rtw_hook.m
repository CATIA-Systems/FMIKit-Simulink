function rtwsfcnfmi_make_rtw_hook(hookMethod, modelName, rtwRoot, templateMakefile, buildOpts, buildArgs, buildInfo)

switch hookMethod
    
    case 'after_make'
        
        % skip if build is disabled
        if strcmp(get_param(modelName, 'GenCodeOnly'), 'on')
            return
        end
        
        % remove FMU build directory from previous build
        if exist('FMUArchive', 'dir')
            rmdir('FMUArchive', 's');
        end
        
        switch mexext
            case 'mexa64'
                fmi_platform = 'linux64';
            case 'mexmaci64'
                fmi_platform = 'darwin64';
            case 'mexw32'
                fmi_platform = 'win32';
            case 'mexw64'
                fmi_platform = 'win64';
        end
        
        % create the archive directory (uncompressed FMU)
        mkdir(fullfile('FMUArchive', 'binaries', fmi_platform));
        
        template_dir = get_param(modelName, 'FMUTemplateDir');
        
        % copy template files
        if ~isempty(template_dir)
            copyfile(template_dir, 'FMUArchive');
        end
        
        % add model.png
        if strcmp(get_param(modelName, 'AddModelImage'), 'on')
            % create an image of the model
            print(['-s' modelName], '-dpng', fullfile('FMUArchive', 'model.png'));
        else
            % use the generic Simulink logo
            copyfile(fullfile(grtfmi_dir, 'model.png'), fullfile('FMUArchive', 'model.png'));
        end
        
        pathstr = which('rtwsfcnfmi.tlc');
        [cmakelists_dir, ~, ~] = fileparts(pathstr);
        command = get_param(modelName, 'CMakeCommand');
        command = grtfmi_find_cmake(command);
        generator = get_param(modelName, 'CMakeGenerator');
        
        % MATLAB version for conditional compilation
        if verLessThan('matlab', '7.12')
            matlab_version = '';  % do nothing
        elseif verLessThan('matlab', '8.5')
            matlab_version = 'MATLAB_R2011a_';  % R2011a - R2014b
        elseif verLessThan('matlab', '9.3')
            matlab_version = 'MATLAB_R2015a_';  % R2015a - R2017a
        else
            matlab_version = 'MATLAB_R2017b_';  % R2017b - R2018b
        end
        
        solver = buildOpts.solver;
        if ~strcmp(solver, {'ode1', 'ode2', 'ode3', 'ode4', 'ode5', 'ode8', 'ode14x'})
            solver = 'ode1';  % use ode1 for model exchange
        end
        
        % get model sources
        [custom_include, custom_source, custom_library, mex_functions] = ...
            rtwsfcnfmi_model_sources(modelName, pwd);
        
        custom_include = cmake_list(custom_include);
        custom_source  = cmake_list(custom_source);
        custom_library = cmake_list(custom_library);
        
        % copy binary S-functions
        if strcmp(get_param(modelName, 'LoadBinaryMEX'), 'on')
            for i = 1:numel(mex_functions)
                copyfile(which(mex_functions{i}), fullfile('FMUArchive', 'binaries', fmi_platform));
            end
        end
        
        % write the CMakeCache.txt file
        fid = fopen('CMakeCache.txt', 'w');
        fprintf(fid, 'MODEL_NAME:STRING=%s\n', modelName);
        fprintf(fid, 'SOLVER:STRING=%s\n', solver);
        fprintf(fid, 'RTW_DIR:STRING=%s\n', strrep(pwd, '\', '/'));
        fprintf(fid, 'MATLAB_ROOT:STRING=%s\n', strrep(matlabroot, '\', '/'));
        fprintf(fid, 'MATLAB_VERSION:STRING=%s\n', matlab_version);
        fprintf(fid, 'CUSTOM_INCLUDE:STRING=%s\n', custom_include);
        fprintf(fid, 'CUSTOM_SOURCE:STRING=%s\n', custom_source);
        fprintf(fid, 'CUSTOM_LIBRARY:STRING=%s\n', custom_library);
        fprintf(fid, 'BINARY_SFUNCTIONS:STRING=%s\n', cmake_list(mex_functions));
        %fprintf(fid, 'COMPILER_OPTIMIZATION_LEVEL:STRING=%s\n', get_param(modelName, 'CMakeCompilerOptimizationLevel'));
        %fprintf(fid, 'COMPILER_OPTIMIZATION_FLAGS:STRING=%s\n', get_param(modelName, 'CMakeCompilerOptimizationFlags'));
        fclose(fid);
        
        disp('### Generating project')
        status = system(['"' command '" -G "' generator '" "' strrep(cmakelists_dir, '\', '/') '"']);
        assert(status == 0, 'Failed to run CMake generator');

        disp('### Building FMU')
        status = system(['"' command '" --build . --config Release']);
        assert(status == 0, 'Failed to build FMU');
        
        % copy the FMU to the working directory
        copyfile([modelName '.fmu'], '..');
end

end


function joined = cmake_list(array)

if isempty(array)
    joined = '';
    return
end

joined = array{1};

for i = 2:numel(array)
    joined = [joined ';' array{i}];  %#ok<ARGROW>
end

end
