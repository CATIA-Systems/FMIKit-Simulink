function dialog = getBlockDialog(block)
% Internal API - do not use

if ~ishandle(block)
    block = get_param(block, 'Handle');
end

% check library version
libraryVersion = char(fmikit.ui.FMUBlockDialog.FMI_KIT_VERSION);

if ~strcmp(fmikit.ui.FMUBlockDialog.FMI_KIT_VERSION, FMIKit.version)
    error(['Wrong fmikit.jar version. Expected ' FMIKit.version ' but was ' libraryVersion '.'])
end

dialog = javaMethod('getDialog', 'fmikit.ui.FMUBlockDialog', block);

end
