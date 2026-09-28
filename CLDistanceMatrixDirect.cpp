// build with:
// Sys.setenv(PKG_LIBS = "-lOpenCL"); Rcpp::sourceCpp("CLDistanceMatrixDirect.cpp")

#include <Rcpp.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <thread>

// 🎯 SCHALTET DIE KHRONOS-EXCEPTIONS FREI
#ifndef CL_HPP_ENABLE_EXCEPTIONS
  #define CL_HPP_ENABLE_EXCEPTIONS
#endif

// [[Rcpp::plugins(cpp20)]]
// [[Rcpp::depends(Rcpp)]]

#ifdef __APPLE__
  #define CL_TARGET_OPENCL_VERSION 120
  #define CL_HPP_TARGET_OPENCL_VERSION 120
  #define CL_HPP_MINIMUM_OPENCL_VERSION 120
#else
  #define CL_TARGET_OPENCL_VERSION 300
  #define CL_HPP_TARGET_OPENCL_VERSION 300
  #define CL_HPP_MINIMUM_OPENCL_VERSION 100
#endif

inline std::string get_opencl_c_code() {
  return "\n";
}

#include "src/OpenCL/include/CL/opencl.hpp"
#include "src/opencl.hpp"

using namespace Rcpp;

// [[Rcpp::export]]
NumericMatrix CLDistanceMatrixDirect(const NumericMatrix& mat, int platform_idx = 0, int device_idx = 0) {
  get_opencl_print_enabled() = (std::getenv("R_OPENCL_PRINT_ENABLED") != nullptr);
  cl_int err = 0;

  // 🛡️ DYNAMISCHE HARDWARE-BARRIERE FÜR WINDOWS & UNIX
  static std::vector<int> cached_p_indices;
  static std::vector<int> cached_d_indices;
  static std::vector<cl::Platform*> cached_platforms;
  static std::vector<cl::Device*> cached_devices;
  static std::vector<cl::Context*> cached_contexts;
  static std::vector<cl::CommandQueue*> cached_queues;

  cl::Platform* active_platform = nullptr;
  cl::Device* active_device = nullptr;
  cl::Context* active_context = nullptr;
  cl::CommandQueue* active_queue = nullptr;
  bool found_cached = false;

  for (size_t i = 0; i < cached_p_indices.size(); ++i) {
      if (cached_p_indices[i] == platform_idx && cached_d_indices[i] == device_idx) {
          active_platform = cached_platforms[i];
          active_device = cached_devices[i];
          active_context = cached_contexts[i];
          active_queue = cached_queues[i];
          found_cached = true;
          break;
      }
  }

  if (!found_cached) {
      std::vector<cl::Platform> platforms;
      cl::Platform::get(&platforms);
      if (platform_idx >= (int)platforms.size()) stop("💥 Fehler: Ungueltiger OpenCL Plattform-Index!");
      cl::Platform platform = platforms[platform_idx];

      std::vector<cl::Device> devices;
      platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
      if (device_idx >= (int)devices.size()) stop("💥 Fehler: Ungueltiger OpenCL Device-Index!");
      cl::Device dev = devices[device_idx];

      Rcout << "Initializing hardware link -> Selected device: " << dev.getInfo<CL_DEVICE_NAME>()
            << " on platform: " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";

      active_platform = new cl::Platform(platform);
      active_device = new cl::Device(dev);
      active_context = new cl::Context(*active_device);
      active_queue = new cl::CommandQueue(*active_context, *active_device, 0, &err);
      
      cached_p_indices.push_back(platform_idx);
      cached_d_indices.push_back(device_idx);
      cached_platforms.push_back(active_platform);
      cached_devices.push_back(active_device);
      cached_contexts.push_back(active_context);
      cached_queues.push_back(active_queue);
  }

  // 🎯 INITIALISIERUNG DES WRAPPERS
  Device device((*active_context)(), (*active_device)(), (*active_queue)());
  device.set_kernel_path("distance_matrix.cl");
  device.initialize_binary_cache_path(platform_idx, device_idx);
  
  std::string binary_path = device.get_binary_cache_path();
  if (binary_path.empty()) {
      stop("💥 Fehler: OpenCL-Hardwarepfad konnte in C++ nicht ermittelt werden!");
  }

  std::string target_dir = binary_path.substr(0, binary_path.find_last_of("/\\"));
  Rcpp::Function r_dir_create("dir.create");
  r_dir_create(target_dir, Rcpp::Named("recursive", true), Rcpp::Named("showWarnings", false));

  std::ifstream check_file(binary_path, std::ios::binary);
  bool binary_exists = check_file.good();
  check_file.close();

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

    std::string extensions = active_device->getInfo<CL_DEVICE_EXTENSIONS>();
    device.info.is_fp64_capable = (extensions.find("cl_khr_fp64") != std::string::npos);

    std::string prolog = "";
    if (!device.info.is_fp64_capable) {
        prolog = "#define real_t float\n#define real2_t float2\n";
    } else {
        prolog = "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n#define real_t double\n#define real2_t double2\n";
    }
    checkpoint("1. Wrapper Device-Objekt standalone initialisiert");

    if (binary_exists) {
        device.load_compiled_binary(binary_path);
        checkpoint("4. Vorkompiliertes Binary direkt geladen (JIT übersprungen)");
    } else {
        std::ifstream core_file(device.get_kernel_path());
        if (!core_file.good()) {
            stop("💥 Fehler: Die Kernel-Datei '" + device.get_kernel_path() + "' wurde nicht gefunden!");
        }
        std::stringstream buffer;
        buffer << core_file.rdbuf();
        std::string core_kernel = buffer.str();
        core_file.close();

        std::string final_kernel_code = prolog + "\n" + core_kernel;
        device.set_kernel_source(final_kernel_code);
        checkpoint("3. Kernel-String an device übergeben");

        std::string compile_flags = "-cl-opt-disable";
        
        // 🚀 SYNCHRONER AUFRUF: Durchbricht das Warten am Funktionsende
        device.compile_kernel(compile_flags, false);
        checkpoint("4. JIT-Compiler über Wrapper beendet (compile_kernel)");

        // 💾 EXPORT: Schreibt das verifizierte ELF-Binary
        device.export_compiled_binary(binary_path);
    }
    
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
      distance_kernel.run();
      checkpoint("8b. GPU-Rechenlauf beendet (kernel.2nd run)");

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
