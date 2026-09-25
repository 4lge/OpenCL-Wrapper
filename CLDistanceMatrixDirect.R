# 🚀 1. DLL vorab laden
Sys.setenv(INTEL_OCL_CACHE_DISABLE = "1")
Sys.setenv(cl_cache_dir = "")
Sys.setenv(PKG_CXXFLAGS = paste("-I", getwd(), "/src/OpenCL/include ", sep = ""))
if (.Platform$OS.type == "windows") {
    Sys.setenv(PKG_LIBS = "-L\"c:/Program Files (x86)/Common Files/Intel/Shared Libraries/bin\" -lOpenCL")
} else {
    Sys.setenv(PKG_LIBS = "-lOpenCL")
}

message("Compiling Rcpp/OpenCL wrappers in main session...")
Rcpp::sourceCpp("CLDistanceMatrixDirect.cpp", rebuild = FALSE)

cl_distance_matrix_direct <- function(mat) {
  # 🎯 SCHRITT A: R fragt C++ direkt nach dem exakten Architektur-Pfad
  raw_path <- CLGetHardwareCachePath()
  if (raw_path == "") {
      stop("💥 Fehler: OpenCL-Hardware konnte nicht abgefragt werden!")
  }
  
  # Pfad absolut und plattformunabhängig auflösen
  session_binary_path <- normalizePath(raw_path, winslash = "/", mustWork = FALSE)
  
  # Sicherstellen, dass das spezifische C++ Verzeichnis existiert
  cache_dir <- dirname(session_binary_path)
  if (!dir.exists(cache_dir)) dir.create(cache_dir, recursive = TRUE, showWarnings = FALSE)
  
  # Den Pfad via Umgebungsvariable für den Child-Prozess bereitstellen
  Sys.setenv(R_OPENCL_BINARY_PATH = session_binary_path)

  run_script <- file.path(tempdir(), "ocl_hintergrund_run.R")
  log_file   <- file.path(tempdir(), "opencl_hintergrund.log")

  # 2. Wenn das spezifische Hardware-Binary fehlt -> Headless-Build via Rscript
  if (!file.exists(session_binary_path)) {
    message("🔄 Starte OpenCL-Build im Hintergrund für diese Architektur...")
    
    script_lines <- c(
      "library(Rcpp)",
      paste0("setwd('", normalizePath(getwd(), winslash = "/"), "')"),
      "Sys.setenv(INTEL_OCL_CACHE_DISABLE = '1')",
      "Sys.setenv(cl_cache_dir = '')",
      paste0("Sys.setenv(PKG_CXXFLAGS = '-I", getwd(), "/src/OpenCL/include')"),
      if (.Platform$OS.type == "windows") {
        "Sys.setenv(PKG_LIBS = '-L\"c:/Program Files (x86)/Common Files/Intel/Shared Libraries/bin\" -lOpenCL')"
      } else {
        "Sys.setenv(PKG_LIBS = '-lOpenCL')"
      },
      "Rcpp::sourceCpp('CLDistanceMatrixDirect.cpp', rebuild = FALSE)",
      # Der Hintergrund-Prozess bekommt den Pfad direkt mitgegeben
      paste0("Sys.setenv(R_OPENCL_BINARY_PATH = '", session_binary_path, "')"),
      paste0("mat_dummy <- matrix(0, ", nrow(mat), ", ", ncol(mat), ")"),
      "CLDistanceMatrixDirect(mat_dummy)"
    )
    
    writeLines(script_lines, run_script)
    
    if (.Platform$OS.type == "windows") {
        rscript_dir <- normalizePath(R.home("bin"), winslash = "/")
        rscript_path <- file.path(rscript_dir, "Rscript.exe")
    } else {
        rscript_path <- "Rscript"
    }
    
    system2(rscript_path, args = shQuote(run_script), wait = FALSE, stdout = log_file, stderr = log_file)
    
    max_wait <- 300
    waited <- 0
    while (!file.exists(session_binary_path) && waited < max_wait) {
      Sys.sleep(1)
      waited <- waited + 1
      if (waited %% 5 == 0) message(paste0("... warte auf Compileroutput unter ", session_binary_path, " (", waited, "s/300s) ..."))
      
    }
    
    if (!file.exists(session_binary_path)) {
      stop(paste0("💥 Hintergrund-Build hat das Limit erreicht. Prüfe das Log unter: ", log_file))
    }
    message("✅ OpenCL-Binary erfolgreich erzeugt!")
  }
  
  # 3. Hauptrechenlauf starten (Läuft ab jetzt in 1 Millisekunde!)
  return(CLDistanceMatrixDirect(mat))
}
