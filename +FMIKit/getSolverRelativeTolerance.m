function reltol = getSolverRelativeTolerance(model)
% Get the relative tolerance of the variable step solver for model

if strcmp(get_param(model, 'SolverType'), 'Variable-step')
    reltol = str2double(get_param(model, 'RelTol'));
else
    reltol = 0;
end
