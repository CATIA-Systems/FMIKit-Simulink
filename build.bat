echo build the Java archive
call mvn -f Java/pom.xml install

echo build the generic S-function (sfun_fmurun.mexw32)
mkdir win32
cd win32
cmake -G "Visual Studio 14 2015" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd ..

echo build the generic S-function (sfun_fmurun.mexw64)
mkdir win64
cd win64
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd ..
