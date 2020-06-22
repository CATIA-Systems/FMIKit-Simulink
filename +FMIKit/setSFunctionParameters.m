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

includedir = ['"' fullfile(fmikitdir, 'include') '"'];
sources = ''; %#ok<*AGROW>

if userData.useSourceCode
    % generated S-function
    includedir = [includedir ' "' fullfile(unzipdir, 'sources') '"'];
    
    it = dialog.getSourceFiles().listIterator();
    
    while it.hasNext()
        sources = [sources ' "' fullfile(unzipdir, 'sources', it.next()) '"'];
    end
else
    % generic S-function
    sources = ['"' fullfile(fmikitdir, 'src', 'FMU.cpp') '" ' ...
               '"' fullfile(fmikitdir, 'src', 'FMU1.cpp') '" ' ...
               '"' fullfile(fmikitdir, 'src', 'FMU2.cpp') '"'];
end

% S-function sources
setSrcParam(mdl, 'SimUserIncludeDirs', includedir);
setSrcParam(mdl, 'SimUserSources', sources);

% RTW sources
setSrcParam(mdl, 'CustomInclude',  includedir);
setSrcParam(mdl, 'CustomSource', sources);

% libraries
if ispc
    try  % Windows SDK might not be installed
        winsdk = winqueryreg('HKEY_LOCAL_MACHINE', 'SOFTWARE\Microsoft\Microsoft SDKs\Windows', 'CurrentInstallFolder');
        libdir = fullfile(winsdk, 'Lib');
        if strcmp(computer('arch'), 'win64')
            libdir = fullfile(libdir, 'x64');
        end
        libraries  = ['"' fullfile(libdir, 'ShLwApi.Lib') '"'];
        setSrcParam(mdl, 'SimUserLibraries', libraries);
        setSrcParam(mdl, 'CustomLibrary', libraries);
    end
end

end


function setSrcParam(object, name, value)

old_value = get_param(object, name);

if isempty(strfind(old_value, value))
    new_value = [old_value ' ' value];
    set_param(object, name, new_value);
end

end
