function sources = grtfmi_simscape_sources()
% return a list of the Simscape runtime source files

arch = computer('arch');

paths = {
  {'common', 'external', 'library'}
  {'common', 'math', 'core'}
  {'network_engine'}
  {'common', 'foundation', 'core'}
  {'simscape', 'engine', 'core'}
  {'simscape', 'engine', 'sli'}
};

sources = {};

for i=1:numel(paths)
  path = paths{i};
  listing  = dir(fullfile(matlabroot, 'toolbox', 'physmod', path{:}, 'c', arch, '*.c'));
  sources = [sources {listing(:).name}]; %#okargrow
end

end
