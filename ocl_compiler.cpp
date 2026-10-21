#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <unistd.h> // für _exit() unter Linux

#ifdef __APPLE__
  #define CL_TARGET_OPENCL_VERSION 120
  #define CL_HPP_TARGET_OPENCL_VERSION 120
  #define CL_HPP_MINIMUM_OPENCL_VERSION 120
#else
  #define CL_TARGET_OPENCL_VERSION 300
  #define CL_HPP_TARGET_OPENCL_VERSION 300
  #define CL_HPP_MINIMUM_OPENCL_VERSION 100
#endif
#ifndef CL_HPP_ENABLE_EXCEPTIONS
  #define CL_HPP_ENABLE_EXCEPTIONS
#endif
 
// 🎯 DIE GLOBALE SPEICHER-VARIABLE FÜR DIE LIBRARAY
static std::string global_math_library_code = "\n";

// 🎯 HIER WIRD DIE DATEI DYNAMISCH AUSGEGEBEN
inline std::string get_opencl_c_code() { 
    return global_math_library_code; 
}

#include "src/OpenCL/include/CL/opencl.hpp"
#include "src/opencl.hpp"

void print_usage() {
  std::cout << "Verwendung: ocl_compiler [-k <kernel_path>] -i <kernel.cl> -c <cache_base_dir> "
            << "-p <platform_idx> -d <device_idx> [-P <platform_name>] [-D <device_name>] "
            << "[-l <library_file.cl>] [-f] [-o <options>] [-v]" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string input_path = "";
    std::string kernel_dir  = "";
    std::string cache_base = "./.cl_cache";
    std::string library_file_path = ""; 
    std::string build_opts = ""; 
    
    // 🚀 HIER SIND DEINE NEUEN STRATEGISCHEN TUNNEL-ARGUMENTE:
    std::string rcpp_platform_name = "";
    std::string rcpp_device_name = "";

    int platform_idx = 0;
    int device_idx = 0;
    bool force_float = false; 

    // 🎯 Der CLI-Parser fängt alle Argumente blockfrei ab
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-i" || arg == "--input") && i + 1 < argc) input_path = argv[++i];
        else if ((arg == "-c" || arg == "--cache") && i + 1 < argc) cache_base = argv[++i];
        else if ((arg == "-p" || arg == "--platform") && i + 1 < argc) platform_idx = std::stoi(argv[++i]);
        else if ((arg == "-d" || arg == "--device") && i + 1 < argc) device_idx = std::stoi(argv[++i]);
        else if ((arg == "-l" || arg == "--library") && i + 1 < argc) library_file_path = argv[++i]; 
        else if ((arg == "-k" || arg == "--kernel-dir") && i + 1 < argc) kernel_dir = argv[++i];
        else if ((arg == "-o" || arg == "--options") && i + 1 < argc) build_opts = argv[++i]; 
        
        // 🔥 ERWEITERUNG: Rcpp Tunnel-Eingänge abfangen!
        else if ((arg == "-P" || arg == "--platform-name") && i + 1 < argc) rcpp_platform_name = argv[++i];
        else if ((arg == "-D" || arg == "--device-name") && i + 1 < argc) rcpp_device_name = argv[++i];
        
        else if (arg == "-f" || arg == "--force-float") force_float = true; 
        else if (arg == "-v" || arg == "--verbose") {
            get_opencl_print_enabled() = true; 
            std::cout << "🔍 [ocl_compiler] Maximaler Verbose-Modus via CLI aktiviert!" << std::endl;
        }
        else if ((arg == "-h" || arg == "--help")) print_usage();
    }
    
    if (input_path.empty()) {
        print_usage();
        return(1);
    }

    std::string kernel_file_name = input_path; 
    std::string kernel_dir_path  = kernel_dir;  
    std::string kernel_path_final = "";

    // 🚀 STRIKTE PFAD-WEICHE FÜR DEN KERNEL IN C++
    bool starts_with_dot = (kernel_file_name.rfind("./", 0) == 0 || kernel_file_name.rfind(".\\", 0) == 0);

    if (starts_with_dot) {
        std::ifstream test_file(kernel_file_name);
        if (test_file.good()) {
            kernel_path_final = kernel_file_name;
        } else {
            std::cerr << "💥 [ocl_compiler] Explizit angeforderte lokale Kernel-Datei nicht gefunden: " << kernel_file_name << std::endl;
            _exit(1);
        }
    } else {
        std::ifstream local_file(kernel_file_name);
        if (local_file.good()) {
            kernel_path_final = kernel_file_name;
        } else {
            std::string fallback_path = kernel_dir_path + "/" + kernel_file_name;
            std::ifstream package_file(fallback_path);
            if (package_file.good()) {
                kernel_path_final = fallback_path;
            } else {
                std::cerr << "💥 [ocl_compiler] Kernel-Datei weder lokal noch unter " << fallback_path << " gefunden!" << std::endl;
                _exit(1);
            }
        }
    }

    std::cout << "🎯 ocl_compiler nutzt Kernel-Quelle: " << kernel_path_final << std::endl;

    std::ifstream kernel_file(kernel_path_final);
    if (!kernel_file.good()) {
        std::cerr << "Fehler: Kernel-Datei konnte nicht geoeffnet werden: " << kernel_path_final << "\n";
        return(1);
    }
    std::stringstream str_stream;
    str_stream << kernel_file.rdbuf();
    std::string core_kernel = str_stream.str();
    kernel_file.close();

    if (!library_file_path.empty()) {
        std::ifstream lib_file(library_file_path);
        if (lib_file.good()) {
            std::stringstream lib_stream;
            lib_stream << lib_file.rdbuf();
            global_math_library_code = lib_stream.str() + "\n";
            lib_file.close();
        } else {
            std::cerr << "⚠️ Warnung: Math-Library '" << library_file_path << "' konnte nicht geoeffnet werden! Fahre ohne fort.\n";
        }
    }

    // =========================================================================
    // 🎯 OS- UND ARCH-LABELS
    // =========================================================================
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

    auto clean_str = [](std::string s) {
        std::string res = "";
        for (char c : s) {
            if (std::isalnum(c)) res += std::tolower(c);
            else if (c == ' ' || c == '-' || c == '_') res += '_';
        }
        return res;
    };

    size_t last_slash = input_path.find_last_of("/\\");
    std::string raw_file = (last_slash == std::string::npos) ? input_path : input_path.substr(last_slash + 1);
    std::string base_filename = raw_file.substr(0, raw_file.find_last_of("."));
    std::string filename = base_filename + (force_float ? "_float" : "_double");

    // 🎯 ÄUẞERE VARIABLEN: Für den Scope sichtbar herausgezogen
    cl::Device device;
    cl::Program cl_program;
    std::string output_path = "";
    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platform_idx >= (int)platforms.size()) {
            std::cerr << "Fehler: Plattform-Index " << platform_idx << " existiert nicht.\n";
            return(1);
        }
        cl::Platform platform = platforms[platform_idx];

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        if (device_idx >= (int)devices.size()) {
            std::cerr << "Fehler: Device-Index " << device_idx << " existiert nicht.\n";
            return(1);
        }
        device = devices[device_idx];

        cl::Context context(device);
        cl_int err = 0;
        cl::CommandQueue queue(context, device, 0, &err);

        // 🎯 1. STRATEGISCHE PFAD-DECISION:
        // Haben wir explizite Namen aus Rcpp diktiert bekommen? Wenn ja, nutzen wir sie bitscharf!
        std::string final_platform_folder = rcpp_platform_name.empty() ? clean_str(platform.getInfo<CL_PLATFORM_NAME>()) : rcpp_platform_name;
        std::string final_device_folder   = rcpp_device_name.empty()   ? clean_str(device.getInfo<CL_DEVICE_NAME>())   : rcpp_device_name;

        // Hier entsteht der absolut synchrone Zielpfad
        output_path = cache_base + "/" + os_label + "/" + 
                      final_platform_folder + "/" + 
                      final_device_folder + "/" + 
                      filename + ".bin";

        std::string platform_name = platform.getInfo<CL_PLATFORM_NAME>();
        std::string platform_lower = "";
        for (char c : platform_name) platform_lower += std::tolower(c);
        
        bool is_amd = (platform_lower.find("amd") != std::string::npos);
        bool is_cpu = (device.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_CPU);

        // =========================================================================
        // 🚀 DER PROAKTIVE AMD-GPU SONDERWEG (Jetzt 100% gekoppelt an Rcpp!)
        // =========================================================================
        if (is_amd && !is_cpu) {
            std::cout << "🏎️ [ocl_compiler] AMD GPU erkannt (Ordner-Target: " << final_device_folder << ")!" << std::endl;
            std::cout << "🛡️ Schalte proaktiv auf den AMD-Clang-Offline-Tunnel um..." << std::endl;

            // 🚀 AUTOMATISCHER ORDNER-ERZEUGER (Verhindert 'No such file or directory')
            size_t final_slash = output_path.find_last_of("/\\");
            if (final_slash != std::string::npos) {
                std::string target_dir = output_path.substr(0, final_slash);
                std::string mkdir_cmd = "mkdir -p " + target_dir;
                std::system(mkdir_cmd.c_str()); 
            }

            // Vollständigen OpenCL-C Quelltext im RAM emulieren
            std::string tmp_cl_source = cache_base + "/amd_temp_jit.cl";
            std::ofstream tmp_out(tmp_cl_source);
            
            tmp_out << "#define cl_workgroup_size 64u\n"
                    << (force_float ? "typedef float real_t;\n" : "typedef double real_t;\n")
                    << "typedef " << (force_float ? "float2" : "double2") << " real2_t;\n"
                    << "typedef " << (force_float ? "float3" : "double3") << " real3_t;\n"
                    << "typedef " << (force_float ? "float4" : "double4") << " real4_t;\n"
                    << "typedef " << (force_float ? "float8" : "double8") << " real8_t;\n"
                    << global_math_library_code << "\n" << core_kernel;
            tmp_out.close();

            // Kommandozeile für das native AMD-Compiler-Frontend (Target wird aus Rcpp-Vorgabe gefüttert!)
            std::string clang_cmd = "/opt/rocm/llvm/bin/clang -x cl -cl-std=CL2.0 " 
                                    "-Xclang -finclude-default-header " 
                                    "-target amdgcn-amd-amdhsa -mcpu=" + final_device_folder + 
                                    " " + tmp_cl_source + " -o " + output_path;

            std::cout << "🔄 [BACKEND-EXEC] " << clang_cmd << std::endl;
            
            int exit_code = std::system(clang_cmd.c_str());
            std::remove(tmp_cl_source.c_str()); // Temporäre Quelltext-Leiche löschen

            if (exit_code == 0) {
                std::cout << "🎉 [ocl_compiler] Proaktiver AMD-Clang-Build erfolgreich!" << std::endl;
                std::cout << "SUCCESS" << std::endl << std::flush;
                _exit(0); 
            } else {
                std::cerr << "💥 Fehler: Proaktiver AMD-Clang-Build fehlgeschlagen mit Code: " << exit_code << std::endl;
                return(1);
            }
        }

        // =========================================================================
        // STANDARD-PFAD (Für Nvidia, Intel GPU, Intel CPU und AMD CPU via PoCL)
        // =========================================================================
        Device physx_device(context(), device(), queue());
// 🚀 DER POCL- & ALT-SYSTEM-SCHUTZWALL:
// Frage das Gerät ab, welche OpenCL-C-Version es REEL unterstützt!
std::string device_cl_version = device.getInfo<CL_DEVICE_OPENCL_C_VERSION>();
 
std::string cl_std_flag = "-cl-std=CL1.2"; // Sicherer Standard-Fallback

if (device_cl_version.find("3.0") != std::string::npos) {
    cl_std_flag = "-cl-std=CL3.0";
} else if (device_cl_version.find("2.0") != std::string::npos) {
    cl_std_flag = "-cl-std=CL2.0";
} else if (device_cl_version.find("1.2") != std::string::npos) {
    cl_std_flag = "-cl-std=CL1.2";
}

// Füge das dynamisch ermittelte Flag zu deinen restlichen Optionen hinzu:
std::string options = cl_std_flag + " -cl-mad-enable ";
 
        if (force_float) {
          std::cout << "🏎️ [ocl_compiler] Zwinge Device-Info in den FLOAT-Modus (-f aktiv)!" << std::endl;
          physx_device.info.is_fp64_capable = 0; 
        } 
    
        physx_device.set_kernel_source(core_kernel);
// 🔍 DER FORENSIK-PRINT VOR clProgramBuild / program.build():
std::cout << "\n================= BACKEND DEBUG START =================" << std::endl;
std::cout << "▶ Eigene übergebene Options-Variable: '" << options << "'" << std::endl;

// Prüfen, ob Linux-Umgebungsvariablen reinschießen
if (const char* env_args = std::getenv("RUSTICL_COMPILER_ARGS")) {
    std::cout << "▶ Umgebung RUSTICL_COMPILER_ARGS: '" << env_args << "'" << std::endl;
}
if (const char* env_cl = std::getenv("OCL_ICD_VENDORS")) {
    std::cout << "▶ Umgebung OCL_ICD_VENDORS: '" << env_cl << "'" << std::endl;
}

// Auslesen, was das OpenCL-Gerät intern als Standard-Optionen definiert
std::string dev_ver = device.getInfo<CL_DEVICE_OPENCL_C_VERSION>();
std::cout << "▶ Physische Device OpenCL C Version: " << dev_ver << std::endl;
std::cout << "================== BACKEND DEBUG END ==================\n" << std::endl;
std::cout << "▶ options passed to compile_kernel: " << options+build_opts << std::endl;
std::cout << "================== BACKEND DEBUG END ==================\n" << std::endl;
        
        physx_device.compile_kernel(options+build_opts, false);

        cl_program = physx_device.get_cl_program();

        // Falls wir nicht im AMD Sonderweg waren, stellen wir sicher, dass Ordner existieren
        size_t final_slash = output_path.find_last_of("/\\");
        if (final_slash != std::string::npos) {
            std::string target_dir = output_path.substr(0, final_slash);
            std::string mkdir_cmd = "mkdir -p " + target_dir;
            std::system(mkdir_cmd.c_str()); 
        }

        auto bin_data = cl_program.getInfo<CL_PROGRAM_BINARIES>();
        if (!bin_data[0].empty() && bin_data[0].size() > 0) {
            std::ofstream out(output_path, std::ios::binary | std::ios::out);
            out.write(reinterpret_cast<const char*>(bin_data[0].data()), bin_data[0].size());
            out.close();
            
            std::cout << "SUCCESS" << std::endl << std::flush;
            std::cerr << std::flush;
            
            #ifndef _WIN32
              _exit(0); 
            #else
              return(0); 
            #endif
        } else {
            std::cerr << "Fehler: Keine gueltigen OpenCL-Binaries vom Treiber zurueckgegeben.\n" << std::flush;
            return(1);
        }
    }
    catch (cl::Error &err) {
        if (err.err() == CL_BUILD_PROGRAM_FAILURE && device() != nullptr) {
            std::cerr << "\n=================== NATIVE OPENCL COMPILER LOG ===================\n";
            try {
                std::string build_log = cl_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
                std::cerr << build_log << "\n";
            } 
            catch (const cl::Error &log_err) {
                std::cerr << "💥 Fehler beim Abrufen des Logs über den C++ Wrapper: " << log_err.what() << "\n";
                std::cerr << "Versuche rohe C-API Abfrage...\n";
                
                size_t log_size = 0;
                clGetProgramBuildInfo(cl_program(), device(), CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
                if (log_size > 0) {
                    std::vector<char> raw_log(log_size);
                    clGetProgramBuildInfo(cl_program(), device(), CL_PROGRAM_BUILD_LOG, log_size, raw_log.data(), NULL);
                    std::cerr << raw_log.data() << "\n";
                }
            }
            std::cerr << "==================================================================\n\n" << std::flush;
        }
      
        return(1);
    }

    return(0);
}
