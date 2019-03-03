function labels = getInportLabels(block)
% Internal API - do not use

% get the input port labels as a cell array of strings

%#ok<*AGROW>

labels = {};

userData = getUserData(block);

if ~isempty(userData)
    for i = 1:numel(userData.inputPorts)
        labels{end+1} = userData.inputPorts(i).label;
    end
end

end