function varargout = grtfmi_model_sources(model, rtw_dir)

% convert to absolute path
if ~strncmp(rtw_dir, pwd, numel(pwd))
    rtw_dir = fullfile(pwd, rtw_dir);
end

% remember the current working directory
start_dir = pwd;

% file extentions for source files and static libaries
src_file_ext = {'.c', '.cc', '.cpp', '.cxx', '.c++'};

if ispc
    lib_file_ext = '.lib';
else
    lib_file_ext = '.a';
end

model_file = which(model);
[filepath,~,~] = fileparts(model_file);

include   = {rtw_dir, filepath};
sources   = {};
libraries = {};

gen_sources = dir(fullfile(rtw_dir, '*.c'));

for i = 1:numel(gen_sources)
    sources{end+1} = fullfile(rtw_dir, gen_sources(i).name); %#ok<AGROW>
end

% S-functions sources
sfcns = find_system(model, 'LookUnderMasks', 'on', 'FollowLinks', 'on', 'BlockType', 'S-Function');

for i = 1:numel(sfcns)
    
    block            = sfcns{i};
    sfun_name        = get_param(block, 'FunctionName');
    sfun_mexfile     = which([sfun_name '.' mexext]);
    [sfun_dir,~,~]   = fileparts(sfun_mexfile);
    sfun_modules     = [{sfun_name} regexp(get_param(block, 'SFunctionModules'), '\s+', 'split')];
    include          = {sfun_dir}; 
    sfun_source_dirs = {sfun_dir};
    
    if exist(fullfile(sfun_dir, 'rtwmakecfg.m'), 'file') || exist(fullfile(sfun_dir, 'rtwmakecfg.p'), 'file')
      
        cd(sfun_dir);
        makeInfo = rtwmakecfg();
        cd(start_dir);

        include          = [include          makeInfo.includePath]; %#ok<AGROW>
        sfun_source_dirs = [sfun_source_dirs makeInfo.sourcePath];  %#ok<AGROW>
        
        % add S-function sources
        for j = 1:numel(makeInfo.sources)
            for k = 1:numel(sfun_source_dirs)
                source_file = fullfile(sfun_source_dirs{k}, makeInfo.sources{j});
                if exist(source_file, 'file')
                    sources{end+1} = source_file; %#ok<AGROW>
                    break
                end
            end
        end
        
        if isfield(makeInfo, 'library')
            % add S-function libraries
            for j = 1:numel(makeInfo.library)
                libraries = [libraries fullfile(makeInfo.library(j).Location, [makeInfo.library(j).Name lib_file_ext])]; %#ok<AGROW>
            end        
        end
        
    end
    
    % add S-function modules
    for j = 1:numel(sfun_modules)
        for k = 1:numel(src_file_ext)
            for l = 1:numel(sfun_source_dirs)
                source_file = fullfile(sfun_source_dirs{l}, [sfun_modules{j} src_file_ext{k}]);
                if exist(source_file, 'file')
                    sources{end+1} = source_file; %#ok<AGROW>
                    break
                end
            end
        end
    end 
    
end

% referenced models
[parent_dir, ~, ~] = fileparts(rtw_dir);

referenced_models = find_mdlrefs(model);

include{end+1} = fullfile(parent_dir, 'slprj', 'grtfmi', '_sharedutils');

for i = 1:numel(referenced_models)
    referenced_model = referenced_models{i};
    if strcmp(referenced_model, model)
        continue
    end
    include{end+1} = fullfile(parent_dir, 'slprj', 'grtfmi', referenced_model); %#ok<AGROW>
    sources{end+1} = fullfile(parent_dir, 'slprj', 'grtfmi', referenced_model, [referenced_model '.c']); %#ok<AGROW>
end

% custom includes, sources and libraries
include   = [include   split_path_list(get_param(model, 'CustomInclude'))];
sources   = [sources   split_path_list(get_param(model, 'CustomSource'))];
libraries = [libraries split_path_list(get_param(model, 'CustomLibrary'))];

% replace backslashes with forward slashes
include   = strrep(include, '\', '/');
sources   = strrep(sources, '\', '/');
libraries = strrep(libraries, '\', '/');

% remove duplicates
include   = unique(include);
sources   = unique(sources);
libraries = unique(libraries);

if nargout == 3
   varargout = {include, sources, libraries};
else
   varargout = {{include, sources, libraries}};
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

