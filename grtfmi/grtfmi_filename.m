function filename = grtfmi_filename(path)
% remove the path from a absolute file path

[~, name, ext] = fileparts(path);

filename = [name ext];

end
