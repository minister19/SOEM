rmdir build /s
mkdir build
cd build
cmake .. -G "NMake Makefiles"
nmake
cd ..