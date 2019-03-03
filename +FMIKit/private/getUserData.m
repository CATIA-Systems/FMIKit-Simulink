function userData = getUserData(block)

userData = get_param(block, 'UserData');

if isempty(userData)
    return
end


if ~isfield(userData, 'directInput')
    disp(['adding userData.directInput to ' block])
    userData.directInput = false;
    userData.parameters = strrep(userData.parameters, 'logical(',  'false logical(');
    set_param(block, 'UserData', userData, 'UserDataPersistent', 'on')
    save_system
end

end