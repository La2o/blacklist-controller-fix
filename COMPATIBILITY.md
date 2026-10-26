# Verified compatibility

| Setup | Automated evidence | In-game evidence |
| --- | --- | --- |
| DualSense Bluetooth, DX11, Ubisoft Connect | Physical analog/button input, XInput ABI, game DLL loading and selection, all 16 PlayStation button textures loaded | User confirmed controller menus, analog movement, PlayStation icons and working inputs |
| DualSense USB, DX11, Ubisoft Connect | USB device present with Bluetooth absent; ABI diagnostic passed; both sticks returned analog values; L2 and R2 independently reached 0–255; prompt hook and DualSense recognition initialized inside the game | User confirmed it works and requested release |

USB was tested using the unchanged v1.1.0 DLLs. Version 1.1.1 updates documentation and packaging only. No controller code change was required.

Two short USB rumble requests and a stop request returned API success; physical vibration was not separately confirmed. One stick direction did not reach its maximum endpoint during the USB capture, so full travel on every axis is not claimed. Button presses were not captured in the USB diagnostic; the subsequent in-game user confirmation is separate evidence.

DX9 gameplay, DualSense Edge, other controllers, disconnect/reconnect, multiplayer, and exhaustive coverage of every screen have not been verified. Icons reuse the game's PS3 artwork, including its Start/Select symbols.

No raw device identifiers, personal configuration, game binaries, extracted game assets, backups or session logs are included here.
