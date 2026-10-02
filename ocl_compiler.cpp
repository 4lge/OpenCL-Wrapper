#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <cstdlib>


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
    std::cout << "Verwendung: ocl_compiler -i <kernel.cl> -c <cache_base_dir> -p <platform_idx> -d <device_idx> [-l <library_file.cl>]\\n";
}

int main(int argc, char* argv[]) {
    std::string input_path = "";
    std::string cache_base = "./.cl_cache";
    std::string library_file_path = ""; // 🚀 Einheitlicher Name für den Parser
    int platform_idx = 0;
    int device_idx = 0;

    // 🎯 Der CLI-Parser akzeptiert jetzt die Basis anstelle des Vollpfads
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-i" || arg == "--input") && i + 1 < argc) input_path = argv[++i];
        else if ((arg == "-c" || arg == "--cache") && i + 1 < argc) cache_base = argv[++i];
        else if ((arg == "-p" || arg == "--platform") && i + 1 < argc) platform_idx = std::stoi(argv[++i]);
        else if ((arg == "-d" || arg == "--device") && i + 1 < argc) device_idx = std::stoi(argv[++i]);
        else if ((arg == "-l" || arg == "--library") && i + 1 < argc) library_file_path = argv[++i]; 
        else if ((arg == "-h" || arg == "--help") && i + 1 < argc) print_usage(); 
    }
    
    if (input_path.empty()) {
        print_usage();
        std::_Exit(1);
    }

    std::ifstream kernel_file(input_path);
    if (!kernel_file.good()) {
        std::cerr << "Fehler: Kernel-Datei konnte nicht geoeffnet werden: " << input_path << "\n";
        std::_Exit(1);
    }
    std::stringstream str_stream;
    str_stream << kernel_file.rdbuf();
    std::string core_kernel = str_stream.str();
    kernel_file.close();
    // 🚀 2. NEU: Optionale Math-Library einlesen (falls übergeben)
    std::string math_library_code = "";
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
    // 🎯 ÄUẞERE VARIABLEN: Für den catch-Scope sichtbar herausgezogen!
    cl::Device device;
    cl::Program cl_program;
    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platform_idx >= (int)platforms.size()) {
            std::cerr << "Fehler: Plattform-Index " << platform_idx << " existiert nicht.\n";
            std::_Exit(1);
        }
        cl::Platform platform = platforms[platform_idx];

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        if (device_idx >= (int)devices.size()) {
            std::cerr << "Fehler: Device-Index " << device_idx << " existiert nicht.\n";
            std::_Exit(1);
        }
        cl::Device device = devices[device_idx];

        cl::Context context(device);
        cl_int err = 0;
        cl::CommandQueue queue(context, device, 0, &err);

        Device physx_device(context(), device(), queue());

        physx_device.set_kernel_source(core_kernel);
        
        physx_device.compile_kernel("-cl-opt-disable", false);

        // Handle für den catch-Block sichern
        cl_program = physx_device.get_cl_program();

        // 🎯 LOGISCHE RESTRUKTURIERUNG DES OUTPUTS:
        // Der Compiler baut sich seinen Zielpfad jetzt vollkommen autonom zusammen!

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
        std::string filename = raw_file.substr(0, raw_file.find_last_of("."));

        // Hier entsteht der exakt identische Pfad wie in Rcpp
        std::string output_path = cache_base + "/" + os_label + "/" + 
                                  clean_str(platform.getInfo<CL_PLATFORM_NAME>()) + "/" + 
                                  clean_str(device.getInfo<CL_DEVICE_NAME>()) + "/" + 
                                  filename + ".bin";

        // 🎯 FIX: Korrekter Zugriff auf das Khronos-Vektor-Layout [0]
        auto bin_data = cl_program.getInfo<CL_PROGRAM_BINARIES>();
        if (!bin_data.empty() && bin_data[0].size() > 0) {
            std::ofstream out(output_path, std::ios::binary | std::ios::out);
            // Greift gezielt auf das Byte-Array des primären Geräts zu [0]
            out.write(reinterpret_cast<const char*>(bin_data[0].data()), bin_data[0].size());
            out.close();
            
            std::cout << "SUCCESS" << std::endl << std::flush;
            //cstd::_Exit(0);
            return(0);
        } else {
            std::cerr << "Fehler: Keine gueltigen OpenCL-Binaries vom Treiber zurueckgegeben.\n";
            //std::_Exit(1);
            return(1);
        }
    }
    catch (cl::Error &err) {
      //        std::cerr << "OpenCL CLI-Compiler Fehler: " << err.what() << " (" << err.err() << ")\n";
        
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
            std::cerr << "==================================================================\n\n";
        }
      
        //std::_Exit(1);
        return(1);
    }

    //std::_Exit(0); 
    return(0);
}

// set PATH=C:\rtools45\usr\bin;C:\rtools45\x86_64-w64-mingw32.static.posix\bin;%PATH%
// g++ -O2 ocl_compiler.cpp -o ocl_compiler.exe -I./ -I./src/OpenCL/include -L./src/OpenCL/lib -lOpenCL
