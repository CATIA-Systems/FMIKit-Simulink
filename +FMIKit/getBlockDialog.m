function dialog = getBlockDialog(block)
% Internal API - do not use

if ~ishandle(block)
    block = get_param(block, 'Handle');
end

dialog = javaMethod('getDialog', 'fmikit.ui.FMUBlockDialog', block);

end
