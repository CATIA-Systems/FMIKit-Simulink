function dialog = getBlockDialog(block)
% Internal API - do not use

if ~ishandle(block)
    block = get_param(block, 'Handle');
end

% check library version
libraryVersion = char(fmikit.ui.FMUBlockDialog.FMI_KIT_VERSION);

% still using the 2.7 format for userData struct
if ~strcmp(fmikit.ui.FMUBlockDialog.FMI_KIT_VERSION, '2.7')
    error(['Wrong fmikit.jar version. Expected 2.7 but was ' libraryVersion '.'])
end

dialog = javaMethod('getDialog', 'fmikit.ui.FMUBlockDialog', block);

end
