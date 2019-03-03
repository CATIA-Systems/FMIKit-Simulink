function closeBlockDialog(blockHandle)
% Internal API - do not use

javaMethod('closeDialog', 'fmikit.ui.FMUBlockDialog', blockHandle);

end
