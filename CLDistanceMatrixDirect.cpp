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

// 🎯 SCHALTET DIE KHRONOS-EXCEPTIONS RECHTSKONFORM FREI
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

  // 🚀 HARDWARE-AUFLÖSUNG DIREKT ÜBER DIE INTERNEN KOORDINATEN
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);
  if (platform_idx >= (int)platforms.size()) stop("💥 Fehler: Ungueltiger OpenCL Plattform-Index!");
  cl::Platform platform = platforms[platform_idx];

  std::vector<cl::Device> devices;
  platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
  if (device_idx >= (int)devices.size()) stop("💥 Fehler: Ungueltiger OpenCL Device-Index!");
  cl::Device dev = devices[device_idx];

  // 🛡️ DIE ASYMMETRISCHE SPEICHER-WEICHE (Rettet Windows & Linux gleichzeitig!)
#ifndef _WIN32
  // 🐧 LINUX / MACOS: Brauchen zwingend statisch langlebige Vektor-Pointer gegen den Segfault!
  static std::vector<cl::Context*> cached_contexts;
  static std::vector<cl::CommandQueue*> cached_queues;
  static std::vector<cl_device_id> cached_device_ids;

  cl::Context* active_context = nullptr;
  cl::CommandQueue* active_queue = nullptr;
  bool found_cached = false;

  for (size_t i = 0; i < cached_device_ids.size(); ++i) {
      if (cached_device_ids[i] == dev()) {
          active_context = cached_contexts[i];
          active_queue = cached_queues[i];
          found_cached = true;
          break;
      }
  }

  if (!found_cached) {
      Rcout << "Initializing Linux/Mac static hardware link -> " << dev.getInfo<CL_DEVICE_NAME>() << "\n";
      active_context = new cl::Context(dev);
      active_queue = new cl::CommandQueue(*active_context, dev, 0, &err);
      
      cached_device_ids.push_back(dev());
      cached_contexts.push_back(active_context);
      cached_queues.push_back(active_queue);
  }
#else
  // 🪟 WINDOWS: Erzeugt bei JEDEM Aufruf einen unbefleckten Kontext. Das hebelt den Intel-Cache-Lock aus!
  cl::Context current_context(dev);
  cl::CommandQueue current_queue(current_context, dev, 0, &err);
#endif

  // 🎯 INSTANZIIRUNG DES WRAPPERS MIT DEN PLATTFORMSPEZIFISCHEN HANDLES
#ifdef _WIN32
  Device device(current_context(), dev(), current_queue());
#else
  Device device((*active_context)(), dev(), (*active_queue)());
#endif

  
  // 🎯 DAS NEUE GEKAPSELTE INITIALISIERUNGS-MUSTER:
  // Setzt die Quelldatei, berechnet die Ordner und regelt den Kalt-/Warmstart vollkommen autonom!
  device.set_kernel_path("distance_matrix.cl");
  device.initialize_binary_cache_path(platform_idx, device_idx);
  
  // Ordnerstruktur für den Cache anlegen (Muss vor dem Compiler-Lauf existieren)
  std::string binary_path = device.get_binary_cache_path();
  std::string target_dir = binary_path.substr(0, binary_path.find_last_of("/\\"));
  Rcpp::Function r_dir_create("dir.create");
  r_dir_create(target_dir, Rcpp::Named("recursive", true), Rcpp::Named("showWarnings", false));

  // 🚀 EIN EINZIGER BEFEHL: Erledigt alles im Backend sychron und prozess-isoliert!
  device.load_or_build_kernel(platform_idx, device_idx);

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

    // 🚀 HARDWARE-EIGENSCHAFTEN AUSLESEN: Wichtig für die folgende Datenpfad-Weiche (Float vs Double)
    std::string extensions = dev.getInfo<CL_DEVICE_EXTENSIONS>();
    device.info.is_fp64_capable = (extensions.find("cl_khr_fp64") != std::string::npos);
    checkpoint("1. Hardware-Faehigkeiten verifiziert");

    // 🎯 DER NEUE REINE WRAPPER-DURCHREICHER:
    // Der gesamte JIT-Compile-Zweig, das Einlesen der .cl-Datei und das Exportieren
    // sind vollständig in diese eine Methode gewandert!
    device.load_or_build_kernel(platform_idx, device_idx);
    checkpoint("2. OpenCL-Programm erfolgreich geladen (JIT übersprungen oder über CLI-Compiler erzeugt)");

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
