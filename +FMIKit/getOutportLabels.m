function labels = getOutportLabels(block)
% Internal API - do not use

% get the output port labels as a cell array of strings

%#ok<*AGROW>

labels = {};

userData = getUserData(block);

if ~isempty(userData)
    for i = 1:numel(userData.outputPorts)
        labels{end+1} = userData.outputPorts(i).label;
    end
end

end
