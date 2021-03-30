function resettable = isResettable(block)
% FMIKit.isResettable  True if the FMU block is resettable 
%
% Example:
%
%   resettable = FMIKit.isResettable(gcb)
resettable = false;

userData = getUserData(block);

if ~isempty(userData)
    resettable = userData.resettable;
end

end
