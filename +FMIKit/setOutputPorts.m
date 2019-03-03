function setOutputPorts(block, ports)
% FMIKit.setOutputPorts  Set the output ports of an FMU block.
%
% Example:
%   
%   ports.label = 'out1';
%   ports.variables = { 'x' };
%   ports(2).label = 'out2';
%   ports(2).variables = { 'y1', 'y2' };
%
%   FMIKit.setOutputPorts(gcb, ports)

assert(strcmp(get_param(block, 'ReferenceBlock'), 'FMIKit_blocks/FMU'), 'Block is not an FMU')

userData = getUserData(block);

userData.outputPorts = ports;

% if ~isempty(userData)
%     userData = userDataFromStruct(userData);
%     dialog.setUserData(userData);
%     dialog.loadModelDescription();
% end

set_param(block, 'UserData', userData, 'UserDataPersistent', 'on');


dialog = FMIKit.showBlockDialog(block, false);


applyDialog(dialog);

end
