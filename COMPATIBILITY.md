# Verified compatibility

| Setup | Verification version | Automated evidence | User-confirmed in-game result |
| --- | --- | --- | --- |
| DualSense Bluetooth, DX11, Ubisoft Connect | v1.2.0, same DLL as Xbox test | Connection isolated from Xbox/USB; ABI diagnostic; correct Sony slot selection; PlayStation style selected; all 16 button textures loaded | Controls, analog movement and PlayStation icons work in menus and gameplay |
| Native Xbox controller USB, DX11, Ubisoft Connect | v1.2.0 | Native XInput slot 0; exact native capability forwarding; Xbox prioritized ahead of Keychron; game XInput flag set; Xbox prompt style selected | Controls, analog movement and Xbox icons work in menus and gameplay |
| DualSense USB, DX11, Ubisoft Connect | v1.1.x; not repeated on v1.2.0 | USB present with Bluetooth absent; ABI diagnostic; analog stick input; independent L2/R2 0–255 | User confirmed USB works and requested release |

## What changed in v1.2.0

The old build failed the Xbox in-game test because a Keychron Q6 HE game-controller interface occupied the first DirectInput slot. The ordering hook now prioritizes native XInput controllers recognized by the original WMI classifier, alongside the existing DualSense recognition. It preserves real-controller order, keeps the game arrays at four entries and continues past peripheral interfaces so a real controller is not missed. Stable ordering and full-table eviction are covered by synthetic checks.

The prompt hook selects PS3-style PlayStation textures when a DualSense is detected; otherwise it leaves original Xbox textures intact. Close and restart Blacklist when switching controller families because textures are cached. If both are connected, DualSense retains the existing XInput slot-0 priority. Simultaneous multi-controller play is not verified.

Windows XInput 1.3 reports capability Type=0 for the tested native Xbox device. The diagnostic now checks exact system capability forwarding in that case instead of assuming synthetic DualSense Type=1. This is a diagnostic correction, not a modified native capability report.

## Limits

USB rumble API calls returned success in the earlier DualSense test; felt vibration was not separately confirmed. Full physical Xbox axis/trigger travel was not established by its neutral/drift diagnostic capture; analog movement and controls were subsequently user-confirmed in game.

PS5 USB regression on v1.2.0, Xbox Bluetooth/wireless adapter, DX9 gameplay, DualSense Edge, physical rumble, hotplug, simultaneous-controller play, multiplayer and every possible screen remain untested. PS3-style icons include older Start/Select symbols.

No raw hardware serials, personal configuration, game binaries, extracted assets, backups or session logs are distributed.
