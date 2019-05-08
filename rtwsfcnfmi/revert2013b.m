function res = revert2013b
%REVERT2013B	Help utility to revert code generation
%		for inline parameters OFF to R2013b

rel = version('-release');

res = 0;
if (strcmpi(rel,'2014a') || strcmpi(rel,'2014b') || strcmpi(rel,'2015a'))
    evalin('base', 'revertInlineParametersOffToR2013b');
    res = 1;
end
