function startValue = getStartValue(block, variableName)
% FMIKit.getStartValue  Get the start value of a variable.
%
% Example:
%
%    step = FMIKit.getStartValue(gcb, 'step')
%
% returns the start value of variable 'step' or '' if no start
% value is defined.

assert(strcmp(get_param(gcb, 'ReferenceBlock'), 'FMIKit_blocks/FMU'), 'Block is not an FMU')
assert(ischar(variableName), 'variableName must be a string')

dialog = FMIKit.showBlockDialog(block, false);

startValue = char(dialog.getStartValue(java.lang.String(variableName)));     

end
