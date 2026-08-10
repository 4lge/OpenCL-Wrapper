#!/usr/bin/env bash

# useful when using POCL:
POCL_LEAVE_KERNEL_COMPILER_TEMP_FILES=1
export POCL_LEAVE_KERNEL_COMPILER_TEMP_FILES

Rscript -e "rmarkdown::render(\"OpenCL-Wrapper.Rmd\", output_format = \"md_document\")"

mkdir -p bin # create directory for executable
rm -f bin/OpenCL-Wrapper # prevent execution of old version if compiling fails

case "$(uname -a)" in # automatically detect operating system
	 Darwin*) 
        # get macOS-Version (e.g. "10.14.6" or "11.1")
        MACOS_VER=$(sw_vers -productVersion)
        # extract MAJOR AND MINR NUMBER
        MACOS_MAJOR=$(echo "$MACOS_VER" | cut -d. -f1)
        MACOS_MINOR=$(echo "$MACOS_VER" | cut -d. -f2)
        
        # add mocos specific flags
        MAC_FLAGS="-framework OpenCL"
        
        # Wenn Version 10.x und kleiner als 10.15, hänge das Linker-Flag an
        if [ "$MACOS_MAJOR" -eq 10 ] && [ "$MACOS_MINOR" -lt 15 ]; then
            MAC_FLAGS="$MAC_FLAGS" ### " -lstdc++fs"
        fi

        g++ -g -O0 src/*.cpp -o bin/OpenCL-Wrapper -I./ -std=c++17 -pthread -O -Wno-comment -I./src/OpenCL/include $MAC_FLAGS
        ;; 
	*Android) g++ -g -O0 src/*.cpp -o bin/OpenCL-Wrapper -I./ -std=c++17 -pthread -O -Wno-comment -I./src/OpenCL/include -L/system/vendor/lib64 -lOpenCL ;; # Android
        *Windows*|*MINGW*|*MSYS*)
                echo "🚀 Starte Standalone-Build mit RTools GCC..."
                # Pfad mit Vorwärts-Slashes erweitern und g++ zünden
                PATH="$PATH:/c/RBuildTools/4.4/x86_64-w64-mingw32.static.posix/bin"
                
                g++ -g -O0 src/*.cpp -o bin/OpenCL-Wrapper.exe \
                    -I./ -std=c++17 -pthread -Wno-comment \
                    -I./src/OpenCL/include -L./src/OpenCL/lib -lOpenCL
                
                if [ $? -eq 0 ]; then
                    echo "✅ Kompilierung erfolgreich! Starte Anwendung..."
                    ./bin/OpenCL-Wrapper.exe
                else
                    echo "❌ GCC Compilation fehlgeschlagen!"
                fi
                
                echo "🛑 [PAUSE] Drücken Sie [ENTER] um dieses Fenster zu schließen..."
                read
                ;;
	*       ) g++ -g -O0 src/*.cpp -o bin/OpenCL-Wrapper -I./ -std=c++17 -pthread -O -Wno-comment -I./src/OpenCL/include -L./src/OpenCL/lib -lOpenCL     ;; # Linux
esac

if [[ $? == 0 ]]; then bin/OpenCL-Wrapper "$@"; fi # run executable only if last compilation was successful
