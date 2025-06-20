---
category: Development
generated: 2025-02-10 02:07
version: 2.8.2
---

# DodecaRGB v2 Development

- There's a guide for developing animations in [creating animations](creating_animations.md).
- Development standards and best practices documented in [Coding Guidelines](coding_guidelines.md).
- The project has Python utilities for generating LED coordinates and visualizing the 3D model. For info about using them , please refer to the [Utilities README](util/README.md).

## C++ Firmware

We're using PlatformIO to build the firmware and manage dependencies. Just clone the repo and open the project in VSCode/Cursor/whatever.

### Configuring your dodecaRGB model

If you need to change the orientation settings of the PCB faces, you should edit the `rotation` values in your model's YAML configuration file (e.g., `src/models/DodecaRGBv2_1/model.yaml`). Each face can have a `rotation` value from 0-4, representing 72-degree increments (0° = 0, 72° = 1, 144° = 2, 216° = 3, 288° = 4). After making changes, re-run the model generation utility:

```bash
python util/generate_model.py -d src/models/YourModel -y
```

This will regenerate the `model.h` file with the updated LED coordinates and geometric data.

To configure the animation settings, look at the bottom of the `setup()` function in `src/main.cpp`, there you will find how to disable the slideshow mode, or change the order of animations. Currently, the pushbutton is configured to advance to the next animation in the list.

## Testing and Debugging Animations

### Using the Web Simulator

The web simulator provides a browser-based environment for testing animations without hardware:

1. Build for web: `pio run -e web_simulator`
2. Open `web/index.html` in browser
3. Adjust parameters in real-time
4. Verify visual effects before hardware deployment

### Debugging Techniques

**Logging in Scenes:**
```cpp
void MyScene::tick() {
    Scene::tick();
    
    // Log parameter changes
    static float lastSpeed = -1.0f;
    float speed = settings["speed"];
    if (speed != lastSpeed) {
        logInfo("Speed changed to: %.2f", speed);
        lastSpeed = speed;
    }
}
```

**Visual Debugging:**
```cpp
// Highlight specific LEDs for debugging
if (settings["debug_mode"]) {
    // Show face boundaries
    for (size_t face = 0; face < model().faceCount(); ++face) {
        auto faceLeds = model().face(face).leds();
        CRGB debugColor = CHSV(face * 30, 255, 100);
        for (auto& led : faceLeds) {
            led = debugColor;
        }
    }
} else {
    // Normal animation
    // ...
}
```

## Python Environment

If you just want to make animations, you can skip the Python environment setup.

1. **Install Required Tools**

```bash
# Install Python 3.12 and direnv
brew install python@3.12 direnv tcl-tk
```

2. **Configure direnv**

Add this to your `~/.zshrc` or `~/.bashrc`:

```bash
# Add direnv hook
eval "$(direnv hook zsh)"  # or bash if you use bash
```

Restart your terminal or run: `source ~/.zshrc`

3. **Project Setup**

In your project root directory:

```bash
# Copy example configuration files
cp .env.example .env
cp .envrc.example .envrc

# Create and activate virtual environment
/opt/homebrew/opt/python@3.12/bin/python3.12 -m venv venv
source venv/bin/activate

# Allow direnv for this directory
direnv allow .

# Install requirements
pip install -r util/requirements.txt
```

Now whenever you cd into the project directory:

- The correct Python version (3.12) will be activated
- The virtual environment will be activated automatically
- PYTHONPATH will be set correctly

To verify your setup:

```bash
python --version  # Should show Python 3.12.x
echo $PYTHONPATH  # Should show your project root
pip list  # Should show installed packages
```

### Testing

Tests are organized into two environments:

```bash
# Run native cpp tests
pio test -e native

# Run python tests (from root of repo)
python -m util.tests.run_tests
```

The [fireworks.yaml](utils/test/fixtures/fireworks.yaml) example file is used to generate a C++ code for scene parameters which should be written to the `test/fixtures/` directory. Running the python tests will generate the files needed for the C++ tests.

The C++ codebase uses the doctest framework for testing. PlatformIO's toolchains are used for the C++ tests. The native test environment only tests the library code, not the hardware. That means the arduino framework and FastLED are mocked out.

Th Python tests are located in the `util/tests` directory. The `run_tests.py` script runs all the tests and formats the results.

### Troubleshooting

If you see Python version mismatches when opening a new terminal:

1. Ensure direnv is properly hooked into your shell
2. Check that `.envrc` and `.env` are in the project root
3. Run `direnv allow .` in the project directory
4. Verify with `which python` that it points to your venv

If Tkinter issues occur:

```bash
# Test Tkinter
python -c "import tkinter; tkinter._test()"

# If needed, reinstall Python with Tk support
brew unlink python-tk
brew uninstall python-tk@3.12
brew install python@3.12 --with-tcl-tk
```

<<<<<<< HEAD:Development.md
=======
## Further Inspiration

- [What I learned from making a dodecahedron](https://www.youtube.com/watch?v=pcV9YAWSDRE) by the fabulous Dave Darko - honestly the video that got me addicted to this project
- [Designing a Dodecahedron](https://www.youtube.com/watch?v=vR6oae0s6_M) in OnShape
- [Geometry - Platonic Solids](https://www.cosmic-core.org/free/article-42-geometry-platonic-solids-part-3-spherical-stereographic-solids/) - great for learning about the geometry of the dodecahedron
-
<<<<<<< HEAD
>>>>>>> a40f713 (reorganized docs into /docs folder, added build step and python script to compile headers and frontmatter in markdown, added script to update VERSION strings in project and docs):docs/development.md
=======
>>>>>>> 10248fd (fix doc_builder.py script to take parameters and check file dates against headers (avoiding needless modifications))
