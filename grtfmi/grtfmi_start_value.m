function literals = grtfmi_start_value(values)
% convert an array into a space separated list of literals

literals = '';

for i = 1:numel(values)
  
  value = values(i);
  
  if value == Inf
    literal = 'INF';
  elseif value == -Inf 
    literal = '-INF';
  elseif isnan(value)
    literal = 'NAN';
  else
    literal = num2str(value);
  end
  
  literals = [literals literal];  %#ok AGROW
  
  if i < numel(values)
    literals = [literals ' '];  %#ok AGROW
  end
  
end

end
