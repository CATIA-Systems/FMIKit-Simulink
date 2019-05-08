function rtwsfcnfmi_selectcallback(hDlg, hSrc)

slConfigUISetVal(hDlg, hSrc, 'TargetHWDeviceType', 'MATLAB Host'); 
set_param(hSrc, 'CompOptLevelCompliant', 'on');

% disable makefile generation
slConfigUISetVal(hDlg, hSrc, 'GenerateMakefile', 'off');

end

