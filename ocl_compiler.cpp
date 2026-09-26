#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 300
#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_MINIMUM_OPENCL_VERSION 100

#include "src/OpenCL/include/CL/opencl.hpp"
#include "src/opencl.hpp"

// Alibi-Funktion für den Wrapper (wird in opencl.hpp benötigt)
inline std::string get_opencl_c_code() { return "\n"; }

void print_usage() {
    std::cout << "Verwendung: ocl_compiler -i <kernel.cl> -o <output.bin> -p <platform_index> -d <device_index>\n";
}

int main(int argc, char* argv[]) {
    std::string input_path = "";
    std::string output_path = "";
    int platform_idx = 0;
    int device_idx = 0;

    // 🎯 Flexibler CLI-Parser für R-Schnittstelle
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-i" || arg == "--input") && i + 1 < argc) input_path = argv[++i];
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) output_path = argv[++i];
        else if ((arg == "-p" || arg == "--platform") && i + 1 < argc) platform_idx = std::stoi(argv[++i]);
        else if ((arg == "-d" || arg == "--device") && i + 1 < argc) device_idx = std::stoi(argv[++i]);
    }

    if (input_path.empty() || output_path.empty()) {
        print_usage();
        return 1;
    }

    // 1. Beliebige Kernel-Quelldatei von Festplatte einlesen
    std::ifstream kernel_file(input_path);
    if (!kernel_file.good()) {
        std::cerr << "Fehler: Kernel-Datei konnte nicht geoeffnet werden: " << input_path << "\n";
        return 1;
    }
    std::stringstream str_stream;
    str_stream << kernel_file.rdbuf();
    std::string core_kernel = str_stream.str();
    kernel_file.close();

    try {
        // 2. OpenCL-Hardware gezielt anhand der R-Argumente ansteuern
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platform_idx >= (int)platforms.size()) {
            std::cerr << "Fehler: Plattform-Index " << platform_idx << " existiert nicht.\n";
            return 1;
        }
        cl::Platform platform = platforms[platform_idx];

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        if (device_idx >= (int)devices.size()) {
            std::cerr << "Fehler: Device-Index " << device_idx << " auf Plattform " << platform_idx << " existiert nicht.\n";
            return 1;
        }
        cl::Device device = devices[device_idx];

        cl::Context context(device);
        cl_int err = 0;
        cl::CommandQueue queue(context, device, 0, &err);

        // Instanziere den Framework-Wrapper (Windows nutzt hier 'false' zwecks Thread-Sicherheit)
        Device physx_device(context(), device(), queue());

        // 3. Hardware-Power ermitteln und Typdefinitionen dynamisch davorheften
        std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
        bool is_fp64 = (extensions.find("cl_khr_fp64") != std::string::npos);
        
        std::string prolog = "";
        if (!is_fp64) {
            prolog = "#define real_t float\n#define real2_t float2\n";
        } else {
            prolog = "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n#define real_t double\n#define real2_t double2\n";
        }
        
        std::string final_kernel_code = prolog + "\n" + core_kernel;
        physx_device.set_kernel_source(final_kernel_code);
        
        // 4. JIT-Kompilierung ohne riskantere Math-Flags ausführen
        physx_device.compile_kernel("-cl-opt-disable", false);

        // 5. 🚀 ABSOLUT BYTESYNCHRONER BINÄR-EXPORT (Sichert den ELF-Header auf Windows)
        auto bin_data = physx_device.get_cl_program().getInfo<CL_PROGRAM_BINARIES>();
        if (!bin_data.empty() && bin_data[0].size() > 0) {
            std::ofstream out(output_path, std::ios::binary | std::ios::out);
            
            // Greift gezielt auf das Byte-Array des primären Geräts zu [0]
            // reinterpret_cast verhindert implizite signed/unsigned-Vorzeichenfehler von Windows-Streams
            out.write(reinterpret_cast<const char*>(bin_data[0].data()), bin_data[0].size());
            out.close();
            
            std::cout << "SUCCESS" << std::endl;
        } else {
            std::cerr << "Fehler: Keine gueltigen OpenCL-Binaries vom Treiber zurueckgegeben.\n";
            return 1;
        }
    }
    catch (cl::Error &err) {
        std::cerr << "OpenCL CLI-Compiler Fehler: " << err.what() << " (" << err.err() << ")\n";
        return 1;
    }
    return 0;
}

// set PATH=C:\rtools45\usr\bin;C:\rtools45\x86_64-w64-mingw32.static.posix\bin;%PATH%
// g++ -O2 ocl_compiler.cpp -o ocl_compiler.exe -I./ -I./src/OpenCL/include -L./src/OpenCL/lib -lOpenCL
