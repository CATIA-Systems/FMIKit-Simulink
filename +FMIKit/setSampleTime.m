function setSampleTime(block, sampleTime)
% FMIKit.setSampleTime  Set the sample time for an FMU block
%
% Example:
%
%   FMIKit.setSampleTime(gcb, '1e-3');
%
%   sets the sample time of the FMU block to 1e-3.

assert(strcmp(get_param(block, 'ReferenceBlock'), 'FMIKit_blocks/FMU'), 'Block is not an FMU')

assert(isa(sampleTime, 'char'), 'Argument sampleTime must be a char array')

userData = getUserData(block);

userData.sampleTime = sampleTime;

set_param(block, 'UserData', userData, 'UserDataPersistent', 'on');

dialog = FMIKit.showBlockDialog(block, false);

applyDialog(dialog);

end
