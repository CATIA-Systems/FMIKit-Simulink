% export sldemo_clutch with grtfmi.tlc

model = 'sldemo_clutch';

% clean up
if exist('slprj', 'dir')
    rmdir('slprj', 's');
end

if exist([model '_grt_fmi_rtw'], 'dir')
    rmdir([model '_grt_fmi_rtw'], 's');
end

if exist([model '.fmu'], 'file')
    delete([model '.fmu']);
end

h = load_system(model);

set_param(h, 'RTWInlineParameters', 'off')
set_param(h, 'GenerateReport', 'off');
set_param(h, 'Solver', 'ode1');

cs = getActiveConfigSet(h);
switchTarget(cs, 'grtfmi.tlc', []);

rtwbuild(h);

close_system(h, 0);

assert(exist([model '.fmu'], 'file') == 2);

if strcmp(computer('arch'), 'win64')
    status = system(['fmpy simulate ' model '.fmu']);
    assert(status == 0);
else
    status = system(['fmpy info ' model '.fmu']);    
    assert(status == 0);
end
