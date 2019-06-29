function grtfmi_selectcallback(hDlg, hSrc)

% disable "Package code and artifacts"
slConfigUISetVal(hDlg, hSrc, 'PackageGeneratedCodeAndArtifacts', 'off');
slConfigUISetEnabled(hDlg, hSrc, 'PackageGeneratedCodeAndArtifacts', false);

% disable "Generate makefile"
slConfigUISetVal(hDlg, hSrc, 'GenerateMakefile', 'off');
slConfigUISetEnabled(hDlg, hSrc, 'GenerateMakefile', false);

slConfigUISetVal(hDlg, hSrc, 'RetainRTWFile', 'on');

% Reusable functions are not supported in R2012b
params = get_param(gcs ,'ObjectParameters');
if isfield(params, 'CodeInterfacePackaging')
    slConfigUISetVal(hDlg, hSrc, 'CodeInterfacePackaging', 'Reusable function');
end

% disable Mat file logging
slConfigUISetVal(hDlg, hSrc, 'MatFileLogging', 'off');
slConfigUISetEnabled(hDlg, hSrc, 'MatFileLogging', false);

% declare model reference compliance
slConfigUISetVal(hDlg, hSrc, 'ModelReferenceCompliant', 'on');
slConfigUISetEnabled(hDlg, hSrc, 'ModelReferenceCompliant', false);

end
