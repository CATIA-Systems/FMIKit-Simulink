function grtfmi_make_rtw_hook(hookMethod, modelName, rtwRoot, templateMakefile, buildOpts, buildArgs, buildInfo)

switch hookMethod

    case 'after_make'

        current_dir = pwd;

        % remove fmiwrapper.inc for referenced models
        if ~strcmp(current_dir(end-11:end), '_grt_fmi_rtw')
            delete('fmiwrapper.inc');
            return
        end

        if strcmp(get_param(gcs, 'GenCodeOnly'), 'on')
            return
        end

        pathstr = which('grtfmi.tlc');
        [grtfmi_dir, ~, ~] = fileparts(pathstr);
        
        command = get_param(modelName, 'CMakeCommand');
        command = grtfmi_find_cmake(command);
        generator = get_param(modelName, 'CMakeGenerator');
        source_code_fmu = get_param(modelName, 'SourceCodeFMU');
        fmi_version = get_param(modelName, 'FMIVersion');

        disp('### Running CMake generator')
        custom_include = get_param(gcs, 'CustomInclude');
        custom_include = regexp(custom_include, '\s+', 'split');
        source_files = get_param(gcs, 'CustomSource');
        source_files = regexp(source_files, '\s+', 'split');
        custom_source = {};
        for i = 1:length(source_files)
             source_file = which(source_files{i});
             if ~isempty(source_file)
                custom_source{end+1} = source_file; %#ok<AGROW>
             end
        end

        if isfield(buildOpts, 'libsToCopy') && ~isempty(buildOpts.libsToCopy)
            [parent_dir, ~, ~] = fileparts(pwd);
            custom_include{end+1} = fullfile(parent_dir, 'slprj', 'grtfmi', '_sharedutils');
            for i = 1:numel(buildOpts.libsToCopy)
                [~, refmodel, ~] = fileparts(buildOpts.libsToCopy{i});
                refmodel = refmodel(1:end-7);
                custom_include{end+1} = fullfile(parent_dir, 'slprj', 'grtfmi', refmodel); %#ok<AGROW>
                custom_source{end+1}  = fullfile(parent_dir, 'slprj', 'grtfmi', refmodel, [refmodel '.c']); %#ok<AGROW>
            end
        end
        
        % get non-inlined S-functions
        if isfield(buildOpts, 'noninlinedSFcns')
            sfuns = buildOpts.noninlinedSFcns;
        else
            sfuns = Simulink.sfunction.analyzer.findSfunctions(modelName);
        end
        
        % add S-function sources
        for i = 1:numel(sfuns)
            sfcn = which(sfuns{i});
            [sfcn_dir, sfcn_name, ~] = fileparts(sfcn);
            src_file_ext = {'.c', '.cc', '.cpp', '.cxx', '.c++'};
            for j = 1:numel(src_file_ext)
                ext = src_file_ext{j};
                if exist(fullfile(sfcn_dir, [sfcn_name ext]), 'file') == 2
                    custom_source{end+1} = fullfile(sfcn_dir, [sfcn_name ext]); %#ok<AGROW>
                    break
                end
            end
        end

        % write the CMakeCache.txt file
        fid = fopen('CMakeCache.txt', 'w');
        fprintf(fid, 'MODEL:STRING=%s\n', modelName);
        fprintf(fid, 'RTW_DIR:STRING=%s\n', strrep(pwd, '\', '/'));
        fprintf(fid, 'MATLAB_ROOT:STRING=%s\n', strrep(matlabroot, '\', '/'));
        fprintf(fid, 'CUSTOM_INCLUDE:STRING=%s\n', build_path_list(custom_include));
        fprintf(fid, 'SOURCE_CODE_FMU:BOOL=%s\n', upper(source_code_fmu));
        fprintf(fid, 'FMI_VERSION:STRING=%s\n', fmi_version);
        fprintf(fid, 'CUSTOM_SOURCE:STRING=%s\n', build_path_list(custom_source));
        fprintf(fid, 'COMPILER_OPTIMIZATION_LEVEL:STRING=%s\n', get_param(gcs, 'CMakeCompilerOptimizationLevel'));
        fprintf(fid, 'COMPILER_OPTIMIZATION_FLAGS:STRING=%s\n', get_param(gcs, 'CMakeCompilerOptimizationFlags'));
        fclose(fid);
        
        disp('### Generating project')
        status = system(['"' command '" -G "' generator '" "' strrep(grtfmi_dir, '\', '/') '"']);
        assert(status == 0, 'Failed to run CMake generator');

        disp('### Building FMU')
        status = system(['"' command '" --build . --config Release']);
        assert(status == 0, 'Failed to build FMU');

        % copy the FMU to the working directory
        copyfile([modelName '.fmu'], '..');
end

end

function list = build_path_list(segments)

list = '';

for i = 1:numel(segments)
  segment = segments{i};
  if ~isempty(segment)
    if isempty(list)
      list = segment;
    else
      list = [segment ';' list]; %#ok<AGROW>
    end
  end
end

list = strrep(list, '\', '/');

end
