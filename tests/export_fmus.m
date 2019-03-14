function export_fmus(varargin)

if numel(varargin) == 1
    fmus_dir = varargin{1};
else
    fmus_dir = 'fmus';
end

params = {
{'BouncingBall', '1.0', 'cs'}
{'BouncingBall', '1.0', 'me'}
{'BouncingBall', '2.0', 'cs'}
{'BouncingBall', '2.0', 'me'}
{'TestModel1', '1.0', 'cs'}
{'TestModel1', '1.0', 'me'}
{'TestModel1', '2.0', 'cs'}
{'TestModel1', '2.0', 'me'}
{'TestModel2', '1.0', 'cs'}
{'TestModel2', '1.0', 'me'}
{'TestModel2', '2.0', 'cs'}
{'TestModel2', '2.0', 'me'}
{'TriggeredSubsystems', '1.0', 'cs'}
{'TriggeredSubsystems', '1.0', 'me'}
{'TriggeredSubsystems', '2.0', 'cs'}
{'TriggeredSubsystems', '2.0', 'me'}
};

tic
for i = 1:size(params, 1)
    
    model_name = params{i}{1};
    fmi_version = params{i}{2};
    fmi_kind = params{i}{3};
        
    load_system(model_name);
    
    % write ref.opt file
    fid = fopen([model_name '_ref.opt'], 'w'); 
    fprintf(fid, 'StartTime,  %s\n', get_param(model_name, 'StartTime'));
    fprintf(fid, 'StopTime,%s\n', get_param(model_name, 'StopTime'));
    fprintf(fid, 'StepSize, 0\n'); % variable step
    fprintf(fid, 'RelTol, %s\n', get_param(model_name, 'RelTol'));
    fclose(fid);

    if strcmp(fmi_kind, 'me')
        build_fmu(model_name, fmi_version, 'ModelExchange');
    else
        build_fmu(model_name, fmi_version, 'CoSimulation');
    end
    
    model_dir = fullfile(fmus_dir, fmi_version, ...
        fmi_kind, 'win64', 'FMIKit', '2.6', model_name);
    
    mkdir(model_dir);

    copyfile([model_name '_ref.csv'], ...
        fullfile(model_dir, [model_name '_ref.csv']));
    
    movefile([model_name '_ref.opt'], ...
        fullfile(model_dir, [model_name '_ref.opt']));
    
    movefile([model_name '_sf.fmu'], ...
        fullfile(model_dir, [model_name '.fmu']));

    % close without saving
    close_system(model_name, 0);

end
toc

end


function build_fmu(model_name, fmi_version, fmi_kind)

% set the FMI version and kind
set_param(model_name, 'FMIVersion', fmi_version)
set_param(model_name, 'FMIType', fmi_kind)

% set the solver
if strcmp(fmi_kind, 'ModelExchange')
    set_param(model_name, 'SolverName', 'ode45')
else
    set_param(model_name, 'SolverName', 'ode1')
    set_param(model_name, 'FixedStep', '1e-2')
end

% export the FMU
rtwbuild(model_name)

end
