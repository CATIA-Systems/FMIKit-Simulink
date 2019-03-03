function setUseSourceCode(block, useSourceCode)
% FMIKit.setUseSourceCode  Enable source code for an FMU block
%
% Example:
%
%   FMIKit.setUseSourceCode(gcb, true);
%
%   enables source code for the current block.

assert(strcmp(get_param(block, 'ReferenceBlock'), 'FMIKit_blocks/FMU'), 'Block is not an FMU')

userData = getUserData(block);

userData.useSourceCode = useSourceCode;

set_param(block, 'UserData', userData, 'UserDataPersistent', 'on');

dialog = FMIKit.showBlockDialog(block, false);

applyDialog(dialog);

end
