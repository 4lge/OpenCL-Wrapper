#include "opencl.hpp"

int main() {
  //Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
  Device device(select_device_with_most_memory()); // compile OpenCL C code for the fastest available device
  //Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
  
// vector<string> kernel_files = find_files("kernels",".cl");
//
// for (vector<string>::iterator t=kernel_files.begin(); t!=kernel_files.end(); ++t){
//   print_message("kernel file found: " + *t);
// }
//
//
// string add_kernel_file;
// for (vector<string>::iterator t=kernel_files.begin(); t!=kernel_files.end(); ++t){
//   if(equals_regex(*t,".*/add.cl")){
//     add_kernel_file=*t;
//     break;
//   }
// }
//
// print_message("kernel file used: >>" + add_kernel_file + "<<", "Info:");
//
// 
// device.set_kernel_code(read_file(add_kernel_file));
  device.load_kernel("kernels","add.cl");
  device.compile_kernel();

  string code = device.get_c_code()+device.get_kernel_code();
  std::cout << "CL C code\n" << code << std::endl;

  
  const uint N = 10u; //1024u; // size of vectors
  Memory<float> A(device, N); // allocate memory on both host and device
  Memory<float> B(device, N);
  Memory<float> C(device, N);

  Kernel add_kernel(device, N, "add_kernel", A, B, C); // kernel that runs on the device

  for(uint n=0u; n<N; n++) {
    A[n] = 3.0f; // initialize memory
    B[n] = 2.0f;
    C[n] = 1.0f;
  }

  print_info("Value before kernel execution: C[0] = "+to_string(C[0]));

  A.write_to_device(); // copy data from host memory to device memory
  B.write_to_device();
  add_kernel.run(); // run add_kernel on the device
  C.read_from_device(); // copy data from device memory to host memory

  print_info("Value after kernel execution: C[0] = "+to_string(C[0]));

  device.load_kernel("kernels","runif.cl");
#ifdef _WIN32
  device.compile_kernel("-cl-opt-disable"); // Schaltet den Optimierer unter Windows aus
#else
  device.compile_kernel();
#endif  

  code = device.get_c_code()+device.get_kernel_code();
  std::cout << "runif CL C code\n" << code << std::endl;
  

  Memory<float> OutputF;(device, N);
  Memory<double> OutputD; (device, N);
  Memory<int> Seed(device, 1);

  Seed[0]=42;

  double lower = -1.0;
  double upper = 1.0;

  Kernel unif_rng;
    // kernel that runs on the device
    if(device.info.is_fp64_capable){ // TODO: use float via parameter also on double device via argument.
      OutputD = Memory<double>(device, N);
      unif_rng = Kernel(device, N, "unif_rng", OutputD, Seed, lower, upper);
    } else {
      OutputF = Memory<float>(device, N);
      // this does not (yet) work!?
      // norm_rng = Kernel(device, N, "norm_rng", Memory<float>(Output), Seed, (float)mean, (float)sd);
      unif_rng = Kernel(device, N, "unif_rng", OutputF, Seed, (float)lower, (float)upper);
    }
  
  Seed.write_to_device(); // copy data from host memory to device memory

      // run add_kernel on the device
    unif_rng.run();
    
  std::cout << "r_unif <- c(";;
    if(device.info.is_fp64_capable){ 
      OutputD.read_from_device(); 
      for(auto i=0; i<OutputD.length(); i++){
        std::cout << (double)OutputD[i];
        if(i<OutputF.length()-1)
        std::cout << ", ";
      }
    } else {
      OutputF.read_from_device();
      for(auto i=0; i<OutputF.length(); i++){
        std::cout << (double)OutputF[i];
        if(i<OutputF.length()-1)
        std::cout << ", ";
      }
    }
    std::cout << ")" << std::endl;


    
  device.load_kernel("kernels","rnorm.cl");
#ifdef _WIN32
  device.compile_kernel("-cl-opt-disable"); // Schaltet den unendlichen Optimierer unter Windows aus
#else
  device.compile_kernel();
#endif
  
  double mean = 0.0;
  double sd = 1.0;

      Kernel norm_rng;
    // kernel that runs on the device
    if(device.info.is_fp64_capable){ // TODO: use float via parameter also on double device via argument.
      OutputD = Memory<double>(device, N);
      norm_rng = Kernel(device, N, "norm_rng", OutputD, Seed, mean, sd);
    } else {
      OutputF = Memory<float>(device, N);
      // this does not (yet) work!?
      // norm_rng = Kernel(device, N, "norm_rng", Memory<float>(Output), Seed, (float)mean, (float)sd);
      norm_rng = Kernel(device, N, "norm_rng", OutputF, Seed, (float)mean, (float)sd);
    }
    Seed.write_to_device(); // copy data from host memory to device memory
    
    // run add_kernel on the device
    norm_rng.run();
    
    // copy data from device memory to host memory
  
    std::cout << "r_norm <- c(";;
    if(device.info.is_fp64_capable){ 
      OutputD.read_from_device(); 
      for(auto i=0; i<OutputD.length(); i++){
        std::cout << (double)OutputD[i];
        if(i<OutputF.length()-1)
        std::cout << ", ";
      }
    } else {
      OutputF.read_from_device();
      for(auto i=0; i<OutputF.length(); i++){
        std::cout << (double)OutputF[i];
        if(i<OutputF.length()-1)
        std::cout << ", ";
      }
    }
    std::cout << ")" << std::endl;

  std::cout << "example runs finished." << std::endl; 

  wait();
  return 0;
}
