function initialize()
% FMIKit.initialize  Initialize FMI Kit
%
%   Sets the paths and environment variables for FMI Kit.
%   This function must be called before FMI Kit can be used.

msg = false;

info = what('FMIKit');
[folder, ~, ~] = fileparts(info(1).path);
      
% initialize FMU export
if isempty(which('rtwsfcnfmi_init'))
    rtwsfcnfmi_m_folder = fullfile(folder, 'rtwsfcnfmi', 'm');
    if exist(rtwsfcnfmi_m_folder, 'dir')
        addpath(rtwsfcnfmi_m_folder);
    end
end
if ~isempty(which('rtwsfcnfmi_init'))
    rtwsfcnfmi_init();
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

if msg
    disp(['Initializing FMI Kit ' FMIKit.version()])
    
    % check MATLAB version
    rel = version('-release');
    rel_year = str2double(rel(1:4));
    if rel(5) ~= 'a'
        rel_year = rel_year + 0.1;
    end
    
    if rel_year < 2012.1 || rel_year > 2019.0
        warning('FMU import is only supported on MATLAB R2012b - R2019a')
    end
    
    if rel_year < 2012.1 || rel_year > 2018.1
        warning('FMU export is only supported on MATLAB R2012b - R2018b')
    end
end

end
