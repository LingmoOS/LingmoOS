# lingmo-oobe

First-boot Out-Of-Box Experience (OOBE) wizard for Lingmo OS.

Runs on the first boot after installation to create a user account, set the hostname, configure SDDM autologin, and clean up the live session user — all before the desktop environment starts.

## Architecture

```
Power On
  │
  ├── BIOS/UEFI
  ├── Bootloader (GRUB)
  ├── Kernel
  └── systemd
        │
        ├── firstboot.service (if /etc/.firstboot-complete missing)
        │     └─ Write SDDM autologin for liveuser + OOBE session
        │
        └── SDDM
              │
              ├── [First Boot] Autologin liveuser → OOBE Session
              │     └─ lingmo-oobe (fullscreen card UI)
              │           ├─ Create user account
              │           ├─ Set hostname
              │           ├─ Write SDDM autologin (real user)
              │           ├─ Delete liveuser
              │           ├─ Mark complete (/etc/.firstboot-complete)
              │           └─ Reboot
              │
              └── [Subsequent Boots] Autologin real user → Desktop
```

## Pages

| Page       | Description                                |
|------------|--------------------------------------------|
| Welcome    | Branding splash with logo and description  |
| CreateUser | Input for new username and password        |
| Hostname   | Set the machine hostname                   |
| Confirm    | Review choices before applying             |
| Install    | Progress bar while applying settings       |
| Finished   | Setup complete, reboot button              |

## Boot Sequence

| Boot # | Action                                          |
|--------|-------------------------------------------------|
| 1st    | `firstboot.service` writes SDDM first-boot cfg  |
|        | SDDM autologins `liveuser` → `lingmo-oobe` runs |
|        | OOBE creates real user, writes DM config         |
|        | `liveuser` deleted, flag file created            |
|        | System reboots                                   |
| 2nd+   | SDDM autologins real user → normal desktop       |

## Dependencies

- Qt6 (Widgets)
- CMake ≥ 3.16
- C++17 compiler
- SDDM or LightDM (runtime)
- polkit (for passwordless pkexec on first boot)

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
sudo cmake --install .
```

To build a Debian package:

```bash
dpkg-buildpackage -uc -us -b
```

## Install Layout

```
/usr/bin/lingmo-oobe                     – Main binary
/usr/lib/lingmo-oobe/oobe.sh             – Launcher script
/usr/share/xsessions/lingmo-oobe.desktop – X11 session entry
/usr/share/wayland-sessions/lingmo-oobe.desktop – Wayland session entry
/etc/xdg/autostart/oobe.desktop          – Fallback autostart
/lib/systemd/system/firstboot.service    – Pre-DM setup service
/etc/polkit-1/rules.d/10-lingmo-oobe.rules – Polkit rule for liveuser
```

## Files

| File                  | Purpose                                |
|-----------------------|----------------------------------------|
| `CMakeLists.txt`      | Build configuration                    |
| `main.cpp`            | Entry point, translation loading       |
| `OobeWindow.h/.cpp`   | Main window and all page logic         |
| `oobe.sh`             | Session launcher / firstboot setup     |
| `oobe.desktop`        | XDG autostart fallback                 |
| `firstboot.service`   | Systemd unit for pre-DM configuration  |
| `lingmo-oobe.rules`   | Polkit rule for passwordless pkexec    |
| `xsessions/*.desktop` | Display manager session entries        |
| `translations/*.ts`   | Qt translation source files            |
| `debian/`             | Debian packaging                       |

## Translations

Translation files are in `translations/` as Qt `.ts` files. To add a new language:

```bash
# Add the .ts file to CMakeLists.txt TS_FILES
# Then build (requires qt6-linguist-tools)
lupdate lingmo-oobe_<lang>.ts
# Edit the .ts file with translations
lrelease lingmo-oobe_<lang>.ts
```

## License

GNU General Public License v3.0 or later (GPL-3.0-or-later).

See `debian/copyright` for details.
