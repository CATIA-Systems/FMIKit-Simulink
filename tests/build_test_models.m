function build_test_models(varargin)

% compile timesthree.c
mex timesthree.c

switch mexext
    case 'mexa64'
        platform = 'linux64';
    case 'mexmaci64'
        platform = 'darwin64';
    case 'mexw32'
        platform = 'win32';
    case 'mexw64'
        platform = 'win64';
end

if numel(varargin) == 1
    fmus_dir = varargin{1};
else
    fmus_dir = 'fmus';
end

params = {
{'BouncingBall', 'cs'}
{'BouncingBall', 'me'}
{'TestModel1', 'cs'}
{'TestModel1', 'me'}
{'TestModel2', 'cs'}
{'TestModel2', 'me'}
{'TriggeredSubsystems', 'cs'}
{'TriggeredSubsystems', 'me'}
};

tic
for i = 1:size(params, 1)
    
    model_name = params{i}{1};
    fmi_kind = params{i}{2};
        
    load_system(model_name);
    
    % write ref.opt file
    fid = fopen([model_name '_ref.opt'], 'w'); 
    fprintf(fid, 'StartTime,  %s\n', get_param(model_name, 'StartTime'));
    fprintf(fid, 'StopTime,%s\n', get_param(model_name, 'StopTime'));
    fprintf(fid, 'StepSize, 0\n'); % variable step
    fprintf(fid, 'RelTol, %s\n', get_param(model_name, 'RelTol'));
    fclose(fid);

    if strcmp(fmi_kind, 'me')
        build_fmu(model_name, 'ModelExchange');
    else
        build_fmu(model_name, 'CoSimulation');
    end
    
    model_dir = fullfile(fmus_dir, '2.0', ...
        fmi_kind, platform, 'FMIKit', num2str(FMIKit.version), model_name);
    
    mkdir(model_dir);

    if exist([model_name '_in.csv'], 'file')
        copyfile([model_name '_in.csv'], ...
            fullfile(model_dir, [model_name '_in.csv']));
    end
    
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


function build_fmu(model_name, fmi_kind)

% set the FMI version and kind
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
