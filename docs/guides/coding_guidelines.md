---
category: Guidelines
generated: 2025-02-10 02:07
version: 2.8.2
---

# Project Guidelines

This serves as documentation for chosen best practices and standards that all developers should follow for this project.

## Overview

DodecaRGB is a project to enable the creation and orchestration of multiple colorful animations for a hardware-based LED object.
Animations are structured in a 360-degree environment, with support for different coordinate strategies: spherical, cartesian, and connected point graphs.
The PixelTheater library is used to define scenes, and the Point class is used to define the geometry of the LED object.

### The hardware

- A Teensy 4.1 microcontroller drives 1,248 individually addressable RGB LEDs arranged as follows:
  - 12 identical pentagon-shaped PCBs with 104 LEDs each
  - All PCBs assembled form a dodecahedron which is around 13cm in diameter
  - Each face's LEDs can be controlled independently using FastLED, addressible as one long strip
- IMU (accelerometer, gyroscope, magnetometer)
- Pushbutton (for advancing control or resetting)

### The main application (the "firmware")

- Developed in C++ using the Arduino framework
- main.cc is where setup() initializes and loop() runs the animation environment
- the PixelTheater library is where the animation system is defined
- Each scene is defined in a separate .cpp file in the /src/scenes directory
- The Point class in points.cpp defines every LED pixel and their X,Y,Z coordinates, and which face they belong to as well as polar coordinate functions. It can also find nearest neighbors and measure distances.

## Development Guidelines

### Code Style and Structure

- Write concise, idiomatic C++ code with accurate examples.
- Follow modern C++ conventions and best practices.
- Use object-oriented, procedural, or functional programming patterns as appropriate.
- Leverage STL and standard algorithms for collection operations.
- Use descriptive variable and method names (e.g., 'isUserSignedIn', 'calculateTotal').
- Structure files into headers (*.hpp) and implementation files (*.cpp) with logical separation of concerns.
- Before adding implementation to a .h file, first check the .cpp file to see if it's already implemented.

### Naming Conventions

- Use PascalCase for class names.
- Use camelCase for variable names and methods.
- Use SCREAMING_SNAKE_CASE for constants and macros.
- Prefix member variables with an underscore (e.g., `_speed`).
- Use namespaces to organize code logically.

### C++ Features Usage

- Prefer modern C++ features (e.g., auto, range-based loops, smart pointers).
- Use `std::unique_ptr` and `std::shared_ptr` for memory management.
- Prefer `std::optional`, `std::variant`, and `std::any` for type-safe alternatives.
- Use `constexpr` and `const` to optimize compile-time computations.
- Use `std::string_view` for read-only string operations to avoid unnecessary copies.
- Use `std::map` for key-value pairs. Prefer `std::unordered_map` for faster lookups, and use `at()` to access values over `[]` to avoid silent failures.

### Syntax and Formatting

- Follow a consistent coding style, suggest improvements for clarity and consistency.
- Place braces on the same line for control structures and methods.
- Use clear and consistent commenting practices.
- Add comments to code that represents important design decisions, or to explain complex code.

### Error Handling and Validation

- Use RAII for resource management to avoid memory leaks.
- Validate inputs at function boundaries.
- Log errors using Serial.printf()

### Performance Optimization

- Avoid unnecessary heap allocations; prefer stack-based objects where possible.
- Use `std::move` to enable move semantics and avoid copies.

### Key Conventions

- Use smart pointers over raw pointers for better memory safety.
- Use `enum class` for strongly typed enumerations.
- Separate interface from implementation in classes unless the class is very simple.
- Use templates and metaprogramming judiciously for generic solutions.

### Testing

- Aim to structure code in a way that is easy to test.
- Firmware can be hard to test as the arduino framework is not easy to mock.
- Aim to test modules or classes in isolation, and use the physical model to test the integration.

### Security

- Use secure coding practices to avoid vulnerabilities (e.g., buffer overflows, dangling pointers).
- Prefer `std::array` or `std::vector` over raw arrays.
- Avoid C-style casts; use `static_cast`, `dynamic_cast`, or `reinterpret_cast` when necessary.
- Enforce const-correctness in functions and member variables.

Follow the official ISO C++ standards and guidelines for best practices in modern C++ development.

## Utilities and Build System

The util/ directory contains python helper scripts for visualizing the physical model, generating the LED coordinates and other pre-calculation tasks, as well as importing and exporting different files used (such as CSV pick-and-place files, json, etc).

As each face of the dodecahedron is a pentagon that could be rotated in 5 different positions. This configuration is defined in the YAML model configuration using the `rotation` field for each face, where each face has a number 0-4 indicating one of 5 possible rotations in 72 degree increments (0° = 0, 72° = 1, 144° = 2, 216° = 3, 288° = 4).

## Documentation

- In code, write clear comments for classes, methods, and critical logic.
- Add coments to code where it helps document assumptions, constraints, and expected behavior.
- Add comments to code that has known problems, limitations, or could be the source of common errors.
- Aim to keep documentation short and factual, organized into logical sections, and explained with examples.
- In Markdown files, leave a blank line between headings and code blocks.
- After any major change is working, update the developer guide and tutorial creating_animations.md.
- After any major change, update the README.md with any new or changed information.
- After any major commit, update the VERSION variable in the main.cc file.

### Include Strategy

To ensure consistency and maintainability, follow these guidelines for `#include` directives:

1.  **Library User Perspective (e.g., `src/main.cpp`):
    *   **Goal:** Simple, single include for using the library.
    *   **Method:** Include the main library header: `#include "PixelTheater.h"`. PlatformIO automatically makes library headers available.
    *   **Do Not Include:** Avoid including internal library headers (e.g., `PixelTheater/core/scene.h`) directly.

2.  **Library Internal Perspective (`lib/PixelTheater/src/` and `lib/PixelTheater/include/PixelTheater/`):
    *   **Goal:** Manage dependencies within the library.
    *   **Method:**
        *   Source files (`*.cpp`) should include their corresponding header first (e.g., `native_platform.cpp` includes `"PixelTheater/platform/native_platform.h"`).
        *   Include other necessary library headers using the full path relative to the `lib/PixelTheater/include` directory (e.g., `#include "PixelTheater/core/crgb.h"`).
        *   Use forward declarations in headers (`*.h`) when possible to reduce compile times.
        *   Always use `#pragma once` in header files.

3.  **Testing Perspective (`test/`):
    *   **Goal:** Include library components under test and test infrastructure.
    *   **Method:**
        *   **Library Headers:** Include using the full library path (e.g., `#include "PixelTheater/scene.h"`).
        *   **Test Framework:** Include using the standard path (e.g., `#include <doctest/doctest.h>`).
        *   **Test Helpers/Fixtures:** Include using relative paths from the test file being written (e.g., from `test/test_native/core/test_file.cpp` include a fixture in `test/fixtures/models/` using `#include "../../fixtures/models/my_fixture.h"`).
