function showBlockDialogHelp()
% Open the documenation of the FMU block dialog

info = what('+FMIKit');
[folder, ~, ~] = fileparts(info(1).path);
web(fullfile(folder, 'html', 'fmu_import.html'));

end
