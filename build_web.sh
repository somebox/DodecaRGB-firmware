#!/bin/bash
# build_web.sh

# Set default output directory if not provided
OUTPUT_DIR=${1:-"web"}
echo "Building web simulator to: $OUTPUT_DIR"

# --- ADDED: Clear Emscripten cache ---
# echo "Clearing Emscripten cache..."
# emcc --clear-cache
# echo "Cache cleared."
# --- END ADDED ---

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Check if the version file exists and is causing problems
if [ -f "./version" ]; then
  echo "Found problematic version file. Moving it temporarily..."
  mv ./version ./version.bak
fi

# --- ADDED: Find source files automatically --- 
echo "Finding source files..."
# CPP_FILES=$(find src lib/PixelTheater/src -name '*.cpp')
# Find files, excluding specific ones and the _old_scenes directory
CPP_FILES=$(find src lib/PixelTheater/src -name '*.cpp' \
                -not -path 'src/_old_scenes/*' \
                -not -name 'main.cpp' \
                -not -name 'web_build_proxy.cpp')
# You might want to add other filters here if needed, e.g.:
# -not -path '*/test/*'
echo "Using files:"
echo "$CPP_FILES"
# --- END ADDED ---

echo "Building WebGL LED Simulator with debugging options..."

# Compile the C++ code to WebAssembly with debugging information
emcc ${CPP_FILES} \
     -I"." \
     -I"lib" \
     -I"lib/PixelTheater/include" \
     -I"src" \
     -I"src/models" \
     -I"src/scenes" \
     -I".pio/libdeps/web/ArduinoEigen/ArduinoEigen" \
     -I"include" \
     -std=c++20 \
     -DPLATFORM_WEB \
     -DEMSCRIPTEN \
     -DDEBUG \
     -s WASM=1 \
     -s USE_WEBGL2=1 \
     -s FULL_ES3=1 \
     -s ALLOW_MEMORY_GROWTH=1 \
     -s EXPORTED_RUNTIME_METHODS='["UTF8ToString", "ccall", "cwrap", "getValue", "stackAlloc"]' \
     -s EXPORTED_FUNCTIONS='["_main", "_init_simulator", "_change_scene", "_get_num_scenes", "_set_brightness", "_get_brightness", "_update_rotation", "_reset_rotation", "_set_auto_rotation", "_set_zoom_level", "_show_benchmark_report", "_toggle_debug_mode", "_get_led_count", "_get_fps", "_set_led_size", "_get_led_size", "_set_atmosphere_intensity", "_get_atmosphere_intensity", "_set_show_mesh", "_get_show_mesh", "_set_mesh_opacity", "_get_mesh_opacity", "_log_message", "_get_current_time", "_update_ui_brightness", "_get_canvas_width", "_get_canvas_height", "_resizeCanvas", "_get_scene_parameters_json", "_update_scene_parameter_string", "_get_current_scene_metadata_json", "_free_string_memory", "_free", "_do_simulation_step"]' \
     -s INITIAL_MEMORY=32MB \
     -s MAXIMUM_MEMORY=128MB \
     -s ASSERTIONS=2 \
     -s SAFE_HEAP=1 \
     -s GL_DEBUG=1 \
     -s STACK_OVERFLOW_CHECK=2 \
     -g \
     -O0 \
     -fno-lto \
     --source-map-base ./ \
     -o "$OUTPUT_DIR/simulator.js"

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Build successful!"
  
  # Copy all web assets
  echo "Copying web assets..."
  
  # Copy HTML file
  if [ -f "web/index.html" ]; then
    echo "Copying index.html..."
    cp web/index.html "$OUTPUT_DIR/"
    
    # Fix paths in the HTML file if we're building for docs
    if [[ "$OUTPUT_DIR" == *"docs"* ]]; then
      echo "Fixing paths for documentation site..."
      # Add base tag to ensure relative paths work correctly
      sed -i.bak 's/<head>/<head>\n    <base href="\/simulator\/">/' "$OUTPUT_DIR/index.html"
      rm "$OUTPUT_DIR/index.html.bak"
    fi
  fi
  
  # Copy CSS files
  if [ -d "web/css" ]; then
    echo "Copying CSS files..."
    mkdir -p "$OUTPUT_DIR/css"
    cp -r web/css/* "$OUTPUT_DIR/css/"
  fi
  
  # Copy JavaScript files
  if [ -d "web/js" ]; then
    echo "Copying JavaScript files..."
    mkdir -p "$OUTPUT_DIR/js"
    cp -r web/js/* "$OUTPUT_DIR/js/"
  fi
  
  # Copy any other assets
  if [ -d "web/assets" ]; then
    echo "Copying additional assets..."
    mkdir -p "$OUTPUT_DIR/assets"
    cp -r web/assets/* "$OUTPUT_DIR/assets/"
  fi
  
  # Copy any other files in the web directory (excluding directories we've already copied)
  echo "Copying remaining files..."
  find web -maxdepth 1 -type f -not -name "index.html" -not -name "simulator.js" -not -name "simulator.wasm" -exec cp {} "$OUTPUT_DIR/" \;
  
else
  echo "Build failed!"
  exit 1
fi

# Restore the version file if we moved it
if [ -f "./version.bak" ]; then
  echo "Restoring version file..."
  mv ./version.bak ./version
fi

echo "Build complete. Files are in the $OUTPUT_DIR directory."
echo "Run a local server to test: cd $OUTPUT_DIR && python -m http.server"
