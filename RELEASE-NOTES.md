## v1.2.0 — Blacklist Controller Fix

Fixes native Xbox controller detection when a keyboard's game-controller interface takes the first slot. Controller ordering now prioritizes native XInput pads as well as DualSense, preserves controller order and keeps the game's four-entry limit.

**Automatic icons:** DualSense uses PlayStation artwork; native Xbox input keeps Xbox artwork. Restart Blacklist when switching controller families because button textures are cached.

**User-confirmed on the same v1.2.0 DLL:** Xbox USB controls, analog movement and Xbox icons; PS5 Bluetooth controls, analog movement and PlayStation icons, in menus and gameplay. Automated checks verified native capability forwarding, GUID ordering/capacity, correct game slot selection, controller-specific prompt selection and all 16 PlayStation texture redirects.

PS5 USB was tested on v1.1.x and has not been separately repeated on v1.2.0. Xbox Bluetooth/wireless adapter, physical rumble, hotplug, multiple simultaneous controllers, DX9 gameplay, Edge and multiplayer remain unverified.

**Install:** Close the game, extract **Blacklist-Controller-Fix.zip** into the folder containing `Blacklist_Launcher.exe`, then launch through Ubisoft Connect. Existing users replace the same two DLLs. Back up preexisting files from other mods first. Legacy-named archives remain available for existing download links. The project was renamed from splinter-cell-blacklist-dualsense; the mod DLLs are unchanged by this rename.

**Remove:** Close the game and delete `src/SYSTEM/xinput1_3.dll` and `src/SYSTEM/SDL3.dll`; restore other mod files from your backup if applicable. Reinstall v1.1.1 to roll back. No original game executable or archive is changed on disk, and no Ubisoft assets are distributed.
