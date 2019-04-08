function rtwsfcnfmi_init
% RTWSFCNFMI_INIT
%
%	Initialize the FMU export

rtwsfcnfmi_root = fileparts(fileparts(which(mfilename)));

% add the code folders to the MATLAB path
addpath(fullfile(rtwsfcnfmi_root, 'm'));
addpath(fullfile(rtwsfcnfmi_root, 'tlc'));

% set environment variable
setenv('SFCN_FMI_ROOT', rtwsfcnfmi_root);
