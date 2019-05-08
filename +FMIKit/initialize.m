function initialize()
% FMIKit.initialize  Initialize FMI Kit
%
%   Sets the paths and environment variables for FMI Kit.
%   This function must be called before FMI Kit can be used.

msg = false;

info = what('+FMIKit');
[folder, ~, ~] = fileparts(info(1).path);

% add the src folder to the MATLAB path
if isempty(which('FMU.cpp'))
    src_folder = fullfile(folder, 'src');
    if exist(src_folder, 'dir')
        addpath(src_folder);
        msg = true;
    end
end

% add the GRTFMI target to the MATLAB path
if isempty(which('grtfmi.tlc'))
    grtfmi_folder = fullfile(folder, 'grtfmi');
    if exist(grtfmi_folder, 'dir')
        addpath(grtfmi_folder);
        msg = true;
    end
end

% add the RTWSFCNFMI target to the MATLAB path
if isempty(which('rtwsfcnfmi.tlc'))
    rtwsfcnfmi_folder = fullfile(folder, 'rtwsfcnfmi');
    if exist(rtwsfcnfmi_folder, 'dir')
        addpath(rtwsfcnfmi_folder);
        msg = true;
    end
end

% add the Java libraries to the path
if isempty(which('org.jdesktop.swingx.JXTreeTable'))
    javaaddpath(fullfile(folder, 'swingx-1.6.jar'))
    msg = true;
end

if isempty(which('com.intellij.uiDesigner.core.GridLayoutManager'))
    javaaddpath(fullfile(folder, 'forms_rt.jar'))
    msg = true;
end

if isempty(which('fmikit.ui.FMUBlockDialog'))
    javaaddpath(fullfile(folder, 'fmikit.jar'))
    msg = true;
end

% delete re-saved block library
close_system('FMIKit_blocks', 0);
library_file = fullfile(folder, 'FMIKit_blocks.slx');
if exist(library_file, 'file')
    delete(library_file);
end

% re-save block library for current release
h = load_system('FMIKit_blocks_R2012b');
save_system(h, library_file);
close_system(h);

% add repository information to avoid "fix" message in library browser
if ~verLessThan('matlab', '8.4') % R2014b
  try
    h = load_system('FMIKit_blocks');
    if (strcmp(get_param(h, 'EnableLBRepository'), 'off'))
      set_param(h, 'Lock', 'off');
      set_param(h, 'EnableLBRepository', 'on');
      set_param(h, 'Lock', 'on');
      save_system(h);
    end
    close_system(h);
  catch
  end
end

% add examples to MATLAB path
if isempty(which('fmikit_demo_BouncingBall'))
    addpath(fullfile(folder, 'examples'))
    msg = true;
end

if msg
    disp(['Initializing FMI Kit ' [num2str(FMIKit.majorVersion) '.' ...
        num2str(FMIKit.minorVersion) '.' num2str(FMIKit.patchVersion)]])
        
    % check MATLAB version
    rel = version('-release');
    rel_year = str2double(rel(1:4));
    if rel(5) ~= 'a'
        rel_year = rel_year + 0.1;
    end
    
    if rel_year < 2012.1
        warning('MATLAB releases prior to R2012b are not supported')
    end
end

end
