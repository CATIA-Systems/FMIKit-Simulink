function applyDialog(dialog)

%#ok<*AGROW>

% set the user data
userData = userDataToStruct(dialog.getUserData());
set_param(dialog.blockHandle, 'UserData', userData, 'UserDataPersistent', 'on');

if userData.useSourceCode

    % generate the S-function source
    dialog.generateSourceSFunction();

    model_identifier = char(dialog.getModelIdentifier());
    unzipdir = char(dialog.getUnzipDirectory());

    pkginfo = what('FMIKit');
    [fmikitdir, ~, ~] = fileparts(pkginfo.path);

    % build the S-function
    clear(['sfun_' model_identifier])

    disp(['Compiling S-function ' model_identifier])

    mex_args = {['sfun_' model_identifier '.c'], ...
                ['-I' fullfile(fmikitdir, 'include')], ...
                ['-I' fullfile(unzipdir, 'sources')], ...
                '-lshlwapi'};

    it = dialog.getSourceFiles().listIterator();

    while it.hasNext()
        mex_args{end+1} = fullfile(unzipdir, 'sources', it.next());
    end

    try
        mex(mex_args{:})
    catch e
        disp('Failed to compile S-function')
        disp(e.message)
        % rethrow(e)
    end

end

FMIKit.setSFunctionParameters(dialog.blockHandle)

end
