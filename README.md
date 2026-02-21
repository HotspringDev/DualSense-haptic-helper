# DualSense-haptic-helper

A collection of testing utilities for the Sony DualSense controller, focused on haptic feedback behavior under Linux and Proton.

---

## Project Purpose

This repository stores experimental and diagnostic code used to research DualSense vibration and haptic feedback behavior.

The primary goals are:

* Test DualSense haptic functionality
* Analyze vibration behavior under native Linux
* Verify haptic passthrough under Proton
* Develop a reliable method to enable correct vibration behavior when running Windows games through Proton

A working solution allowing DualSense vibration to function correctly under Proton on Linux has been identified. The implementation details and related research materials are currently being prepared for public release.

---

## Included Tools

### Proton Test Utility

`ds5_test.exe`

* Windows executable
* Intended to be run through Proton
* Used to validate haptic passthrough behavior

---

### Native Linux Diagnostic Tool

`ds_verbose_test`

* Native Linux executable
* Uses ALSA for low-level audio/haptic interaction
* Provides verbose output for vibration diagnostics
* Useful for testing raw signal behavior

---

## Build Instructions

### Build everything

```shell
make
```

### Build Proton test binary only

```shell
make ds5_test.exe
```

### Build Linux haptic tester only

```shell
make ds_verbose_test
```

---

## Requirements

### Linux build

* GCC
* ALSA development libraries

### Windows (Proton) build

* MinGW-w64
* SDL2 development libraries
* Properly configured `SDL_PATH`

---

## Status

* DualSense vibration under Proton: method confirmed
* Tooling cleanup: in progress
* Documentation and technical notes: upcoming

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.
