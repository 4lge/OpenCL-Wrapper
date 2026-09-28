#!/bin/sh
if test "$(uname)" = "Darwin" ; then
  g++ -O2 ocl_compiler.cpp -o ocl_compiler.mac -I./ -I./src/OpenCL/include -L./src/OpenCL/lib -framework OpenCL
elif test "$(uname)" = "Linux" ; then
  g++ -O2 ocl_compiler.cpp -o ocl_compiler.linux -I./ -I./src/OpenCL/include -L./src/OpenCL/lib -lOpenCL
fi
