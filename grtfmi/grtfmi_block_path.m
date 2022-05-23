function variable_name = grtfmi_block_path(p)

segments = {};

i = 1;
j = 1;

while j <= numel(p)
  if p(j) == '/'
    if p(j+1) == '/'
      j = j + 2;
      continue
    end
    segments{end+1} = p(i+1:j-1); %#ok<AGROW>
    i = j;
  elseif j == numel(p)
    segments{end+1} = p(i+1:j); %#ok<AGROW>
  end
  j = j + 1;
end

variable_name = '';

for i = 2:numel(segments)
  
  segment = segments{i};
  
  % add apostrophes if the segment contains spaces
  if regexp(segment, '\s')
    segment = ['''' segment '''']; %#ok<AGROW>
  end
  
  if isempty(variable_name)
    variable_name = segment;
  else
    variable_name = [variable_name '.' segment]; %#ok<AGROW>
  end
  
end

end