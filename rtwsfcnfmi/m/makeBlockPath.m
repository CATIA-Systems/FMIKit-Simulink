function blockPath = makeBlockPath(varPath)
%MAKEBLOCKPATH	Help utility to construct valid hierarchical path
%               with dot notation according to FMI specification

% Replace original '/' in block names
varPath = strrep(varPath, '//', '_');
% Replace original '.' in block names
varPath = strrep(varPath, '.', '_');
% Remove model name and replace path separators with '.'
idx = strfind(varPath,'/');
varPath = varPath(idx(1)+1:end);
varPath = strrep(varPath, '/', '.');
% Replace unsupported characters, special case with 
% numbers not supported as first character.
blockPath = [];
pathIdx = 0;
lastChar = '.';
for ch=1:length(varPath)
    pathIdx = pathIdx + 1;
    currChar = varPath(ch);
    if (currChar~='.')
        isLower  = ( currChar>='a' && currChar<='z' );
        isUpper  = ( currChar>='A' && currChar<='Z' );
        isDigit  = ( currChar>='0' && currChar<='9' );
        if ~(isLower || isUpper || isDigit)
            blockPath(pathIdx) = '_';
        elseif (isDigit && lastChar=='.')
            % Add leading underscore if block starts with digit
            blockPath(pathIdx) = '_';
            pathIdx = pathIdx + 1;
            blockPath(pathIdx) = currChar;
        else
            blockPath(pathIdx) = currChar;
        end
    else
        blockPath(pathIdx) = '.';
    end
    lastChar = currChar;
end

blockPath = char(blockPath);

end