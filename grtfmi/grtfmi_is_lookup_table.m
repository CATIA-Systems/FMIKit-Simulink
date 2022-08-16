function is_lookup_table = grtfmi_is_lookup_table(record)
% check if the record belongs to a Simulink.LookupTable

is_lookup_table = false;

if isfield(record, 'WorkspaceVarName')
  
  v = evalin('base', record.WorkspaceVarName);
  
  is_lookup_table = isa(v, 'Simulink.LookupTable');

  end

end
