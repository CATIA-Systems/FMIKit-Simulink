function modelDescription = getModelDescription(fmu)
% FMIKit.getModelDescription  Get the model description from an FMU without extracting it
%
% Example:
%
%   md = FMIKit.getModelDescription('IntegerNetwork1.fmu')

modelDescription = javaMethod('readFromFMU', 'fmikit.ModelDescriptionReader', fmu);

end
