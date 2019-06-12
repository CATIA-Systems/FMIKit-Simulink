function setRelativeTolerance(block, reltol)
% FMIKit.setRelativeTolerance  Set the relative tolerance for an FMU block
%
% Example:
%
%   FMIKit.setRelativeTolerance(gcb, '1e-3');
%
%   sets the relative tolerance for a co-simulation FMU to 1e-3.

assert(strcmp(get_param(block, 'ReferenceBlock'), 'FMIKit_blocks/FMU'), 'Block is not an FMU')

assert(isa(reltol, 'char'), 'Argument reltol must be a char array')

userData = getUserData(block);

userData.relativeTolerance = reltol;

set_param(block, 'UserData', userData, 'UserDataPersistent', 'on');

dialog = FMIKit.showBlockDialog(block, false);

applyDialog(dialog);

end
