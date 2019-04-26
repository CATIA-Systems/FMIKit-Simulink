function userData = updateUserData( userData )
% Update the UserData block parameter from a previous version

if strcmp(userData.fmiKitVersion, '2.4')
    
    disp('Updating UserData to 2.6')
    
    % add the directInput field 
    if ~isfield(userData, 'directInput')
        disp(['adding userData.directInput to ' block])
        userData.directInput = false;
        userData.parameters = strrep(userData.parameters, 'logical(',  'false logical(');
        set_param(block, 'UserData', userData, 'UserDataPersistent', 'on')
        save_system
    end
    
    % 
    userData.fmiKitVersion = '2.6';

    
    %TODO: re-import FMU
    
    
end

end

