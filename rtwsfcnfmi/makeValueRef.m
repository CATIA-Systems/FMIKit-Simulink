function valueRef = makeValueRef(category_str, datatype_str, index)
% MAKEVALUEREF  Help utility called by sfcnmdi.tlc to construct value reference for FMI
%
%  category: bits 31-28  datatype: bits 27-24  index: bits 23-0

% Check range of index
if (index < 0 || index > 2^24-1)
    valueRef = -1;
    return;
end

categories = {'parameter', 'state', 'derivative', 'output', 'input', 'blockio', 'dwork'};
datatypes   = {'real_T', 'real32_T', 'int8_T', 'uint8_T', 'int16_T', 'uint16_T', 'int32_T', 'uint32_T', 'boolean_T'};

% Check category
category = 0;
for c=1:length(categories)
    if strcmpi(categories{c},category_str)==1
        category = c;
        break;
    end
end
% Check datatype
datatype = 1; % real_T as fallback
for d=1:length(datatypes)
    if strcmp(datatypes{d},datatype_str)==1
        datatype = d;
        break;
    end
end
datatype = datatype - 1; % BuiltInDTypeId enumeration starts at 0

valueRef = num2str(uint32(category*2^28 + datatype*2^24 + index));
