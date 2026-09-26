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

#define CL_HPP_ENABLE_EXCEPTIONS
// =========================================================================
// 🍏 MACOS OPENCL 1.2 ABWÄRTSKOMPATIBILITÄT:
// Apple friert OpenCL bei Version 1.2 ein. Wir zwingen das Khronos-Header,
// alle 2.0+ Features (SVM, Pipes, On-Device Queues) wegzulassen.
// =========================================================================
#ifdef __APPLE__
  #define CL_TARGET_OPENCL_VERSION 120
  #define CL_HPP_TARGET_OPENCL_VERSION 120
  #define CL_HPP_MINIMUM_OPENCL_VERSION 120
#else
  #define CL_TARGET_OPENCL_VERSION 300
  #define CL_HPP_TARGET_OPENCL_VERSION 300
  #define CL_HPP_MINIMUM_OPENCL_VERSION 100
#endif

std::string CLGetHardwareCachePath();

inline std::string get_opencl_c_code() {
  return "\n";
}

#include "src/OpenCL/include/CL/opencl.hpp"
#include "src/opencl.hpp"

using namespace Rcpp;

// [[Rcpp::export]]
NumericMatrix CLDistanceMatrixDirect(const NumericMatrix& mat) {
  get_opencl_print_enabled() = (std::getenv("R_OPENCL_PRINT_ENABLED") != nullptr);

  cl_int err = 0;

  // 🚀 DIE DYNAMISCHE SINGLETON-RETTUNG FÜR STANDALONE RUNS:
  // Durch das Schlüsselwort static überleben diese Hardware-Handles im RAM der R-Sitzung.
  // Das verhindert den doppelten Aufruf von clReleaseContext am Funktionsende vollständig!
  static cl::Platform* best_platform = nullptr;
  static cl::Device* best_device = nullptr;
  // 🚀 Statische Pointer für Linux/Mac (Müssen dort langlebig sein)
#ifndef _WIN32
  static cl::Context* best_context = nullptr;
  static cl::CommandQueue* best_queue = nullptr;
#endif

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

        // 🎯 Saubere found-Logik für Pointer:
        if (!found || 
            (type == CL_DEVICE_TYPE_GPU && best_device->getInfo<CL_DEVICE_TYPE>() != CL_DEVICE_TYPE_GPU) ||
            (type == best_device->getInfo<CL_DEVICE_TYPE>() && compute_units > best_device->getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>())) {
          
          // Alte Heap-Objekte löschen, falls schon welche existierten
          if (best_device) delete best_device;
          if (best_platform) delete best_platform;

          best_device = new cl::Device(dev);
          best_platform = new cl::Platform(platform);
 
          found = true;
        }
      }
    }

    if (!found) {
      stop("*** IMPORTANT: No suitable OpenCL devices found. Entering Fallback Mode. ***");
    }

    // Zugriff über den Pfeil-Operator ->
    Rcout << "Selected device: " << best_device->getInfo<CL_DEVICE_NAME>()
          << " on platform: " << best_platform->getInfo<CL_PLATFORM_NAME>() << "\n";
          

    // 💥 FIX: Konstruktoren nutzen jetzt die dereferenzierten Heap-Objekte (*best_device)
    //    best_context = new cl::Context(*best_device);
    //best_queue = new cl::CommandQueue(*best_context, *best_device, 0, &err);
    // Linux/Mac initialisiert die langlebigen Pointer genau einmal hier drin
#ifndef _WIN32
    best_context = new cl::Context(*best_device);
    best_queue = new cl::CommandQueue(*best_context, *best_device, 0, &err);
#endif
    hardware_initialized = true;
  }

  // 🎯 WINDOWS-SPEZIFISCH: Frisch allokieren bei jedem Aufruf gegen den Intel-Lock
#ifdef _WIN32
  cl::Context current_context(*best_device);
  cl::CommandQueue current_queue(current_context, *best_device, 0, &err);
#endif
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
    // 🚀 FIX: Klammern um den Stern und den Variablennamen setzen!
#ifdef _WIN32
    Device device(current_context(), (*best_device)(), current_queue());
#else
    Device device((*best_context)(), (*best_device)(), (*best_queue)());
#endif

    std::string extensions = best_device->getInfo<CL_DEVICE_EXTENSIONS>();
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
    // 🌪️ DIE VERSCHMELZUNG IM RAM: Prolog und Core MÜSSEN wieder zusammengeklebt werden!
    std::string final_kernel_code = prolog + "\n" + core_kernel;

    // Wir übergeben das vollständige Paket an das Device
    device.set_kernel_source(final_kernel_code);
    checkpoint("3. Kernel-String an device übergeben");

    // 🎯 DER ULTIMATIVE PLATTFORM- & HARDWARE-SPECIFIC CACHE-PATH
    std::string os_label = "unknown";
    std::string arch_label = "x86_64";
    
#if defined(__x86_64__) || defined(_M_X64)
    arch_label = "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
    arch_label = "arm64";
#endif

#if defined(_WIN32)
    os_label = "windows_" + arch_label;
#elif defined(__APPLE__)
    os_label = "macos_" + arch_label;
#elif defined(__linux__)
    os_label = "linux_" + arch_label;
#endif

    std::string platform_name = best_platform->getInfo<CL_PLATFORM_NAME>();
    std::string device_name = best_device->getInfo<CL_DEVICE_NAME>();
    
    // Lambda zum Bereinigen von Leer- und Sonderzeichen für Windows/Linux/Mac-Ordnerpfade
    auto clean_str = [](std::string s) {
        std::string res = "";
        for (char c : s) {
            if (std::isalnum(c)) res += std::tolower(c);
            else if (c == ' ' || c == '-' || c == '_') res += '_';
        }
        return res;
    };
    ////////
    // Baut exakt: .cl_cache/macos_arm64/apple/apple_m3... oder .cl_cache/windows_x86_64/...
        // 🎯 Der plattform- und hardware-spezifische Pfad wird berechnet:
    std::string target_dir = "./.cl_cache/" + os_label + "/" + clean_str(platform_name) + "/" + clean_str(device_name);
    
    Rcpp::Function r_dir_create("dir.create");
    r_dir_create(target_dir, Rcpp::Named("recursive", true), Rcpp::Named("showWarnings", false));

    std::string binary_path = CLGetHardwareCachePath();

    // 🚀 KORREKTUR: Die Rückmeldung MUSS vor der if-Bedingung stehen!
    // Damit weiß R immer sofort, wo das File liegt.
    Rcpp::Function r_sys_setenv("Sys.setenv");
    r_sys_setenv(Rcpp::Named("R_OPENCL_GENERATED_PATH", binary_path));

    // Erst JETZT prüfen wir, ob die Datei da ist
    std::ifstream check_file(binary_path, std::ios::binary);
    bool binary_exists = check_file.good();
    check_file.close();

    if (binary_exists) {
        // 🚀 HIGH-SPEED: Lade fertiges Binary
        device.load_compiled_binary(binary_path);
        checkpoint("4. Vorkompiliertes Binary direkt geladen (JIT übersprungen)");
    } else {
        // 🛠️ BUILD-PFAD: Kompiliert regulär und exportiert danach
        std::string compile_flags = "-cl-opt-disable";
        device.compile_kernel(compile_flags, false);
        checkpoint("4. JIT-Compiler über Wrapper beendet (compile_kernel)");

        // 💾 DER KORREKTE BINÄR-EXPORT (Greift tief in das Khronos-Vektor-Layout):
        // 💾 DER PLATTFORMÜBERGREIFENDE BYTESYNCHRONE BINÄR-EXPORT
        auto bin_data = device.get_cl_program().getInfo<CL_PROGRAM_BINARIES>();
        if (!bin_data.empty() && bin_data[0].size() > 0) {
            std::ofstream out(binary_path, std::ios::binary | std::ios::out);
            
            // 🎯 Vektor-Index [0] holt den inneren Byte-Vektor der CPU.
            // .data() liefert den unsigned char* Pointer.
            // Das Casting auf (const char*) ist rein fuer den Funktionskopf von write(), 
            // da der Stream im Binärmodus geöffnet ist, bleiben die Bytes zu 100% unverändert!
            out.write(reinterpret_cast<const char*>(bin_data[0].data()), bin_data[0].size());
            out.close();
            
            std::cout << "💾 OpenCL-Binary erfolgreich exportiert: " << binary_path 
                      << " (" << bin_data[0].size() << " Bytes) nach " << binary_path << std::endl << std::flush;
        }
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


// [[Rcpp::export]]
std::string CLGetHardwareCachePath() {
    // 1. Hardware-Abfrage (nutzt die gleichen statischen Handles wie der Hauptlauf)
    static cl::Platform* best_platform = nullptr;
    static cl::Device* best_device = nullptr;
    static bool hw_init = false;

    if (!hw_init) {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (!platforms.empty()) {
            std::vector<cl::Device> devices;
            platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);
            if (!devices.empty()) {
                best_device = new cl::Device(devices[0]);
                best_platform = new cl::Platform(platforms[0]);
                hw_init = true;
            }
        }
    }

    if (!hw_init) return "";

    // 2. Architektur und OS-Labels bestimmen
    std::string os_label = "unknown";
    std::string arch_label = "x86_64";
#if defined(__x86_64__) || defined(_M_X64)
    arch_label = "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
    arch_label = "arm64";
#endif

#if defined(_WIN32)
    os_label = "windows_" + arch_label;
#elif defined(__APPLE__)
    os_label = "macos_" + arch_label;
#elif defined(__linux__)
    os_label = "linux_" + arch_label;
#endif

    std::string platform_name = best_platform->getInfo<CL_PLATFORM_NAME>();
    std::string device_name = best_device->getInfo<CL_DEVICE_NAME>();
    
    auto clean_str = [](std::string s) {
        std::string res = "";
        for (char c : s) {
            if (std::isalnum(c)) res += std::tolower(c);
            else if (c == ' ' || c == '-' || c == '_') res += '_';
        }
        return res;
    };

    // Baut exakt deine Wunschstruktur: .cl_cache/OS_Arch/Platform/Device/cl_distance_matrix.bin
    std::string target_dir = "./.cl_cache/" + os_label + "/" + clean_str(platform_name) + "/" + clean_str(device_name);
    return target_dir + "/cl_distance_matrix.bin";
}
