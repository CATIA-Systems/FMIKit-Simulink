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
        
        pathstr = which('grtfmi.tlc');
        [grtfmi_dir, ~, ~] = fileparts(pathstr);
        
        pathstr = which('rtwsfcnfmi.tlc');
        [rtwsfcnfmi_dir, ~, ~] = fileparts(pathstr);
        
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
        
        command = get_param(modelName, 'CMakeCommand');
        command = grtfmi_find_cmake(command);
        generator = get_param(modelName, 'CMakeGenerator');
        toolset             = get_param(modelName, 'CMakeToolset');
        build_configuration = get_param(modelName, 'CMakeBuildConfiguration');
        
        % MATLAB version for conditional compilation
        if verLessThan('matlab', '7.12')
            matlab_version = '';  % do nothing
        elseif verLessThan('matlab', '8.5')
            matlab_version = 'MATLAB_R2011a_';  % R2011a - R2014b
        elseif verLessThan('matlab', '9.3')
            matlab_version = 'MATLAB_R2015a_';  % R2015a - R2017a
        elseif verLessThan('matlab', '9.8')
            matlab_version = 'MATLAB_R2017b_';  % R2017b - R2019b
        else
            matlab_version = 'MATLAB_R2020a_';  % R2020a and later
        end
        
        solver = get_param(modelName, 'Solver');
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
            load_mex = 'YES';
        else
            load_mex = 'NO';
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
        fprintf(fid, 'LOAD_MEX:BOOL=%s\n', load_mex);
        fprintf(fid, 'BINARY_SFUNCTIONS:STRING=%s\n', cmake_list(mex_functions));
        fprintf(fid, 'COMPILER_OPTIMIZATION_LEVEL:STRING=%s\n', get_param(modelName, 'CMakeCompilerOptimizationLevel'));
        fprintf(fid, 'COMPILER_OPTIMIZATION_FLAGS:STRING=%s\n', get_param(modelName, 'CMakeCompilerOptimizationFlags'));
        fclose(fid);
        
        disp('### Generating project')

        cmake_options = [' -G "' generator '"'];

        if ispc
            generator_platform = get_param(modelName, 'CMakeGeneratorPlatform');
            cmake_options = [cmake_options ' -A ' generator_platform];
        end

        if ~isempty(toolset)
            cmake_options = [cmake_options ' -T "' toolset '"'];
        end

        status = system(['"' command '"' cmake_options ' "' strrep(rtwsfcnfmi_dir, '\', '/') '"']);
        assert(status == 0, 'Failed to run CMake generator');

        disp('### Building FMU')
        status = system(['"' command '" --build . --config ' build_configuration]);
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
