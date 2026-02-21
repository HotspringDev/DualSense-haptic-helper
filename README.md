# DualSense-haptic-helper

A collection of testing utilities for the Sony DualSense controller, focused on haptic feedback behavior under Linux and Proton.

---

## Project Purpose

This repository stores experimental and diagnostic code used to research DualSense vibration and haptic feedback behavior.

The primary goals are:

* Test DualSense haptic functionality (Standard Rumble & VCM Haptics).
* Analyze vibration behavior under native Linux.
* Verify haptic passthrough under Proton/Wine.
* Provide a reliable environment for testing variables that enable correct vibration behavior in Windows games.

---

## Included Tools

### Proton Test Utility (`ds5_test.exe`)

* **Type**: Windows Executable (Cross-compiled with MinGW).
* **Usage**: Run via Proton/Steam.
* **Features**:
  * Separate Left/Right motor rumble tests.
  * 4-channel audio haptic test (targeting Channels 2/3).
  * Verbose device logging.

### Native Linux Diagnostic Tool (`ds_verbose_test`)

* **Type**: Native Linux Executable.
* **Usage**: Direct execution.
* **Features**: Uses ALSA/HIDAPI for low-level interaction and verbose signal diagnostics.

---

## Build Instructions

### Prerequisites

#### Linux Build

* `gcc`
* `libasound2-dev` (ALSA development files)

#### Windows (Proton) Build

* `mingw-w64`
* `SDL2` Development Libraries (MinGW version)

### SDL2 Setup for Cross-Compilation

To compile `ds5_test.exe`, you must download the SDL2 MinGW development package:

1. Download `SDL2-devel-2.30.x-mingw.tar.gz` from the [SDL2 Releases](https://github.com/libsdl-org/SDL/releases).
2. Extract the archive.
3. Export the path to the 64-bit MinGW directory:

   ```shell
   export SDL_PATH=/path/to/SDL2-2.30.0/x86_64-w64-mingw32
   ```

### Building

```shell
# Build all tools
make

# Build Proton test binary only
make ds5_test.exe

# Build Linux haptic tester only
make ds_verbose_test
```

*Note: The build process for `ds5_test.exe` will automatically attempt to copy `SDL2.dll` to the output directory.*

---

## Proton Configuration Guide

To ensure `ds5_test.exe` (or games) can access DualSense haptics, verify the following:

### 1. Steam Settings

* **Disable Steam Input**: Right-click game -> Properties -> Controller -> Disable Steam Input. Steam Input often masks the DualSense as an Xbox controller, breaking 4-channel audio support.

### 2. Environment Variables

Common variables to test for haptic passthrough:

* `PROTON_ENABLE_HIDRAW=1` (Enables direct HID access)
* `SDL_JOYSTICK_HIDAPI_PS5=1` (Forces SDL to use its internal HIDAPI driver)
* `SDL_ALSA_CHANNELS=4` (Forces 4-channel mode for ALSA backends)

### 3. System Audio Setup

DualSense haptics rely on audio channels 2 and 3.

* Ensure the DualSense is recognized as a **Quadraphonic (4.0) Surround** device in your system sound settings (e.g., `pavucontrol`).
* If the haptic test plays sound through your main speakers, the application is not correctly targeting the DualSense audio device. Check the verbose log for the detected device list.

---

## Status

* **DualSense vibration under Proton**: Method confirmed via `ds5_test.exe`.
* **Tooling cleanup**: In progress.
* **Documentation**: Technical notes on VCM frequency response upcoming.

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.
