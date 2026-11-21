# Lingmo Aurora Initial Setup

The first-run setup application for an installed Lingmo Aurora system.

The installer configures `lingmo-dm` to start the `lingmo-initial-setup` Wayland
session for the installed account. OOBE writes
`~/.config/lingmo/oobe.conf` after the user chooses **Get Started**. On later
automatic logins, `lingmo-dm` sees that marker and starts `lingmo-wayland`
instead. The marker is per user, so it never affects another account.

## Build

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The build uses the Lingmo SDK and installs `lingmo-initial-setup.desktop` into the
Wayland session directory.
