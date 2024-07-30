# KDecoration2

Plugin based library to create window decorations.

## Introduction

KDecoration2 is a library to create window decorations. These window decorations can be used by
for example an X11 based window manager which re-parents a Client window to a window decoration
frame.

The library consists of two parts:
* Decoration API for implementing a Decoration theme
* Private API to implement the backend part (e.g. from Window Manager side)

## Providing a Decoration

To provide a custom decoration one needs to create a plugin and provide an own implementation
of KDecoration2::Decoration. For a framework to load and find the plugin it needs to be compiled
with the proper json metadata. An example for such metadata (deco.json):

```json
{
    "KPlugin": {
        "Id": "org.kde.myAweseomeDecoration",
        "ServiceTypes": [
            "org.kde.kdecoration2"
        ]
    },
    "X-KDE-ConfigModule": "kcm_name", /* comes with a configuration module */
    "org.kde.kdecoration2": {
        "blur": false, /* blur behind not needed */
    }
}
```

To simplify one can use the KPluginFactory macro from the KCoreAddons framework:

```cpp
K_PLUGIN_FACTORY_WITH_JSON(
    MyAwesomeDecorationFactory,
    "deco.json",
    registerPlugin<MyAwesomeDecoration::Decoration>();
)
```

The plugin needs to get installed to `${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration2`.

## Configuring the Decoration

It is possible to provide a configuration module to tweak some aspects of the decoration. This is done
by creating a plugin that provides such a configuration module.

The `kcmoduleName` specifies the name of the configuration module. It needs to be installed under
`${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration2.kcm` so that it can be looked up.
