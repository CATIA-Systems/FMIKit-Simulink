function grtfmi_compile_wsl(varargin)
% Build a Linux binary on Windows with WSL for an FMU generated with grtfmi.tlc
%
% Parameters:
%   cmakeCommand    cmake command on Linux (default: 'cmake')
%   rtwDir          the FMU's RTW directory (default: pwd)
%
% Example:
%   grtfmi_compile_wsl('cmakeCommand', '/usr/bin/cmake', ...
%       'rtwDir', fullfile(pwd, 'f14_grt_fmi_rtw'))

p = inputParser;

addParameter(p, 'cmakeCommand', 'cmake', @ischar);
addParameter(p, 'rtwDir', pwd, @ischar);

parse(p, varargin{:});

cmake_command = p.Results.cmakeCommand;
rtw_dir       = p.Results.rtwDir;

[~,modelName,~] = fileparts(rtw_dir);
modelName = modelName(1:end-12);

pathstr = which('grtfmi.tlc');
[grtfmi_dir, ~, ~] = fileparts(pathstr);

[custom_include, custom_source, custom_library] = ...
    grtfmi_model_sources(modelName, rtw_dir);

custom_include = grtfmi_wslpath(custom_include);
custom_source  = grtfmi_wslpath(custom_source);
custom_library = grtfmi_wslpath(custom_library);
custom_define  = regexp(get_param(modelName, 'CustomDefine'), '\s+', 'split');
matlab_root    = grtfmi_wslpath(matlabroot);
rtw_dir        = grtfmi_wslpath(rtw_dir);

% check for Simscape blocks
if isempty(find_system(modelName, 'BlockType', 'SimscapeBlock'))
    simscape_blocks = 'off';
else
    simscape_blocks = 'on';
end

build_configuration = get_param(modelName, 'CMakeBuildConfiguration');
source_code_fmu     = get_param(modelName, 'SourceCodeFMU');
fmi_version         = get_param(modelName, 'FMIVersion');

% write the CMakeCache.txt file
fid = fopen(fullfile(p.Results.rtwDir, 'CMakeCache.txt'), 'w');
fprintf(fid, 'CMAKE_GENERATOR:STRING=Unix Makefiles\n');
fprintf(fid, 'CMAKE_BUILD_TYPE:STRING=%s\n', build_configuration);
fprintf(fid, 'MODEL_NAME:STRING=%s\n', modelName);
fprintf(fid, 'RTW_DIR:STRING=%s\n', rtw_dir);
fprintf(fid, 'MATLAB_ROOT:STRING=%s\n', matlab_root);
fprintf(fid, 'CUSTOM_INCLUDE:STRING=%s\n', cmake_list(custom_include));
fprintf(fid, 'CUSTOM_SOURCE:STRING=%s\n', cmake_list(custom_source));
fprintf(fid, 'CUSTOM_LIBRARY:STRING=%s\n', cmake_list(custom_library));
fprintf(fid, 'CUSTOM_DEFINE:STRING=%s\n', cmake_list(custom_define));
fprintf(fid, 'SOURCE_CODE_FMU:BOOL=%s\n', upper(source_code_fmu));
fprintf(fid, 'SIMSCAPE:BOOL=%s\n', upper(simscape_blocks));
fprintf(fid, 'FMI_VERSION:STRING=%s\n', fmi_version);
fclose(fid);

disp('### Generating project')

source_dir = grtfmi_wslpath(grtfmi_dir);

command = ['wsl "' cmake_command '" -S "' source_dir '" -B "' rtw_dir '"'];

status = system(command);
assert(status == 0, 'Failed to run CMake generator');

disp('### Building FMU')

command = ['wsl "' cmake_command '" --build "' rtw_dir '"'];
status = system(command);
assert(status == 0, 'Failed to build FMU');

end

function joined = cmake_list(array)

if isempty(array)
    joined = '';
    return
end

joined = array{1};

for i = 2:numel(array)
    joined = [joined ';' array{i}];  
end

end

function wslpath = grtfmi_wslpath(path)

if isempty(path)
    wslpath = path;
    return
end

if iscell(path)
    for i = 1:length(path)
        [status, p] = system(['wsl wslpath -a "' path{i} '"']);
        assert(status == 0);
        wslpath{i} = strtrim(p);
    end
else
    [status, wslpath] = system(['wsl wslpath -a "' path '"']);
    assert(status == 0);
    wslpath = strtrim(wslpath);
end

end
