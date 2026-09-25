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
    ## 🎯 Absolut prozess- und multi-user-sicher im R-eigenen, flüchtigen Session-Ordner
    session_binary_path <- file.path(tempdir(), "cl_distance_matrix.bin")
    Sys.setenv(R_OPENCL_BINARY_PATH = session_binary_path)
    
  if (!file.exists(session_binary_path)) {
    message("🔄 Starte OpenCL-Build im Hintergrund mit permanentem Logging...")
    
    # 📝 Festgelegter, stabiler Pfad für das ausgeführte Skript (bleibt stehen!)
    # 📝 Dynamische Pfade innerhalb des flüchtigen tempdir()
    run_script <- file.path(tempdir(), "ocl_hintergrund_run.R")
    log_file   <- file.path(tempdir(), "opencl_hintergrund.log")
       
    script_lines <- c(
      "cat('=== HINTERGRUND-PROZESS GESTARTET ===\\n')",
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
      "cat('Kompiliere sourceCpp im Hintergrund...\\n')",
      "Rcpp::sourceCpp('CLDistanceMatrixDirect.cpp', rebuild = FALSE)",
      paste0("Sys.setenv(R_OPENCL_BINARY_PATH = '", normalizePath(session_binary_path, winslash = "/"), "')"),
      # Bei Windows-Fehlersuche aktivieren wir auch hier den Print-Output im Treiber
      "Sys.setenv(R_OPENCL_PRINT_ENABLED = '1')",
      paste0("mat_dummy <- matrix(0, ", nrow(mat), ", ", ncol(mat), ")"),
      "cat('Rufe CLDistanceMatrixDirect auf...\\n')",
      "tryCatch({",
      "  CLDistanceMatrixDirect(mat_dummy)",
      "  cat('=== C++ SEITE BEENDET ODER IM LOOK-LOCK ===\\n')",
      "}, error = function(e) {",
      "  cat('💥 CRASH IM C++ AUFRUF:\\n')",
      "  print(e)",
      "})"
    )
    
    writeLines(script_lines, run_script)
    

    # 🛡️ BETRIEBSSYSTEM-WEICHE FÜR RSCRIPT:
    if (.Platform$OS.type == "windows") {
        # Windows nutzt den präzisen Scoop-x64-Pfad
        ## 🎯 Dynamische x64-Pfad-Generierung für Windows (Scoop-kompatibel):
        # R.home("bin") liefert z.B.: "C:/Users/agebhard/scoop/apps/r/current/bin/x64"
        # normalizePath sorgt für saubere Windows-Slashes
        rscript_dir <- normalizePath(R.home("bin"), winslash = "/")
        rscript_path <- file.path(rscript_dir, "Rscript.exe")
    } else {
        # Linux/Ubuntu nutzt die native System-Verknüpfung
        rscript_path <- "Rscript"
    }    
    # 🚀 JETZT LEITEN WIR STDOUT UND STDERR DIREKT IN DIE LOG-DATEI UM
    system2(rscript_path, 
            args = shQuote(run_script), 
            wait = FALSE, 
            stdout = log_file,   # Fängt allen normalen Output ab
            stderr = log_file)   # Fängt alle harten Compiler-/Linker- und DLL-Fehler ab
    
    # Überwachungsschleife (300 Sekunden)
    max_wait <- 300
    waited <- 0
    while (!file.exists(session_binary_path) && waited < max_wait) {
      Sys.sleep(1)
      waited <- waited + 1
      if (waited %% 5 == 0) message(paste0("... warte auf Compiler (", waited, "s/300s) ..."))
    }
    
    if (!file.exists(session_binary_path)) {
      message(paste0("⚠️ Hintergrund-Build hat das Limit erreicht. Prüfe das Log unter: ", log_file))
    } else {
      message("✅ OpenCL-Binary erfolgreich erzeugt!")
    }
    
    # 🛡️ HINWEIS: unlink() ist hier absichtlich ENTFERNT!
    # Die Dateien 'C:/Temp/ocl_hintergrund_run.R' und 'C:/Temp/opencl_hintergrund.log' bleiben stehen!
  }
  
  return(CLDistanceMatrixDirect(mat))
}
