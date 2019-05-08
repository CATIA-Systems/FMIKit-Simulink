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
  
  segment = strrep(segment, '//', '/');
  segment = strrep(segment, '&', '&amp;');
  segment = strrep(segment, '<', '&lt;');
  segment = strrep(segment, '>', '&gt;');
  segment = strrep(segment, char(hex2dec('D')), ' '); % carriage return
  segment = strrep(segment, char(hex2dec('A')), ' '); % line feed
  segment = strrep(segment, char(hex2dec('9')), ' '); % tab
  
  % escape non-ASCII characters
  nonascii = segment(segment > 127);
  for j = 1:numel(nonascii)
    c = nonascii(j);
    segment = strrep(segment, c, ['&#x' dec2hex(c) ';']); 
  end
  
  % add apostrophes if the segment contains spaces
  if regexp(segment, '\s')
    segment = ['&#39;' segment '&#39;']; %#ok<AGROW>
  end
  
  if isempty(variable_name)
    variable_name = segment;
  else
    variable_name = [variable_name '.' segment]; %#ok<AGROW>
  end
  
end

end