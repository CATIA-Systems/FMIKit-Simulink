function s = grtfmi_generate_lookup_table_fmi3(vr, variableName, record)
% generate XML and C code for a Simulink.LookupTable for FMI 3.0
%
% s.vr   next value reference
% s.xml  generated XML
% s.c    generated C code

table = evalin('base', record.WorkspaceVarName);

c   = '';
xml = '';

c = [c '    modelVariables[' num2str(vr-1) '].dtypeID = SS_DOUBLE;' newline];
c = [c '    modelVariables[' num2str(vr-1) '].size    = ' num2str(numel(table.Table.Value)) ';' newline];
c = [c '    modelVariables[' num2str(vr-1) '].address = ' record.Identifier '.Table;' newline];

dims = size(table.Table.Value);

xml = [xml '    <Float64 name="' variableName '.Table" valueReference="' num2str(vr) '" causality="parameter" variability="tunable" start="' arr2str(table.Table.Value(:)) '">' newline];

for i = 1:numel(dims)
  xml = [xml '      <Dimension start="' num2str(dims(i)) '"/>' newline];
end

xml = [xml '    </Float64>' newline]; 

vr = vr + 1;

for i = 1:numel(table.Breakpoints)

  c = [c '    modelVariables[' num2str(vr-1) '].dtypeID = SS_DOUBLE;' newline];
  c = [c '    modelVariables[' num2str(vr-1) '].size    = ' num2str(numel(table.Breakpoints(i).Value)) ';' newline];
  c = [c '    modelVariables[' num2str(vr-1) '].address = ' record.Identifier '.' table.Breakpoints(i).FieldName ';' newline];
  
  xml = [xml '    <Float64 name="' variableName '.' table.Breakpoints(i).FieldName '" valueReference="' num2str(vr) '" causality="parameter" variability="tunable" start="' arr2str(table.Breakpoints(i).Value(:)) '">' newline];
  xml = [xml '      <Dimension start="' num2str(numel(table.Breakpoints(i).Value)) '"/>' newline];
  xml = [xml '    </Float64>' newline];
  
  vr = vr + 1;
  
end

s.vr  = int32(vr);
s.xml = xml(1:end-1);
s.c   = c(1:end-1);

end


function s = arr2str(arr)

    s = num2cell(arr);
    s = cellfun(@num2str, s, 'UniformOutput', false);
    s = strjoin(s, ' ');

end
