function setSFunctionParameters(block)
% Internal API - do not use

userData = getUserData(block);
dialog = FMIKit.getBlockDialog(block);

if isempty(userData)
    return
end

set_param(block, 'FunctionName', userData.functionName, 'Parameters', userData.parameters)

pkginfo = what('FMIKit');
[fmikitdir, ~, ~] = fileparts(pkginfo(1).path);
mdl = getfullname(bdroot(block));
[mdldir, ~, ~] = fileparts(which(mdl));
unzipdir = fullfile(mdldir, userData.unzipDirectory);

include_dirs = {['"' fullfile(fmikitdir, 'include') '"']};
sources_files = {}; %#ok<*AGROW>

if userData.useSourceCode
    % generated S-function
    include_dirs{end+1} = ['"' fullfile(unzipdir, 'sources') '"'];
    
    it = dialog.getSourceFiles().listIterator();
    
    while it.hasNext()
        sources_files{end+1} = ['"' fullfile(unzipdir, 'sources', it.next()) '"'];
    end
else
    % generic S-function
    sources_files{end+1} = ['"' fullfile(fmikitdir, 'src', 'FMU.cpp') '"'];
    sources_files{end+1} = ['"' fullfile(fmikitdir, 'src', 'FMU1.cpp') '"'];
    sources_files{end+1} = ['"' fullfile(fmikitdir, 'src', 'FMU2.cpp') '"'];
end

% S-function sources
setSrcParams(mdl, 'SimUserIncludeDirs', include_dirs);
setSrcParams(mdl, 'SimUserSources', sources_files);

% RTW sources
setSrcParams(mdl, 'CustomInclude',  include_dirs);
setSrcParams(mdl, 'CustomSource', sources_files);

end


function setSrcParams(object, name, values)

param_value = get_param(object, name);

for i=1:numel(values)
    value = values{i};
    if isempty(strfind(param_value, value))
        param_value = [param_value ' ' value];
    end
end

set_param(object, name, param_value);

end
