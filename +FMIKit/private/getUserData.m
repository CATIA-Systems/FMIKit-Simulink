function userData = getUserData(block)

userData = get_param(block, 'UserData');

if isempty(userData)
    return
end

if any(strcmp(userData.fmiKitVersion, {'2.4', '2.6'}))

    disp(['Updating ' getfullname(block) ' that was imported with an older version of FMI Kit.'])

    userData.fmiKitVersion = '2.7';
    set_param(block, 'UserData', userData);

    % re-import the FMU
    dialog = FMIKit.showBlockDialog(block, false);
    dialog.loadFMU(false);
    applyDialog(dialog);

    model = bdroot(block);
    set_param(model, 'Dirty', 'on');

    disp('Save the model to apply the changes.')

    userData = get_param(block, 'UserData');

end

if ~isfield(userData, 'relativeTolerance')
    disp(['Adding userData.relativeTolerance to ' getfullname(block)])
    userData.relativeTolerance = '0';
    set_param(block, 'UserData', userData, 'UserDataPersistent', 'on')
    save_system
end

if ~isfield(userData, 'logFMICalls')
    disp(['Adding userData.logFMICalls to ' getfullname(block)])
    userData.logFMICalls = false;
    set_param(block, 'UserData', userData, 'UserDataPersistent', 'on')
    save_system
end

if ~isfield(userData, 'logLevel')
    disp(['Adding userData.logLevel to ' getfullname(block)])
    userData.logLevel = 0;
    set_param(block, 'UserData', userData, 'UserDataPersistent', 'on')
    save_system
end

if ~isfield(userData, 'logFile')
    disp(['Adding userData.logFile to ' getfullname(block)])
    userData.logFile = '';
    set_param(block, 'UserData', userData, 'UserDataPersistent', 'on')
    save_system
end

if ~isfield(userData, 'logToFile')
    disp(['Adding userData.logToFile to ' getfullname(block)])
    userData.logToFile = false;
    set_param(block, 'UserData', userData, 'UserDataPersistent', 'on')
    save_system
end

end
