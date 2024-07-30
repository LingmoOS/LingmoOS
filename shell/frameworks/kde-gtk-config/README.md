# KDE GTK Configurator

This project aims to provide a smooth experience for the users of GNOME/GTK applications on Plasma Desktop. 

## 👷 The Structure

KDE GTK Configurator has a number of facilities to ensure a good integration with GTK applications. The main ones are the following:

1. The daemon, that runs in background, which is a core of the whole ecosystem. It has a number of various liabilities.
2. Part of the System Settings Application Style KCM that allows the users to change and manage their GTK applications' themes. [^1]
3. GTK3 color reload module, which allows GTK3 applications, that use [Breeze GTK theme](https://invent.kde.org/plasma/breeze-gtk), to update their UI colors without restarting them.

## 😈 The daemon

The daemon itself does the following:

1. Watching for changes inside of various Plasma configuration files and on change also apply that setting for the GTK applications using their configuration methods and values.
2. Applying GTK theme on demand from the Application Style KCM
3. Applying colors for Breeze GTK Theme according to the color scheme set in the Colors KCM.

### ⚙ Settings the daemon syncs

- 🔤 UI Font
- 💾 Icon Theme
- 👆 Cursor theme and size
- 🆗 Icons on Buttons (Show them or not)
- 🗒 Icons in Menus (Show them or not)
- 🧰 Toolbar Style
- 📜 Scrollbar behavior on click
- 🌓 Dark Theme Preference (Depending on a color scheme set in Colors KCM)
- ❎ Decorations button order (all GTK themes) and appearance (Breeze GTK theme only) 
- 🏃 Animations (Enable/Disable)
- 🎨 Color Scheme (Breeze GTK theme only)


### ⚙ GTK configuration methods the daemon uses

On X11 Plasma session the daemon uses mainly [xsettingsd](https://github.com/derat/xsettingsd) daemon to control the appearance of the GTK applications. Additionally it uses `settings.ini` and `.gtkrc-2.0` file to provide some settings, that are not available in the xsettingsd configuration.

On Wayland session the daemon uses [GSettings](https://developer.gnome.org/gio/stable/GSettings.html).

##

[^1]: The code for the UI part is located in the [Plasma Workspace](https://invent.kde.org/plasma/plasma-workspace/-/tree/master/kcms/style)
