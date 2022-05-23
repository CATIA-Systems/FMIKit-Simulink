function applyDialog(dialog)

%#ok<*AGROW>

block = dialog.blockHandle;

% break the library link
set_param(block, 'LinkStatus', 'none');

mask = Simulink.Mask.get(block);

mask.removeAllParameters();

userData = dialog.getUserData();

% add dialog parameters
for i = 0:userData.startValues.size()-1
    p = userData.startValues.get(i);
    mask.addParameter(...
      'Name', char(p.name), ...
      'Prompt', char(p.prompt), ...
      'Value', char(p.value));
end

% set the user data
userData = userDataToStruct(dialog.getUserData());
set_param(block, 'UserData', userData, 'UserDataPersistent', 'on');

% set the S-function parameters
FMIKit.setSFunctionParameters(block)

% draw the port labels
display = '';

ports = get_param(block, 'Ports');

for i = 1:min(numel(userData.inputPorts), ports(1))
    display = [display 'port_label(''input'', ' num2str(i) ', ''' userData.inputPorts(i).label ''');' sprintf('\n')];
end

if FMIKit.isResettable(gcb)
    display = [display 'port_label(''input'', ' num2str(ports(1)) ', ''reset'');' sprintf('\n')];
end

for i = 1:min(numel(userData.outputPorts), ports(2))
    display = [display 'port_label(''output'', ' num2str(i) ', ''' userData.outputPorts(i).label ''');' sprintf('\n')];
end

mask.Display = display;

if userData.useSourceCode

    % generate the S-function source
    dialog.generateSourceSFunction();

    model_identifier = char(dialog.getModelIdentifier());
    unzipdir = char(dialog.getUnzipDirectory());

    % build the S-function
    clear(['sfun_' model_identifier])

    disp(['Compiling S-function sfun_' model_identifier])

    mex_args = {};
    
    % debug build
    % mex_args{end+1} = '-g';
    
    % custom inlcude directories
    include_dirs = get_param(bdroot, 'SimUserIncludeDirs');
    include_dirs = split_paths(include_dirs);
    for i = 1:numel(include_dirs)
        mex_args{end+1} = ['-I"' include_dirs{i} '"'];
    end

    % shlwapi.lib for Dymola FMUs
    if ispc
        mex_args{end+1} = '-lshlwapi';
    end

    % custom libraries
    libraries = get_param(bdroot, 'SimUserLibraries');
    libraries = split_paths(libraries);
    for i = 1:numel(libraries)
        [library_path, library_name, ~] = fileparts(libraries{i});
        mex_args{end+1} = ['-L"' library_path '"'];
        mex_args{end+1} = ['-l' library_name];
    end

    % S-function source
    mex_args{end+1} = ['sfun_' model_identifier '.c'];

    % FMU sources
    it = dialog.getSourceFiles().listIterator();
    while it.hasNext()
        mex_args{end+1} = ['"' fullfile(unzipdir, 'sources', it.next()) '"'];
    end

    try
        mex(mex_args{:})
    catch e
        disp('Failed to compile S-function')
        disp(e.message)
        % rethrow(e)
    end

end

end


function l = split_paths(s)
% split a list of space separated and optionally quoted paths into
% a cell array of strings

s = [s ' ']; % append a space to catch the last path

l = {};  % path list

p = '';     % path
q = false;  % quoted path

for i = 1:numel(s)

    c = s(i); % current character

    if q
        if c == '"'
            q = false;
            if ~isempty(p)
                l{end+1} = p;
            end
            p = '';
        else
            p(end+1) = c;
        end
        continue
    end

    if c == '"'
        q = true;
    elseif c == ' '
        if ~isempty(p)
            l{end+1} = p;
        end
        p = '';
    else
        p(end+1) = c;
    end
end

end
