function removeBlockResources(block)
% Internal API - do not use

assert(strcmp(get_param(block, 'ReferenceBlock'), 'FMIKit_blocks/FMU'), 'Block is not an FMU')

userData = getUserData(block);

if ~strcmp('sfun_fmurun', userData.functionName)
    % remove S-function source
    sourceFile = which([userData.functionName '.c']);    
    delete(sourceFile);
    
    % remove S-function binary
    mexFunction = which([userData.functionName '.' mexext]);
    clear(mexFunction);
    delete(mexFunction);
end

unzipdir = FMIKit.getUnzipDirectory(block);

% remove the unzip directory
[status, message, ~] = rmdir(unzipdir, 's');

if status == 0
    warning(['Failed to remove unzip directory. ' message]);
end

end
