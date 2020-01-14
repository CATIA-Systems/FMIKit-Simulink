function setInterfaceType(block, interfaceType)
% FMIKit.setInterfaceType  Set the interface type for the FMU block
%
% Example:
%
%   FMIKit.setInterfaceType(gcb, 'CoSimulation');
%
%   sets the interface type to Co-Simulation.

assert(strcmp(get_param(block, 'ReferenceBlock'), 'FMIKit_blocks/FMU'), ...
  'Block is not an FMU')

userData = getUserData(block);

switch interfaceType
  case 'ModelExchange'
    userData.runAsKind = 0;
  case 'CoSimulation'
    userData.runAsKind = 1;
  otherwise
    error('Argument interfaceType must be ''CoSimulation'' or ''ModelExchange''')
end

set_param(block, 'UserData', userData, 'UserDataPersistent', 'on');

dialog = FMIKit.showBlockDialog(block, false);

applyDialog(dialog);

end
