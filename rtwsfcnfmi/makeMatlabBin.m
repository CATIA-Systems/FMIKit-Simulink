function Matlab_bin = makeMatlabBin
% MAKEDATEANDODC  Help utility to construct path to Matlab bin 

if strcmp(computer,'PCWIN64')
    Matlab_bin = [strrep(matlabroot,'\','\\') '\\bin\\win64'];
elseif strcmp(computer,'GLNX86')
    Matlab_bin = [matlabroot '/bin/glnx86'];
elseif strcmp(computer,'GLNXA64')
    Matlab_bin = [matlabroot '/bin/glnxa64'];
else
    Matlab_bin = [strrep(matlabroot,'\','\\') '\\bin\\win32'];
end

    