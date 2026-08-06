#!/usr/bin/env bash

# useful when using POCL:
POCL_LEAVE_KERNEL_COMPILER_TEMP_FILES=1
export POCL_LEAVE_KERNEL_COMPILER_TEMP_FILES

Rscript -e "rmarkdown::render(\"OpenCL-Wrapper.Rmd\", output_format = \"md_document\")"

mkdir -p bin # create directory for executable
rm -f bin/OpenCL-Wrapper # prevent execution of old version if compiling fails

case "$(uname -a)" in # automatically detect operating system
	 Darwin*) 
        # Holt die macOS-Version (z.B. "10.14.6" oder "11.1")
        MACOS_VER=$(sw_vers -productVersion)
        # Extrahiert die Haupt- und Nebenversionsnummer
        MACOS_MAJOR=$(echo "$MACOS_VER" | cut -d. -f1)
        MACOS_MINOR=$(echo "$MACOS_VER" | cut -d. -f2)
        
        # Initialisiere die zusätzlichen Flags für den Mac
        MAC_FLAGS="-framework OpenCL"
        
        # Wenn Version 10.x und kleiner als 10.15, hänge das Linker-Flag an
        if [ "$MACOS_MAJOR" -eq 10 ] && [ "$MACOS_MINOR" -lt 15 ]; then
            MAC_FLAGS="$MAC_FLAGS" ### " -lstdc++fs"
        fi

        g++ -g -O0 src/*.cpp -o bin/OpenCL-Wrapper -I./ -std=c++17 -pthread -O -Wno-comment -I./src/OpenCL/include $MAC_FLAGS
        ;; 
	*Android) g++ -g -O0 src/*.cpp -o bin/OpenCL-Wrapper -I./ -std=c++17 -pthread -O -Wno-comment -I./src/OpenCL/include -L/system/vendor/lib64 -lOpenCL ;; # Android
	*       ) g++ -g -O0 src/*.cpp -o bin/OpenCL-Wrapper -I./ -std=c++17 -pthread -O -Wno-comment -I./src/OpenCL/include -L./src/OpenCL/lib -lOpenCL     ;; # Linux
esac

if [[ $? == 0 ]]; then bin/OpenCL-Wrapper "$@"; fi # run executable only if last compilation was successful
