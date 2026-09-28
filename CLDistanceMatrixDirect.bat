set PATH=C:\rtools45\usr\bin;C:\rtools45\x86_64-w64-mingw32.static.posix\bin;%PATH%
g++ -O2 ocl_compiler.cpp -o ocl_compiler.exe -I./ -I./src/OpenCL/include -L./src/OpenCL/lib -lOpenCL
