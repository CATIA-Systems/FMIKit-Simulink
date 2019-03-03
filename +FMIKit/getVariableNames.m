function variableNames = getVariableNames(modelDescription)
% FMIKit.getVariablesNames  Get the variable names from a model description
%
% Example:
%
%   md = FMIKit.getModelDescription('IntegerNetwork1.fmu');
%   names = FMIKit.getVariableNames(md);
%   for i=1:numel(names), disp(names{i}), end

n = modelDescription.scalarVariables.size();
variableNames = cell(1, n);

for i = 1:n
    variableNames{i} = char(modelDescription.scalarVariables.get(i-1).name);
end

end
