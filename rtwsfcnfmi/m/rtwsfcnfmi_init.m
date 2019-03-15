function rtwsfcnfmi_init
% RTWSFCNFMI_INIT
%
%	Initialize the FMU export

% check MATLAB version
rel = version('-release');
rel_year = str2double(rel(1:4));

if rel_year < 2010 || rel_year > 2018
    warning('FMI Kit FMU Export is only supported on MATLAB R2010a - R2018b')
    return
end

rtwsfcnfmi_root = fileparts(fileparts(which(mfilename)));

% add the code folders to the MATLAB path
addpath(fullfile(rtwsfcnfmi_root, 'm'));
addpath(fullfile(rtwsfcnfmi_root, 'tlc'));

% set environment variable
setenv('SFCN_FMI_ROOT', rtwsfcnfmi_root);
