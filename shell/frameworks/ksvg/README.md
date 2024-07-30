# KSvg
A library for rendering SVG-based themes with stylesheet re-coloring and on-disk caching.

## Introduction
KSvg provides both C++ classes and QtQuick components to render svgs based on image packs.
Compared to plain QSvg, it caches the rendered images on disk with KImageCache, and can re-color
properly crafted svg shapes that include internal stylesheets.

The default behavior is to re-color with the application palette, making it possible to create UI elements in SVG.

## C++
In C++ there are 3 main classes, usable also in QWidget applications with a QPainter-compatible API:

* ``ImageSet``: Used to tell the other classes where to find the SVG files: by default, SVG "themes"
                will be searched in the application data dir (share/application_name/theme_name)
* ``Svg``: Class to used to render Svg files: it loads a file with ``setImagePath`` using relative paths
            to the theme specified in ``ImageSet``
* ``FrameSvg``: A subclass of Svg used to render 9-patch images, such as Buttons, where you want to stretch
                only the central area but not the edges

## QML
Import QML bindings with ``import org.kde.ksvg 1.0 as KSvg``.

ImageSet is exported directly to QML which makes it possible to set the theme from QML.

Svg and FrameSvg have corresponding items, ``SvgItem`` and ``FrameSvgItem``, which inherit from QQuickItem
and will paint their associated ``Svg`` or ``FrameSvg`` stretched to their full geometry.

QML code example:

```
FrameSvgItem {
    // This resolves to a file like /usr/share/myapp/mytheme/widgets/button.svgz
    imagePath: "widgets/button"
    prefix: "pressed"
}
```

## More documentation
Assume the theme filesystem hierarchy used by the Plasma shell, but the general concepts apply everywhere:

* https://develop.kde.org/docs/plasma/theme/theme-elements/
* https://develop.kde.org/docs/plasma/theme/quickstart/
