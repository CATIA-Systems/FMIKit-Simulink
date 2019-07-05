function dialog = showBlockDialog(block, varargin)
% Internal API - do not use

% check if this is the library block
if strcmp(get_param(bdroot(block), 'Name'), 'FMIKit_blocks')
    helpdlg('Add the block to a model to import FMUs', 'Cannot open library block');
    return
end

mdlfile = get_param(bdroot(block), 'Filename');

% check if the model has been saved
if isempty(mdlfile)
    helpdlg('The model needs to be saved to import FMUs', 'Model is not saved')
    return
end

dialog = FMIKit.getBlockDialog(block);
mdlfile = get_param(bdroot(block), 'Filename');
[folder, ~, ~] = fileparts(mdlfile);
dialog.mdlDirectory = folder;

userData = getUserData(block);

if ~isempty(userData)
    userData = userDataFromStruct(userData);
    dialog.setUserData(userData);
    dialog.loadModelDescription();
end

set(handle(dialog.btnOK,    'CallbackProperties'), 'ActionPerformedCallback', @okButtonClicked);
set(handle(dialog.btnHelp,  'CallbackProperties'), 'ActionPerformedCallback', @helpButtonClicked);
set(handle(dialog.btnApply, 'CallbackProperties'), 'ActionPerformedCallback', @applyButtonClicked);

dialog.setBlockPath(getfullname(gcbh));
dialog.setLocationRelativeTo([]);

if numel(varargin) == 0 || varargin{1}
    dialog.showAsync();
end

end

function okButtonClicked(hObject, ~)

dialog = hObject.getRootPane().getParent();
applyDialog(dialog);            
dialog.close();

end

function helpButtonClicked(~, ~)

FMIKit.showBlockDialogHelp();

end

function applyButtonClicked(hObject, ~)

dialog = hObject.getRootPane().getParent();
applyDialog(dialog);            

end
