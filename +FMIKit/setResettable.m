function setResettable(block, resettable)
% FMIKit.setResettable  Enable the resettable option of an FMU block.
%
% Example:
%
%   FMIKit.setResettable(gcb, true)

assert(strcmp(get_param(block, 'ReferenceBlock'), 'FMIKit_blocks/FMU'), 'Block is not an FMU')

userData = getUserData(block);

userData.resettable = resettable;

set_param(block, 'UserData', userData, 'UserDataPersistent', 'on');

dialog = FMIKit.showBlockDialog(block, false);

applyDialog(dialog);

end
