function grtfmi_install_cmake()
% Download and extract CMake into the FMI Kit folder

info = what('+FMIKit');
[fmikit_folder, ~, ~] = fileparts(info(1).path);

listing = dir(fullfile(fmikit_folder, 'cmake-*'));

if ~isempty(listing)
    error(['An existing CMake directory was found. ' ... 
      'To reinstall CMake delete ' fullfile(listing(1).folder, listing(1).name) ' and try again.'])
end

if ispc
  filename = 'cmake-3.22.2-windows-x86_64.zip';
elseif ismac
  filename = 'cmake-3.22.2-macos-universal.tar.gz';
else
  filename = 'cmake-3.22.2-linux-x86_64.tar.gz';
end

url = ['https://github.com/Kitware/CMake/releases/download/v3.22.2/' filename]; 

disp(['Donwloading ' url])
archive = websave(filename, url);

disp(['Extracting archive to ' fmikit_folder])

if ispc
  unzip(archive, fmikit_folder);
else
  untar(archive, fmikit_folder);
end

delete(archive);

end
