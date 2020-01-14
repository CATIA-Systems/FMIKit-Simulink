function test_grtfmi

for fmi_version = [2 3]
  
  fmi_version = num2str(fmi_version);

  build_dir = ['fmi' fmi_version];
  
  if exist(build_dir, 'dir')
    rmdir(build_dir, 's');
  end
  
  mkdir(build_dir);
  
  cd(build_dir);
  
  build_model('f14', fmi_version);

  build_model('sldemo_clutch', fmi_version);

  build_model('sldemo_fuelsys', fmi_version);

  ref_model = fullfile(pwd, 'sldemo_mdlref_counter_bus.slx');
  if exist(ref_model, 'file')
      delete(ref_model);
  end

  h = load_system('sldemo_mdlref_counter_bus');
  cs = getActiveConfigSet(h);
  switchTarget(cs, 'grtfmi.tlc', []);
  set_param(h, 'FMIVersion', fmi_version);
  set_param(h, 'CMakeGenerator', 'Visual Studio 14 2015 Win64');
  save_system(h, 'sldemo_mdlref_counter_bus');

  build_model('sldemo_mdlref_bus', fmi_version);

  cd('..');
  
end
  
end


function build_model(model, fmi_version)

rwt_dir = fullfile(pwd, [model '_grt_fmi_rtw']);
if exist(rwt_dir, 'dir')
    rmdir(rwt_dir, 's');
end

slprj = fullfile(pwd, 'slprj');
if exist(slprj, 'dir')
    rmdir(slprj, 's');
end

fmu = fullfile(pwd, [model '.fmu']);
if exist(fmu, 'file')
    delete(fmu);
end

h = load_system(model);
cs = getActiveConfigSet(h);
switchTarget(cs, 'grtfmi.tlc', []);

params = get_param(h, 'ObjectParameters');

if isfield(params, 'DefaultParameterBehavior')
    set_param(h, 'DefaultParameterBehavior', 'Tunable');
end

if isfield(params, 'RTWInlineParameters')
    set_param(h, 'RTWInlineParameters', 'off');
end

set_param(h, 'FMIVersion', fmi_version);
set_param(h, 'CMakeGenerator', 'Visual Studio 14 2015 Win64')
set_param(h, 'GenerateReport', 'off');
set_param(h, 'SignalLogging', 'off');
set_param(h, 'Solver', 'ode3');

rtwbuild(h);

close_system(h, 0);

assert(exist([model '.fmu'], 'file') == 2);

end
