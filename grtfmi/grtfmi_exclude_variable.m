function exclude = grtfmi_exclude_variable( variable_name, exposed_variables )

if isempty(exposed_variables)
    exclude = false;
    return
end

names = regexp(exposed_variables, '\s+', 'split');

exclude = ~any(strcmp(variable_name, names));

end
