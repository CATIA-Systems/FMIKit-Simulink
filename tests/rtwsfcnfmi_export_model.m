function rtwsfcnfmi_export_model(model, varargin)
% export model with rtwsfcnfmi.tlc

if rem(numel(varargin), 2) ~= 0
    error('Wrong number of parameters')
end

load_system(model);

set_param(model, 'GenerateReport', 'off');

set_param(model, 'SolverType', 'Variable-step');

cs = getActiveConfigSet(gcs);
switchTarget(cs, 'rtwsfcnfmi.tlc', []);

for i = 1:2:numel(varargin)
    set_param(model, varargin{i}, varargin{i+1})
end

rtwbuild(model);

close_system(model, 0);

end

