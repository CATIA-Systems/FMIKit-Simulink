function makeInfo = rtwmakecfg()

[root,~,~] = fileparts(mfilename('fullpath'));

makeInfo.includePath = { fullfile(root, 'include') };
makeInfo.sourcePath  = { fullfile(root, 'src') };
makeInfo.sources     = { 'multiply.c' };

% makeInfo.precompile = 1;
% 
% makeInfo.library(1).Name     = 'ShLwApi';
% makeInfo.library(1).Location = 'C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17763.0\um\x64';
% makeInfo.library(1).Modules  = {};

end
