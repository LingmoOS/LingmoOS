# liblingmo

Foundational libraries, components, and tools of the Lingmo workspaces

## Introduction
liblingmo provides the following:
- QML components that can be used by any Lingmo shell
- A C++ library: `liblingmo` itself
- Script engines

## QML components
### org.kde.lingmo.core

Bindings for liblingmo functionality, such FrameSvg, see @ref core.

### org.kde.lingmo.components
Graphical components for common items such as buttons, lineedits, tabbars and so on. Compatible subset of the MeeGo components used on the N9, see @ref lingmocomponents.

### org.kde.lingmo.extras
Extra graphical components that extend org.kde.lingmo.components but are not in the standard api, see @ref lingmoextracomponents.

### org.kde.lingmo.plasmoid
Attached properties for manipulating the current applet or containment, see @ref liblingmoquick

## liblingmo
This C++ library provides:
- rendering of SVG themes
- loading of files from a certain filesystem structure: packages
- data access through data engines
- loading of the plugin structure of the workspace: containments and applets

See @ref liblingmo.

## Script engines
Provides support to create applets or containments in various scripting languages.

