#!/bin/sh
if [ "$(uname)" == "Darwin" ]; then
  g++ -O2 ocl_compiler.cpp -o ocl_compiler.mac -I./ -I./src/OpenCL/include -L./src/OpenCL/lib -framework OpenCL
elif [ "$(uname)" == "Linux" ]; then
  g++ -O2 ocl_compiler.cpp -o ocl_compiler.linux -I./ -I./src/OpenCL/include -L./src/OpenCL/lib -lframework OpenCL
fi
