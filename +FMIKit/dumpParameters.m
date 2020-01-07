function dumpParameters(block)

%block = gcb;

op = get_param(block, 'ObjectParameters');

names = fieldnames(op);

for i = 1:numel(names)
  try
    name = names{i};
    p = get_param(block, name);
    s = matlab.unittest.diagnostics.ConstraintDiagnostic.getDisplayableString(p);
    disp([name repmat(' ', 1, 30 - numel(name)) strtrim(s)])
  end
end

end
