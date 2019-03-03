function XMLdate = makeDateAndDoc(modelName, exportImage, copyModel, LoadBinaryMEX)
% MAKEDATEANDODC  Help utility to construct date for XML and to
%                 generate documentation files in resources 
%
%  Author:  Dan Henriksson, Dassault Systemes AB
%  Version: 2.3.0
%  Date:    March 31, 2016
%
%  Copyright 2016 Dassault Systemes
%  All rights reserved.

res_dir = [modelName '_sfcn_rtw_fmi/resources/'];
if exist(res_dir, 'dir') == 7
    rmdir(res_dir, 's');
end
if (exportImage || copyModel)
    mkdir([modelName '_sfcn_rtw_fmi/resources/SimulinkModel']);
end 
if exportImage
    h=get_param(modelName,'Handle');
    orient(modelName, 'Portrait');
    saveas(h,[modelName '.png']);
    movefile([modelName '.png'], [modelName '_sfcn_rtw_fmi/resources/SimulinkModel'], 'f');
end
if copyModel
    copyfile(which(modelName), [modelName '_sfcn_rtw_fmi/resources/SimulinkModel'], 'f');
end
if (LoadBinaryMEX)
    sfncs=find_system(modelName,'LookUnderMasks','all','FollowLinks','on','BlockType','S-Function');
    if ~isempty(sfncs)
        mkdir([modelName '_sfcn_rtw_fmi/resources/SFunctions']);
        for si=1:size(sfncs,1)
            funcName = get_param(sfncs{si},'FunctionName');
            mexName = funcName;
            if (exist(funcName,'file')~=3) 
                % Not a MEX file on path, try to evaluate expression
                mexName = evalin('base', funcName);
            end
            copyfile(which(mexName), [modelName '_sfcn_rtw_fmi/resources/SFunctions'], 'f');
        end
    end
end

dtFact   = javax.xml.datatype.DatatypeFactory.newInstance();
calendar = java.util.GregorianCalendar;
date     = dtFact.newXMLGregorianCalendar(calendar).normalize();
date.setFractionalSecond([]);

XMLdate  = char(date);

    