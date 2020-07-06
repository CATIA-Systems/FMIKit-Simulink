function rtwsfcnfmi_selectcallback(hDlg, hSrc)

slConfigUISetVal(hDlg, hSrc, 'TargetHWDeviceType', 'MATLAB Host'); 
set_param(hSrc, 'CompOptLevelCompliant', 'on');

% set target language to "C"
slConfigUISetVal(hDlg, hSrc, 'TargetLang', 'C');
slConfigUISetEnabled(hDlg, hSrc, 'TargetLang', false);

% disable "Package code and artifacts"
slConfigUISetVal(hDlg, hSrc, 'PackageGeneratedCodeAndArtifacts', 'off');
slConfigUISetEnabled(hDlg, hSrc, 'PackageGeneratedCodeAndArtifacts', false);

% disable "Generate makefile"
slConfigUISetVal(hDlg, hSrc, 'GenerateMakefile', 'off');
slConfigUISetEnabled(hDlg, hSrc, 'GenerateMakefile', false);

end

