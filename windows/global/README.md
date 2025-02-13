# LingmoOS-Rounded-Corners

This is the round corner plugin for LingmoOS.


# Load & Unload

To activate the effect, you can now log out and log back in, or run the command below inside the `build` directory:
```bash
sh ../tools/load.sh
```

To fully uninstall the effect, run the following commands inside the `build` directory:

```bash
sh ../tools/unload.sh
sudo make uninstall
```

# Auto install after KWin update

After each `kwin` package update, the effect becomes incompatible. So it won't load without a rebuild.

As long as the effect is not part of the `kwin` yet (being discussed 
[here](https://invent.kde.org/plasma/kwin/-/issues/198)), you can automate the re-installation by running the command
below inside the `build` directory:

```bash
sh ../tools/install-autorun-test.sh
```

The command above adds a `desktop` file inside the `autorun` directory which checks if the effect is still supported,
if it is not supported, it will automatically rebuild and reinstall the effect.

# Tips

## Disable conflicting native window outline

If using Breeze (default) window decorations with Plasma 5.27 or higher you may wish to disable the native window outline, to prevent it from overlapping and causing visual glitches.

- System settings -> Themes -> Window Decorations -> Breeze -> Edit icon -> Shadows and Outline tab -> Outline intensity (Off)

## Add shadow to windows without decoration (like Steam)

You can add shadows for specific windows using the hack below. I don't know how to enforce it in my code.

1. In [ System settings ] -> [ Window management ] -> [ Window rules ] -> [ Appearance & Fixes ]:

   **Add [steam] and set [ No titlebar ] and frame to [ No ]**

2. In [ System settings ] -> [ Application Style ] -> [ Window decoration ] -> [ Breeze theme setting ] -> [ Window specific overrides ]:

   **Add [steam] and set [ Hide Window title bar ] to [ Yes ].**

After that, the Steam window gets its shadows back.

## Add Debug Messages

When troubleshooting or reporting an issue, it might be useful to enable Debug logs during the build time using:

```bash
cmake .. --DCMAKE_BUILD_TYPE=Debug
cmake --build . -j
```

After the installation and loading the effect, debug messages would appear in `journalctl`:

```bash
journalctl -f | grep kwin
```

or have some colorful logs with

```bash
sh ../tools/show-kwin-logs.sh
```
