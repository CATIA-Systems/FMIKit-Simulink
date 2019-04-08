function [tmf,envVal] = rtwsfcnfmi_tmf
%RTWSFCNFMI_TMF	Get the template makefile for use with rtwsfcnfmi.tlc
%

[tmf1,envVal] = get_tmf_for_target('rtwsfcn');

if (verLessThan('matlab','9.5'))
   tmf = ['rtwsfcnfmi' tmf1(8:end)];
else
    tmf = ['rtwsfcnfmi_18b' tmf1(8:end)];
end
