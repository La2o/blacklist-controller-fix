## v1.1.0 — PlayStation Button Icons

Adds PlayStation button prompts to the working PS5 Bluetooth controller mod. The game uses its own PS3-style Cross, Circle, Square, Triangle, shoulder, trigger, stick and D-pad artwork. Start/Select retain their original PS3 symbols.

Download **Blacklist-DualSense-DropIn.zip**, close Blacklist, and extract it into the folder containing `Blacklist_Launcher.exe`. Launch through Ubisoft Connect. Existing v1.0.0 users can replace the same two DLLs while the game is closed.

**User-confirmed in game:** PlayStation icons appear and inputs still work on the Bluetooth DualSense/DX11 setup. The prior native menus and analog movement remain working. **Automated verification:** prompt-path filters, x86 ABI/input checks, signature-checked hook installation, all 16 button replacement textures loaded, and unchanged original executable/configuration hashes.

Textures are reused from the local game archive, with original-asset fallback. No game assets are distributed and no archive is modified. USB, DX9 gameplay, rumble, reconnect, additional controller models and exhaustive coverage of every game screen remain untested.

Remove by closing the game and deleting `src/SYSTEM/xinput1_3.dll` and `src/SYSTEM/SDL3.dll`; restore any preexisting DLLs from your backup. To revert only the icon update, reinstall the v1.0.0 drop-in ZIP.
