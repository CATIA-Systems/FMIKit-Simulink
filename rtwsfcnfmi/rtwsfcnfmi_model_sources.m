function varargout = rtwsfcnfmi_model_sources(model, rtw_dir)

% convert to absolute path
if ~strncmp(rtw_dir, pwd, numel(pwd))
    rtw_dir = fullfile(pwd, rtw_dir);
end

sources = {};

% generated RTW model
include{1} = rtw_dir;

gen_sources = dir(fullfile(rtw_dir, '*.c'));

for i = 1:numel(gen_sources)
    filename = gen_sources(i).name;
    if strcmp(filename, [model '_sf.c']) || ...
        strcmp(filename, 'rt_nonfinite.c')
        continue
    end
    sources{end+1} = fullfile(rtw_dir, gen_sources(i).name); %#ok<AGROW>
end

% S-functions sources
mex_functions = {};
modules = {};
sfcns = find_system(model, 'LookUnderMasks', 'on', 'FollowLinks', 'on', 'BlockType', 'S-Function');
for i = 1:numel(sfcns)
    block = sfcns{i};
    function_name = get_param(block, 'FunctionName');
    mex_functions{end+1} = which(function_name);
    modules = [modules function_name ...
      regexp(get_param(block, 'SFunctionModules'), '\s+', 'split')]; %#ok<AGROW>
end
mex_functions = unique(mex_functions);
modules = unique(modules);

% add S-function sources
if strcmp(get_param(model, 'LoadBinaryMEX'), 'off')
    for i = 1:numel(modules)
        src_file_ext = {'.c', '.cc', '.cpp', '.cxx', '.c++'};
        for j = 1:numel(src_file_ext)
            source_file = which([modules{i} src_file_ext{j}]);
            if ~isempty(source_file) && ~any(strcmp(sources, source_file))
                sources{end+1} = source_file; %#ok<AGROW>
                break
            end
        end
    end
end

% custom includes, sources and libraries
include = [include split_path_list(get_param(model, 'CustomInclude'))];
sources = [sources split_path_list(get_param(model, 'CustomSource'))];
libraries = split_path_list(get_param(model, 'CustomLibrary'));

% replace backslashes with forward slashes
include   = strrep(include, '\', '/');
sources   = strrep(sources, '\', '/');
libraries = strrep(libraries, '\', '/');

if nargout == 4
   varargout = {include, sources, libraries, mex_functions};
else
   varargout = {{include, sources, libraries, mex_functions}};
end

end


function paths = split_path_list(path_list)
% convert a space separated, quoted list of paths to a cell array

paths = {};
p = strtrim(path_list);

path = '';

join = false;

for i=1:numel(p)
  
    c = p(i);

    if c == '"'
        join = ~join;
        continue
    end

    if c == ' ' && ~join
        if ~isempty(path)
            paths{end+1} = path; %#ok<AGROW>
        end
        path = '';
        continue
    end

    path(end+1) = c; %#ok<AGROW>

end

if ~isempty(path)
    paths{end+1} = path;
end

% replace backslashes
paths = strrep(paths, '\', '/');

end

