rem build the Java archive
call mvn -f Java/pom.xml install

rem build the generic S-function
mkdir build
cd build
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd ..

rem build the MATLAB App installer
matlab -nodisplay -nosplash -wait -r "matlab.apputil.package('FMIKit.prj'); exit()"
