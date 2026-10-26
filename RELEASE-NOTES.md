## v1.1.1 — PS5 USB + Bluetooth Supported

USB is now tested in addition to Bluetooth. The user confirmed the DualSense works in Blacklist over USB through Ubisoft Connect (DX11), using the existing PlayStation-icon build.

**Same DLLs as v1.1.0.** This release updates the package instructions and compatibility documentation. Existing v1.1.0 users do not need to replace their files.

**Automated USB checks:** USB device present with Bluetooth disconnected; XInput ABI/mapping diagnostic passed; analog stick input; L2/R2 independently reached 0–255; game initialized the controller and PlayStation texture hooks. **In-game testing:** user confirmed USB works. Rumble API calls succeeded, but physical vibration was not separately confirmed.

**Install:** Close Blacklist, extract **Blacklist-DualSense-DropIn.zip** into the folder containing `Blacklist_Launcher.exe`, then launch through Ubisoft Connect. No installer, driver or configuration edit is required. Existing files from another mod should be backed up before replacement.

**Remove:** Close the game and delete `src/SYSTEM/xinput1_3.dll` and `src/SYSTEM/SDL3.dll`, restoring preexisting files from your backup if applicable. Game executables, configuration and archives are not modified on disk.

PS3-style PlayStation button prompts are included; Start/Select retain older symbols. DX9 gameplay, Edge/other controllers, physical rumble, reconnect and multiplayer remain unverified. See the repository compatibility notes for the precise testing scope.
