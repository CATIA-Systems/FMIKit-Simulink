function grtfmi_selectcallback(hDlg, hSrc)

% set target language to "C"
slConfigUISetVal(hDlg, hSrc, 'TargetLang', 'C');
slConfigUISetEnabled(hDlg, hSrc, 'TargetLang', false);

% disable "Package code and artifacts"
slConfigUISetVal(hDlg, hSrc, 'PackageGeneratedCodeAndArtifacts', 'off');
slConfigUISetEnabled(hDlg, hSrc, 'PackageGeneratedCodeAndArtifacts', false);

% disable "Generate makefile"
slConfigUISetVal(hDlg, hSrc, 'GenerateMakefile', 'off');
slConfigUISetEnabled(hDlg, hSrc, 'GenerateMakefile', false);

% keep the *.rtw file
slConfigUISetVal(hDlg, hSrc, 'RetainRTWFile', 'on');

% Reusable functions are not supported in R2012b
params = get_param(gcs ,'ObjectParameters');
if isfield(params, 'CodeInterfacePackaging')
    slConfigUISetVal(hDlg, hSrc, 'CodeInterfacePackaging', 'Nonreusable function');
    slConfigUISetEnabled(hDlg, hSrc, 'CodeInterfacePackaging', false);
end

% disable Mat file logging
slConfigUISetVal(hDlg, hSrc, 'MatFileLogging', 'off');
slConfigUISetEnabled(hDlg, hSrc, 'MatFileLogging', false);

% declare model reference compliance
slConfigUISetVal(hDlg, hSrc, 'ModelReferenceCompliant', 'on');
slConfigUISetEnabled(hDlg, hSrc, 'ModelReferenceCompliant', false);

end
