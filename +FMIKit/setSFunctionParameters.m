function setSFunctionParameters(block)
% Internal API - do not use

userData = getUserData(block);
dialog = FMIKit.getBlockDialog(block);

if isempty(userData)
    return
end

set_param(block, 'FunctionName', userData.functionName, 'Parameters', userData.parameters)

mdl = getfullname(bdroot(block));
[mdldir, ~, ~] = fileparts(which(mdl));
unzipdir = fullfile(mdldir, userData.unzipDirectory);

if userData.useSourceCode

    pkginfo = what('FMIKit');
    [fmikitdir, ~, ~] = fileparts(pkginfo(1).path);

    include_dirs = { ...
      ['"' fmikitdir '"'], ...
      ['"' fullfile(fmikitdir, 'include') '"'], ...
      ['"' fullfile(unzipdir, 'sources') '"']
    };
    
    sources_files = {}; %#ok<*AGROW>
    it = dialog.getSourceFiles().listIterator();
    while it.hasNext()
        sources_files{end+1} = ['"' fullfile(unzipdir, 'sources', it.next()) '"'];
    end
    
    % S-function sources
    setSrcParams(mdl, 'SimUserIncludeDirs', include_dirs);
    setSrcParams(mdl, 'SimUserSources', sources_files);

    % RTW sources
    if strcmp(get_param(mdl, 'RTWUseSimCustomCode'), 'off')
        setSrcParams(mdl, 'CustomInclude', include_dirs);
        setSrcParams(mdl, 'CustomSource', sources_files);
    end
end

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
