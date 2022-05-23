function setStartValue(block, varargin)
% FMIKit.setStartValue  Set the start value of variables.
%
% FMIKit.setStartValue(block, variable1, value1, variable2, value2, ...)
%
%
% Examples:
%
%    FMIKit.setStartValue(gcb, 'step', true)
%
% sets the start value of variable 'step' to true.
% 
%    FMIKit.setStartValue(gcb, 'step', [])
%
% resets the start value of variable 'step' to its default
% start value.
%
%   FMIKit.setStartValue(gcb, 'u', [1 2 3]')
%
% sets the variables u[1] = 1, u[2] = 2 and u[3] = 3.
%
%   FMIKit.setStartValue(gcb, 'table', [1 2 3; 4 5 6])
%
% sets the variables table[1,1] = 1 ... table[2,3] = 6.

userData = getUserData(block);

assert(~isempty(userData), 'Block is not an FMU');

assert(numel(varargin) > 1 && mod(numel(varargin), 2) == 0, 'Wrong number of arguments');

for i = 1:2:numel(varargin)   
    assert(ischar(varargin{i}),  'Variable name must be a string');
    assert(ndims(varargin{i+1}) < 3, 'Start value cannot have more than 2 dimensions');
end

dialog = FMIKit.showBlockDialog(block, false);

for i = 1:2:numel(varargin)
    
    variableName = java.lang.String(varargin{i});
    
    if isempty(varargin{i+1})
        % reset the variable
        dialog.setStartValue(variableName, []);
    elseif ischar(varargin{i+1})
        % string
        dialog.setStartValue(variableName, java.lang.String(varargin{i+1}));
    else
        % scalar or array
        if isscalar(varargin{i+1})
            startValue = java.lang.String(num2str(varargin{i+1}));        
            dialog.setStartValue(variableName, startValue);
        else
            [names, values] = split_array(varargin{i}, varargin{i+1});
            for j = 1:numel(names)
                dialog.setStartValue(java.lang.String(names{j}), java.lang.String(values{j}));
            end
        end
    end    
    
end

applyDialog(dialog);

end

function [names, values] = split_array(name, A)

names  = cell(numel(A), 1);
values = cell(numel(A), 1);

s = size(A);

if s(2) == 1
    for i = 1:numel(A)
        names{i}  = [name '[' num2str(i) ']'];
        values{i} = num2str(A(i));
    end
else
    for i = 1:numel(A)
        [m n] = ind2sub(s, i);
        names{i}  = [name '[' num2str(m) ',' num2str(n) ']'];
        values{i} = num2str(A(i));
    end
end

end
