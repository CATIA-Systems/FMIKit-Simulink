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
end

end
