# DodecaRGB v2 Development

- There's a guide for developing animations in [creating animations](creating_animations.md).
- Development standards and best practices documented in [Coding Guidelines](coding_guidelines.md).
- The project has Python utilities for generating LED coordinates and visualizing the 3D model. For info about using them , please refer to the [Utilities README](util/README.md).

## C++ Firmware

We're using PlatformIO to build the firmware and manage dependencies. Just clone the repo and open the project in VSCode/Cursor/whatever.

### Configurating your dodecaRGB model

If for some reason you need to change the orientation settings of the PCB, you will need to change the `side_rotation` array in `util/dodeca_core.py` and re-run the Python utilities to generate the new LED coordinates, and update the `points.h` file.

To configure the animation settings, look at the bottom of the `setup()` function in `src/main.cpp`, there you will find how to disable the slideshow mode, or change the order of animations. Currently, the pushbutton is configured to advance to the next animation in the list.

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

