# Qt Plugins

Qt plugins that make Qt applications at home in the Lingmo session. The
`lingmo` platform theme makes them follow the system's dark mode, fonts,
icon and cursor themes, open the Lingmo file chooser, and scroll with the
same touchpad feel as Lingmo's own apps. The image format plugins add SVG,
HEIF/HEIC/AVIF and JPEG XL.

The repository vendors the Qt Widgets painting source from Darkly under
`platformtheme/lingmo_style`. It is kept separate from the QPA platform theme:
the copied source can be enabled when the KDE Frameworks development packages
are available, while KDE configuration modules, debug tools, window managers
and decoration code are intentionally not part of this plugin repository.

## Dependencies

`sudo pacman -S gcc cmake extra-cmake-modules qt6-base qt6-declarative qt6-tools kf6-kcoreaddons kf6-kcolorscheme kf6-kconfig kf6-kguiaddons kf6-ki18n kf6-kiconthemes kf6-kwindowsystem wayland wayland-protocols librsvg libheif libjxl`

The vendored `lingmo_style` Widgets plugin is built by default. Use
`-DLINGMO_BUILD_DARKLY_STYLE=OFF` only when creating a reduced build without
the style plugin.

The platform theme advertises `Lingmo` as the first Qt Widgets style. To test
an installed plugin in one process, run `QT_STYLE_OVERRIDE=Lingmo ark` or
`QT_STYLE_OVERRIDE=Lingmo fcitx5-config-qt`.

## Build

```shell
mkdir build
cd build
cmake ..
make
sudo make install
```

## License

GPL-3.0-or-later; see [LICENSE](LICENSE).
