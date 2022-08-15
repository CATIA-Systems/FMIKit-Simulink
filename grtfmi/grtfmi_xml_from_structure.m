function s = grtfmi_xml_from_structure(vr, variableName, record)
% generate XML and C code for a Simulink.LookupTable
%
% s.vr   next value reference
% s.xml  generated XML
% s.c    generated C code

table = evalin('base', record.WorkspaceVarName);

c   = '';
xml = '';

c = [c '    for (size_t i = 0; i < ' num2str(numel(table.Table.Value)) '; i++) {' newline];
c = [c '        modelVariables[' num2str(vr-1) ' + i].dtypeID = SS_DOUBLE;' newline];
c = [c '        modelVariables[' num2str(vr-1) ' + i].size    = 0;' newline];
c = [c '        modelVariables[' num2str(vr-1) ' + i].address = &(' record.Identifier '.Table[i]);' newline];
c = [c '    }' newline];


dims = size(table.Table.Value);

for i = 1:numel(table.Table.Value)

  subs = ind2sub0(dims, i-1);
  subs = num2cell(subs);
  subs = cellfun(@num2str, subs, 'UniformOutput', false);
  subs = strjoin(subs, ',');

  xml = [xml '    <ScalarVariable name="' variableName '.Table[' subs ']" valueReference="' num2str(vr) '" causality="parameter" variability="tunable">' newline];
  xml = [xml '      <Real start="' num2str(table.Table.Value(i)) '"/>' newline];
  xml = [xml '    </ScalarVariable>' newline];
  vr = vr + 1;
end

for i = 1:numel(table.Breakpoints)

  c = [c newline];

  c = [c '    for (size_t i = 0; i < ' num2str(numel(table.Breakpoints(i).Value)) '; i++) {' newline];
  c = [c '        modelVariables[' num2str(vr-1) ' + i].dtypeID = SS_DOUBLE;' newline];
  c = [c '        modelVariables[' num2str(vr-1) ' + i].size    = 0;' newline];
  c = [c '        modelVariables[' num2str(vr-1) ' + i].address = &(' record.Identifier '.' table.Breakpoints(i).FieldName '[i]);' newline];
  c = [c '    }' newline];

  for j = 1:numel(table.Breakpoints(i).Value) 
    xml = [xml '    <ScalarVariable name="' variableName '.' table.Breakpoints(i).FieldName '[' num2str(j-1) ']" valueReference="' num2str(vr) '" causality="parameter" variability="tunable">' newline];
    xml = [xml '      <Real start="' num2str(table.Breakpoints(i).Value(j)) '"/>' newline];
    xml = [xml '    </ScalarVariable>' newline];
    vr = vr + 1;
  end

end

s.vr  = int32(vr);
s.xml = xml;
s.c   = c;

end


function subs = ind2sub0(dims, idx)
% calculate the zero-based subscripts subs for a zero-based index idx
% of an array with dimensions dims

if idx >= prod(dims)
  error("Zero-based index exceeds array dimensions.")
end

offs = flip(dims);
offs = cumprod(offs);
offs = flip(offs);
offs = [offs(2:end) 1];

subs = zeros(size(dims));

for i = 1:numel(dims)
  subs(i) = mod(floor(idx / offs(i)), dims(i));
end

end
