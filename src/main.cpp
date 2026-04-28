#include "opencl.hpp"

int main() {
  Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
  
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

  
  const uint N = 1024u; // size of vectors
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
  device.compile_kernel();

  code = device.get_c_code()+device.get_kernel_code();
  std::cout << "runif CL C code\n" << code << std::endl;
  

  Memory<float> real_output(device, N);
  Memory<int> seed(device, 1);
        
  Memory<float> lower(device, 1);
  Memory<float> upper(device, 1);

  lower[0] = 0.5;
  upper[0] = 1.5;

  Kernel unif_rng(device, N, "unif_rng", real_output, seed, lower, upper); // kernel that runs on the device
        
  seed.write_to_device(); // copy data from host memory to device memory
  lower.write_to_device(); // copy data from host memory to device memory
  upper.write_to_device(); // copy data from host memory to device memory
        
  unif_rng.run(); // run add_kernel on the device

  real_output.read_from_device(); // copy data from device memory to host memory

  std::cout << "r_unif <- c(";;
  for(auto i=0; i<real_output.length(); i++){
    std::cout << real_output[i];
    if(i<real_output.length()-1)
      std::cout << ", ";
  }
  std::cout << ")" << std::endl;

    
  device.load_kernel("kernels","rnorm.cl");
  device.compile_kernel();

  code = device.get_c_code()+device.get_kernel_code();
  std::cout << "rnorm CL C code\n" << code << std::endl;
  

  Memory<float> mean(device, 1);
  Memory<float> sd(device, 1);

  mean[0] = 42;
  sd[0] = 5;

  Kernel norm_rng(device, N, "norm_rng", real_output, seed, mean, sd); // kernel that runs on the device
        
  seed.write_to_device(); // copy data from host memory to device memory
  mean.write_to_device(); // copy data from host memory to device memory
  sd.write_to_device(); // copy data from host memory to device memory
        
  norm_rng.run(); // run add_kernel on the device

  real_output.read_from_device(); // copy data from device memory to host memory

  std::cout << "r_norm <- c(";;
  for(auto i=0; i<real_output.length(); i++){
    std::cout << real_output[i];
    if(i<real_output.length()-1)
      std::cout << ", ";
  }
  std::cout << ")" << std::endl;

    
  wait();
  return 0;
}
