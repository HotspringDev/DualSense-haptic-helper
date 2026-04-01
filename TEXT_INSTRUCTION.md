**Note: This file is directly copied from my reddit post. For more information, please refer to the original post.**

# DualSense Adaptive Triggers & Haptics Working in Arknights: Endfield (Linux / Arch / Proton)

I’ve been playing Arknights: Endfield on Arch Linux and noticed that DualSense adaptive triggers and haptic feedback do not work out of the box under Proton.

After investigating PipeWire, WirePlumber, and how the Sony SDK initializes the controller, I managed to get adaptive triggers and full haptics working reliably.

Video guide (Mandarin audio, English subtitles available): [https://www.youtube.com/watch?v=lcw-4S\_CYFQ&t=6s](https://www.youtube.com/watch?v=lcw-4S_CYFQ&t=6s)

GitHub repository: [https://github.com/HotspringDev/DualSense-haptic-helper](https://github.com/HotspringDev/DualSense-haptic-helper)

**UPDATE:**

Thanks to u/Existing-Help-3187 for pointing out the root issue.

The connection problem seems to stem from a regression in alsa-ucm-conf (version 1.2.15+). While my guide below provides a WirePlumber-based workaround (which is safe and survives updates), you can also choose to fix it at the system level by downgrading alsa-ucm or editing the ALSA config files directly.

Solution: [https://www.reddit.com/r/linux\_gaming/comments/1rco7c1/comment/o73hfuv/](https://www.reddit.com/r/linux_gaming/comments/1rco7c1/comment/o73hfuv/)

# Technical Background

DualSense haptics on PC are driven by audio signals. On Linux (PipeWire/WirePlumber), there are two main issues.

# 1. Incorrect Channel Mapping

By default, the controller may appear as:

* Mono output (under "Default")
* Or 4 raw channels (0,1,2,3) under “Pro Audio”

However, games using the Sony SDK expect a proper quadraphonic layout:

FL, FR, RL, RR

If the audio positions are not mapped correctly, haptic signals are sent to invalid channels and vibration does not occur.

# 2. SDK String Matching

The Sony SDK used by many games explicitly checks for the device name:

"Wireless Controller"

If the exposed PipeWire/WirePlumber node name does not match this string, haptics may fail to initialize or behave inconsistently.

So the device must be renamed accordingly.

# The Fix

# 1. Lutris Environment Variables

Add these variables to your game configuration:

* `PROTON_ENABLE_HIDRAW=1`
* `WINE_HAPTICS=1`

This allows Proton to access the raw HID device and enables haptic support inside Wine.

# 2. Install Proper Udev Rules

Make sure the following package is installed:

`game-devices-udev`

Without correct permissions, Proton may not be able to access the controller properly.

# 3. WirePlumber Channel Mapping (Core Fix)

Create the file:

`~/.config/wireplumber/wireplumber.conf.d/51-dualsense.conf`

With the following content:

    monitor.alsa.rules = [
      {
        matches = [
          {
            "device.name": "~alsa_card.usb-Sony_Interactive_Entertainment_DualSense_Wireless_Controller.*"
          }
        ]
        actions = {
          "update-props": {
            "device.description": "DualSense Wireless Controller",
            "device.nick": "DualSense"
          }
        }
      },
      {
        matches = [
          {
            "node.name": "~alsa_output.usb-Sony_Interactive_Entertainment_DualSense_Wireless_Controller.*"
          }
        ]
        actions = {
          "update-props": {
            "node.description": "Wireless Controller",
            "node.nick": "Wireless Controller",
            "audio.channels": 4,
            "audio.position": [ "FL", "FR", "RL", "RR" ],
            "channelmix.normalize": false,
            "priority.session": 2500,
            "priority.driver": 2500,
            "node.pause-on-idle": false
          }
        }
      },
      {
        matches = [
          {
            "node.name": "~alsa_input.usb-Sony_Interactive_Entertainment_DualSense_Wireless_Controller.*"
          }
        ]
        actions = {
          "update-props": {
            "node.description": "Wireless Controller Mic",
            "node.nick": "Wireless Controller Mic"
          }
        }
      }
    ]

After saving, restart PipeWire and WirePlumber:

`systemctl --user restart pipewire wireplumber`

# Testing Utilities

To help distinguish between system-level and game-level issues, I wrote two small tools:

* `ds5_test.exe` — tests haptic signal through Proton
* `ds_verbose_test` (native Linux) — tests direct audio-to-vibration

GitHub repository: [https://github.com/HotspringDev/DualSense-haptic-helper](https://github.com/HotspringDev/DualSense-haptic-helper)

# Tested Environment

OS: Arch Linux Runner: Lutris + UMU Launcher (Proton) Audio: PipeWire + WirePlumber

This was verified specifically with Arknights: Endfield, but the WirePlumber mapping approach should work for other games that use the DualSense haptic engine through audio.

If anyone runs into edge cases or wants to dig deeper into the PipeWire side, feel free to discuss.

(btw, I am not a native English speaker, so my language may be very neutral and may look like it was written by AI, but it was actually all typed by hand.)
