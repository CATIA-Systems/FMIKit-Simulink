function escaped = grtfmi_xml_encode(string)
% escape non-ASCII characters
  
string = strrep(string, '//', '/');
string = strrep(string, '&', '&amp;');
string = strrep(string, '<', '&lt;');
string = strrep(string, '>', '&gt;');
string = strrep(string, '"', '&quot;');
string = strrep(string, char(hex2dec('D')), ' '); % carriage return
string = strrep(string, char(hex2dec('A')), ' '); % line feed
string = strrep(string, char(hex2dec('9')), ' '); % tab

nonascii = string(string > 127);
for j = 1:numel(nonascii)
  c = nonascii(j);
  string = strrep(string, c, ['&#x' dec2hex(c) ';']); 
end

escaped = string;

end
