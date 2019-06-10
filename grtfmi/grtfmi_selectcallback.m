function grtfmi_selectcallback(hDlg, hSrc)
  
slConfigUISetVal(hDlg, hSrc, 'GenerateMakefile', 'off');
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
