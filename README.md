# Blacklist Controller Fix

**PS5 DualSense and native Xbox controllers, with automatic controller-appropriate button icons.**

Version 1.2.0 passed user-confirmed Xbox USB and PS5 Bluetooth tests on the same DLL: menus, analog movement and the correct icons work. DualSense USB was confirmed on v1.1.x; it has not been separately repeated on the v1.2.0 DLL.

Use a PlayStation 5 DualSense over USB or Bluetooth with Blacklist's actual gamepad controls on Windows, including analog movement and controller menus, when launching through Ubisoft Connect.

A native gamepad compatibility mod for **Splinter Cell Blacklist** on Windows, launched through Ubisoft Connect.

## Easy install

1. Download [**Blacklist-Controller-Fix.zip**](https://github.com/La2o/blacklist-controller-fix/releases/latest/download/Blacklist-Controller-Fix.zip).
2. Close Blacklist and extract the ZIP into the game folder containing `Blacklist_Launcher.exe`.
3. Connect your DualSense or Xbox controller and launch the game through Ubisoft Connect.

The previous `splinter-cell-blacklist-dualsense` repository URL redirects here. Existing legacy-named release downloads remain available.

The files should land at `src/SYSTEM/xinput1_3.dll` and `src/SYSTEM/SDL3.dll`. No installer, driver or configuration change is required. If either DLL already belongs to another mod, back it up before replacement. DualSense uses PlayStation prompts, using the game's existing PS3-style textures: Cross, Circle, Square, Triangle, L1/R1 and L2/R2. Start/Select retain their PS3 artwork. Native Xbox input keeps Xbox prompts.

## Remove

Close the game and delete those two DLLs. Restore any DLLs you backed up from another mod. The optional `Blacklist-controller.log` can also be deleted. Original game executables and configuration are never modified on disk.

## How it works

Blacklist's game executables are 32-bit and import XInput 1.3 by ordinal: GetState (2), SetState (3) and GetCapabilities (4). A local x86 proxy uses SDL's DualSense backend to supply those APIs with analog controller state and pass ordinary XInput devices through to the system implementation.

The game also uses an Xbox-specific WMI identity check before polling XInput. The mod recognizes DualSense product GUIDs at this check. It then prioritizes the DualSense in the game's DirectInput enumeration list. This corrects a confirmed device-order problem where a Keychron Q6 HE keyboard's game-controller interface took the first slot. The keyboard and other device entries remain available. Version 1.2.0 also prioritizes native XInput controllers recognized by the original WMI classifier, preserving real-controller enumeration order. Peripheral interfaces cannot stop enumeration before a real controller is found; the game table remains limited to four entries.

These two hooks operate only in process memory. They check executable name, timestamp, image size, instruction signatures and the enumeration counter address before applying. Unknown builds are refused. No DRM or Ubisoft Connect component is patched.

With a DualSense connected, slot 0 is reserved for it and supersedes an ordinary XInput controller at slot 0. Slots 1–3 pass through. Without a DualSense, the XInput APIs pass through normally. Only Sony VID 054c/PID 0ce6 and Edge PID 0df2 are selected. Edge support has not been tested.

## PlayStation button prompts

Version 1.1.0 intercepts the verified archive lookup function and redirects only Flash UI `ICON_IK_JOY1.dds` through `ICON_IK_JOY16.dds` requests to the existing `SC6IconsPS3` assets. It covers matching assets used by menus and HUD. Already-PS3 paths, keyboard artwork, unrelated textures and invalid button names are left alone. If an archive has no replacement, its original lookup runs. Game archives stay unchanged, and no Ubisoft textures are distributed.

The hook validates the lookup function signature for the same supported executable revisions before copying ten whole prologue bytes to a trampoline. Diagnostic logs identify the installed hook and the bitmask of replacement buttons actually loaded. Unknown builds refuse the hook.

Version 1.2.0 selects prompt style when button textures load: a detected DualSense uses PlayStation textures, and native Xbox input keeps original Xbox textures. Close and restart the game when switching controller families; texture caches are not refreshed live. If both families are connected, DualSense retains the existing slot-0 priority. Simultaneous multi-controller play is untested.

## Tested status

- **User-confirmed in game:** native controller menus and analog movement, Bluetooth DualSense, DX11, Ubisoft Connect launch. For v1.1.0, the user also confirmed PlayStation icons appear and inputs still work after checking menus/HUD.
- **Automated checks:** x86 imports/exports and ABI, button and analog conversion, physical stick endpoints and Cross presses, loading inside the game, identity and ordering hooks, correct slot selection, unchanged original file hashes, prompt-path filtering/fallback eligibility, and all 16 replacement button textures loaded inside the game.
- **USB verification:** Windows detected a USB DualSense with no Bluetooth connection present. The installed v1.1.0 DLL passed the diagnostic; both triggers independently reached 0–255 and both sticks produced analog input. The user then confirmed the USB controller works in game.
- **Xbox USB v1.2.0 test:** user confirmed native controls, analog movement and Xbox icons work in menus and gameplay. Windows native capability forwarding, controller prioritization, capacity limits and prompt filtering passed automated checks.
- **PS5 Bluetooth v1.2.0 regression:** user confirmed controls, analog movement and PlayStation icons work in menus and gameplay on the new binary.
- **Not tested on v1.2.0:** PS5 USB regression, Xbox Bluetooth/wireless adapter, DX9 gameplay, DualSense Edge, physical rumble sensation, disconnect/reconnect, simultaneous multi-controller gameplay, and multiplayer. USB rumble API calls returned success, but this alone does not verify felt vibration.

Version 1.2.0 updates the compatibility DLL. Existing users should close the game and replace the two included DLLs to receive native Xbox ordering and automatic icon selection. See [COMPATIBILITY.md](COMPATIBILITY.md) for the verification scope.

Rumble is implemented through SDL but has not been physically verified. There are no adaptive-trigger effects, gyro/touchpad mappings, headset audio bridge. PlayStation glyph replacement is included from v1.1.0. The optional XInputGetKeystroke API has no Sony event queue; Blacklist does not import it.

SDL enhanced reports are enabled. If a separate DirectInput-only app stops recognizing the controller afterward, power the controller off and on to reset its Bluetooth report mode. See [SDL's documentation](https://wiki.libsdl.org/SDL3/SDL_HINT_JOYSTICK_ENHANCED_REPORTS).

## Supported executable revisions

| Executable | SHA-256 |
| --- | --- |
| Blacklist_DX11_game.exe | `c52b3d0927591e477424f389ff0b1314a300938e19ce61a7b0a7bc09f81c2c89` |
| Blacklist_game.exe | `7fcd3a18d4dcc692719984b07268bd42764108e7df37049f1490f20929f9925d` |

The DX9 revision was inspected but has not been tested in gameplay. Runtime guards check PE metadata and code signatures rather than computing these whole-file hashes.

## Build from source

Windows with Python 3 and PowerShell:

```powershell
python download_dependencies.py
.\build.ps1
.\build\controller-check.exe 2
```

Downloads are pinned to Zig 0.17.0 and SDL 3.4.18 and checked against the recorded upstream SHA-256 digests. Built DLLs are written to `build/`. The diagnostic verifies mappings and ABI; its result is not an in-game test. Run it with a longer duration, such as `30`, while moving the controller to observe live XInput values.

## Third-party software

The release includes the unmodified SDL 3.4.18 x86 runtime. Its zlib license is in [third_party/SDL3-LICENSE.txt](third_party/SDL3-LICENSE.txt). No Ubisoft game files, personal configuration, backups or runtime logs are distributed.
