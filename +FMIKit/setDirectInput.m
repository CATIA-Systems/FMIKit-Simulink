function setDirectInput(block, directInput)
% FMIKit.setDirectInput  Enable or disable direct input for an FMU block
%
% Example:
%
%   FMIKit.setDirectInput(gcb, true);
%
%   enables direct input for the current block.

assert(strcmp(get_param(block, 'ReferenceBlock'), 'FMIKit_blocks/FMU'), 'Block is not an FMU')

userData = getUserData(block);

userData.directInput = logical(directInput);

set_param(block, 'UserData', userData, 'UserDataPersistent', 'on');

dialog = FMIKit.showBlockDialog(block, false);

applyDialog(dialog);

end
