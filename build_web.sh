#!/bin/bash
# build_web.sh

# Set default output directory if not provided
OUTPUT_DIR=${1:-"web"}
echo "Building web simulator to: $OUTPUT_DIR"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Check if the version file exists and is causing problems
if [ -f "./version" ]; then
  echo "Found problematic version file. Moving it temporarily..."
  mv ./version ./version.bak
fi

echo "Building WebGL LED Simulator with debugging options..."

# Compile the C++ code to WebAssembly with debugging information
emcc src/web_simulator.cpp \
     src/benchmark.cpp \
     src/math_provider.cpp \
     lib/PixelTheater/src/core/color.cpp \
     lib/PixelTheater/src/core/crgb.cpp \
     lib/PixelTheater/src/model/point.cpp \
     lib/PixelTheater/src/palette.cpp \
     lib/PixelTheater/src/params/handlers/flag_handler.cpp \
     lib/PixelTheater/src/params/handlers/type_handler.cpp \
     lib/PixelTheater/src/params/param_types.cpp \
     lib/PixelTheater/src/platform/web_platform.cpp \
     lib/PixelTheater/src/settings.cpp \
     lib/PixelTheater/src/stage.cpp \
     lib/PixelTheater/src/platform/webgl/util.cpp \
     lib/PixelTheater/src/platform/webgl/camera.cpp \
     lib/PixelTheater/src/platform/webgl/mesh.cpp \
     lib/PixelTheater/src/platform/webgl/shaders.cpp \
     lib/PixelTheater/src/platform/webgl/renderer.cpp \
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
     --bind \
     -s EXPORTED_RUNTIME_METHODS='["UTF8ToString", "ccall", "cwrap"]' \
     -s EXPORTED_FUNCTIONS='["_main", "_change_scene", "_get_scene_count", "_get_scene_name", "_set_brightness", "_show_benchmark_report", "_toggle_debug_mode", "_print_model_info", "_get_current_time", "_update_ui_fps", "_update_ui_brightness", "_get_canvas_width", "_get_canvas_height"]' \
     -s INITIAL_MEMORY=32MB \
     -s MAXIMUM_MEMORY=128MB \
     -s ASSERTIONS=2 \
     -s SAFE_HEAP=1 \
     -s GL_DEBUG=1 \
     -s STACK_OVERFLOW_CHECK=2 \
     -g \
     -O0 \
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
