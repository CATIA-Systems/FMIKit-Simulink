function guid = makeGUID()
%MAKEGUID Create a Globally Unique Identifier

guid = ['{' char(java.util.UUID.randomUUID) '}'];

end
