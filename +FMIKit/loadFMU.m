function loadFMU(block, fmu)
% FMIKit.loadFMU  Load an FMU with an existing FMU block
%
% Example:
%
%   fmu_block = add_block('FMIKit_blocks/FMU', [gcs '/IntegerNetwork1']);
%   FMIKit.loadFMU(fmu_block, 'IntegerNetwork1.fmu');

dialog = FMIKit.showBlockDialog(block, false);

mdlfile = get_param(bdroot(block), 'Filename');
[folder, ~, ~] = fileparts(mdlfile);
dialog.mdlDirectory = folder;

dialog.txtFMUPath.setText(fmu);
dialog.loadFMU(false);
applyDialog(dialog);

end
