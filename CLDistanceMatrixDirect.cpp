// build with:
// Sys.setenv(PKG_LIBS = "-lOpenCL"); Rcpp::sourceCpp("CLDistanceMatrixDirect.cpp")
// or on Windows
// Sys.setenv(PKG_CPPFLAGS = "-IC:/Users/alge/Sources/OpenCL-Work/OpenCL-Wrapper/src/OpenCL/include -IC:/Users/alge/Sources/OpenCL-Work/OpenCL-Wrapper")
// Sys.setenv(PKG_LIBS = "-LC:/Users/alge/Sources/OpenCL-Work/OpenCL-Wrapper/src/OpenCL/lib -lOpenCL")
// Rcpp::sourceCpp("CLDistanceMatrixDirect.cpp")

#include <Rcpp.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <string>

// [[Rcpp::plugins(cpp20)]]
// [[Rcpp::depends(Rcpp)]]
// [[Rcpp::eval(options = list(PKG_CPPFLAGS = "-I./src/OpenCL/include -I.", PKG_LIBS = "-LC:/Windows/System32 -lOpenCL"))]]

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 300
#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_MINIMUM_OPENCL_VERSION 100

inline std::string get_opencl_c_code() {
  return "\n";
}

#include "src/OpenCL/include/CL/opencl.hpp"
#include "src/opencl.hpp"

using namespace Rcpp;

// [[Rcpp::export]]
NumericMatrix CLDistanceMatrixDirect(const NumericMatrix& mat) {
  get_opencl_print_enabled() = true;

  // 🚀 DIE DYNAMISCHE SINGLETON-RETTUNG FÜR STANDALONE RUNS:
  // Durch das Schlüsselwort static überleben diese Hardware-Handles im RAM der R-Sitzung.
  // Das verhindert den doppelten Aufruf von clReleaseContext am Funktionsende vollständig!
  static cl::Platform best_platform;
  static cl::Device best_device;
  static cl::Context best_context;
  static cl::CommandQueue best_queue;
  static bool hardware_initialized = false;

  if (!hardware_initialized) {
    std::vector<cl::Platform> platforms;
    cl_int platform_err = cl::Platform::get(&platforms);

    if (platform_err == -1001 || platform_err != CL_SUCCESS || platforms.empty()) {
      stop("*** IMPORTANT: No OpenCL platforms were found! Entering Fallback Mode. ***");
    }

    bool found = false;
    for (const auto& platform : platforms) {
      std::vector<cl::Device> devices;
      try {
        if (platform.getDevices(CL_DEVICE_TYPE_ALL, &devices) != CL_SUCCESS || devices.empty()) {
          continue;
        }
      } catch (const cl::Error &) {
        continue; 
      }

      for (const auto& dev : devices) {
        cl_device_type type = dev.getInfo<CL_DEVICE_TYPE>();
        cl_uint compute_units = dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();

        if (!found || (type == CL_DEVICE_TYPE_GPU && best_device.getInfo<CL_DEVICE_TYPE>() != CL_DEVICE_TYPE_GPU) ||
            (type == best_device.getInfo<CL_DEVICE_TYPE>() && compute_units > best_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>())) {
          best_device = dev;
          best_platform = platform;
          found = true;
        }
      }
    }

    if (!found) {
      stop("*** IMPORTANT: No suitable OpenCL devices found. Entering Fallback Mode. ***");
    }

    Rcout << "Selected device: " << best_device.getInfo<CL_DEVICE_NAME>()
          << " on platform: " << best_platform.getInfo<CL_PLATFORM_NAME>() << "\n";
          
    cl_int err = 0;
    best_context = cl::Context(best_device);
    best_queue = cl::CommandQueue(best_context, best_device, 0, &err);
    hardware_initialized = true;
  }

  int rows = mat.nrow();
  int cols = mat.ncol();
  NumericMatrix outmat(rows, rows);

  try {
    auto t_start = std::chrono::high_resolution_clock::now();
    auto last_t = t_start;

    auto checkpoint = [&](std::string msg) {
      auto now = std::chrono::high_resolution_clock::now();
      double diff_last = std::chrono::duration<double>(now - last_t).count();
      double diff_total = std::chrono::duration<double>(now - t_start).count();
      std::cout << "⏱️ [DIRECT-PROFILE] " << msg
                << " | Schritt: " << diff_last << "s"
                << " | Gesamt: " << diff_total << "s" << std::endl << std::flush;
      last_t = now;
      Rcpp::checkUserInterrupt();
    };

    std::cout << "\n================ START DIRECT DLL INTERFACE PROFILE ================" << std::endl << std::flush;
    checkpoint("0. Start");

    // 🚀 Wir nutzen die langlebigen C-Handles aus den statischen Objekten!
    Device device(best_context(), best_device(), best_queue());

    std::string extensions = best_device.getInfo<CL_DEVICE_EXTENSIONS>();
    device.info.is_fp64_capable = (extensions.find("cl_khr_fp64") != std::string::npos);
    // =========================================================================
    // 🚀 HIER IST DEIN VERMISSTER PROLOG!
    // Wir bauen den Typ-Prolog dynamisch basierend auf der Hardware-Power!
    // =========================================================================
    std::string prolog = "";
    if (!device.info.is_fp64_capable) {
        prolog = "#define real_t float\n#define real2_t float2\n";
    } else {
        prolog = "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n#define real_t double\n#define real2_t double2\n";
    }
    checkpoint("1. Wrapper Device-Objekt standalone initialisiert");

    // 📍 MESSFELD 3: Kernel Quelltext-Zuweisung im RAM
    std::string core_kernel = R"(
            __kernel void distance_matrix(__global real_t* output, __global const real_t* input, const int N, const int DIM) {
                size_t flat_id = get_global_id(0);
                size_t i = flat_id % N;
                size_t j = flat_id / N;
                if (i < N && j < N) {
                    if (i == j) { output[j * N + i] = (real_t)0.0; return; }
                    if (j < i) {
                        real_t tmpRes = (real_t)0.0;
                        for (int k = 0; k < DIM; ++k) {
                            real_t diff = input[i + k * N] - input[j + k * N];
                            tmpRes += diff * diff;
                        }
                        tmpRes = sqrt(tmpRes);
                        output[j * N + i] = tmpRes;
                        output[i * N + j] = tmpRes;
                    }
                }
            }
        )";
    // 🌪️ DIE VERSCHMELZUNG IM RAM: Prolog + Core werden zusammengeklebt!
    std::string final_kernel_code = prolog + "\n" + core_kernel;

    device.set_kernel_source(final_kernel_code);
    checkpoint("3. Kernel-String an device übergeben");

    device.compile_kernel("", false);
    checkpoint("4. JIT-Compiler über Wrapper beendet (compile_kernel)");

    int input_size = rows * cols;
    int output_size = rows * rows;
    ulong total_threads = (ulong)rows * (ulong)rows;

    if (!device.info.is_fp64_capable) {
      std::cout << "🍏 Pfad: FLOAT (Intel Onboard / Legacy / CPU)" << std::endl << std::flush;

      Memory<float> InputF(device, input_size);
      Memory<float> OutputF(device, output_size);
      checkpoint("5a. Float Memory-Objekte auf dem Stack erzeugt");

      double* r_data = (double*)REAL(mat);
      for (int i = 0; i < input_size; ++i) InputF[i] = (float)r_data[i];
      InputF.write_to_device();
      checkpoint("6a. Daten auf die GPU geschrieben (write_to_device)");

      Kernel distance_kernel(device, total_threads, "distance_matrix", OutputF, InputF, rows, cols);
      checkpoint("7a. Kernel-Objekt instanziiert und Argumente verlinkt");

      distance_kernel.run();
      checkpoint("8a. GPU-Rechenlauf beendet (kernel.run)");

      OutputF.read_from_device();
      double* r_res = (double*)REAL(outmat);
      for (int i = 0; i < output_size; ++i) r_res[i] = (double)OutputF[i];
      checkpoint("9a. Daten von GPU zurückgelesen und zurückkonvertiert");

    } else {
      std::cout << "🚀 Pfad: DOUBLE (NVIDIA RTX / Nativ)" << std::endl << std::flush;

      Memory<double> InputD(device, input_size);
      Memory<double> OutputD(device, output_size);
      checkpoint("5b. Double Memory-Objekte auf dem Stack erzeugt");

      std::copy(REAL(mat), REAL(mat) + input_size, InputD.data());
      InputD.write_to_device();
      checkpoint("6b. Daten auf die GPU geschrieben (write_to_device)");

      Kernel distance_kernel(device, total_threads, "distance_matrix", OutputD, InputD, rows, cols);
      checkpoint("7b. Kernel-Objekt instanziiert und Argumente verlinkt");

      distance_kernel.run();
      checkpoint("8b. GPU-Rechenlauf beendet (kernel.run)");

      OutputD.read_from_device();
      std::copy(OutputD.data(), OutputD.data() + output_size, REAL(outmat));
      checkpoint("9b. Daten bytesynchron in R-Speicher kopiert");
    }

    device.finish_queue();
    checkpoint("10. Hardware-Queue final geleert (finish_queue)");

    std::cout << "================= END DIRECT DLL INTERFACE PROFILE =================\n" << std::endl << std::flush;
  }
  catch (cl::Error &err) {
    Rf_error("OpenCL Native Error: %s (%d)", err.what(), err.err());
  }

  return outmat;
}
