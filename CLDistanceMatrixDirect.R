# 🚀 1. DLL vorab laden
Sys.setenv(INTEL_OCL_CACHE_DISABLE = "1")
Sys.setenv(cl_cache_dir = "")
Sys.setenv(PKG_CXXFLAGS = paste("-I", getwd(), "/src/OpenCL/include ", sep = ""))
if (.Platform$OS.type == "windows") {
    cat("Windows:")
    Sys.setenv(PKG_LIBS = "-L\"c:/Program Files (x86)/Common Files/Intel/Shared Libraries/bin\" -lOpenCL")
} else if (Sys.info()["sysname"] == "Darwin") {
    # 🍏 macOS Framework-Linker für Apple Silicon/Intel-Macs
    cat("macOS:")
    Sys.setenv(PKG_LIBS = "-framework OpenCL")
} else {
    cat("Linux or else:")
    Sys.setenv(PKG_LIBS = "-lOpenCL")
}

message("Compiling Rcpp/OpenCL wrappers in main session...")
Rcpp::sourceCpp("CLDistanceMatrixDirect.cpp", rebuild = FALSE)


cl_distance_matrix_direct <- function(mat, platform_idx = 0, device_idx = 0) {
     return(CLDistanceMatrixDirect(mat, as.integer(platform_idx), as.integer(device_idx)))
}
