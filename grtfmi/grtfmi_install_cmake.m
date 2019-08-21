function grtfmi_install_cmake()
% Download and install CMake into the FMI Kit folder

url = 'https://github.com/Kitware/CMake/releases/download/v3.15.2/cmake-3.15.2-win64-x64.zip';

disp(['Donwloading ' url])
archive = websave('cmake-3.15.2-win64-x64.zip', url);

info = what('+FMIKit');
[fmikit_folder, ~, ~] = fileparts(info(1).path);

disp(['Extracting archive to ' fmikit_folder])
unzip(archive, fmikit_folder);

delete(archive);

end
