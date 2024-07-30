# libplasma

Foundational libraries, components, and tools of the Plasma workspaces

## Introduction
libplasma provides the following:
- QML components that can be used by any Plasma shell
- A C++ library: `libplasma` itself
- Script engines

## QML components
### org.kde.plasma.core

Bindings for libplasma functionality, such FrameSvg, see @ref core.

### org.kde.plasma.components
Graphical components for common items such as buttons, lineedits, tabbars and so on. Compatible subset of the MeeGo components used on the N9, see @ref plasmacomponents.

### org.kde.plasma.extras
Extra graphical components that extend org.kde.plasma.components but are not in the standard api, see @ref plasmaextracomponents.

### org.kde.plasma.plasmoid
Attached properties for manipulating the current applet or containment, see @ref libplasmaquick

## libplasma
This C++ library provides:
- rendering of SVG themes
- loading of files from a certain filesystem structure: packages
- data access through data engines
- loading of the plugin structure of the workspace: containments and applets

See @ref libplasma.

## Script engines
Provides support to create applets or containments in various scripting languages.

