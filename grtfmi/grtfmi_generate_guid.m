function guid = grtfmi_generate_guid()
%GRTFMI_GENERATE_GUID Create a Globally Unique Identifier

guid = ['{' char(java.util.UUID.randomUUID) '}'];

end
