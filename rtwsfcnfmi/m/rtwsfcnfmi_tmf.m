% Copyright (c) 2019 Dassault Systemes. All rights reserved.

function [tmf,envVal] = rtwsfcnfmi_tmf
%RTWSFCNFMI_TMF	Get the template makefile for use with rtwsfcnfmi.tlc
%

[tmf1,envVal] = get_tmf_for_target('rtwsfcn');

tmf = ['rtwsfcnfmi' tmf1(8:end)];

