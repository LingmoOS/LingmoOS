# KDE GUI Addons

Utilities for graphical user interfaces

## Introduction

The &KDE GUI addons provide utilities for graphical user interfaces in the areas
of colors, fonts, text, images, keyboard input.

## Usage

To use KGuiAddons, you'll want to look for it using

```cmake
find_package(KF6GuiAddons)
```

or its QMake counterpart.

### Color utilities

The KColorUtils namespace contains various small utilities to work with colors. KColorCollection provides
handling of color collections ("pallettes"). KColorMimeData adds drag-and-drop and clipboard mimedata
manipulation to QColor objects

### Text utilities

KWordWrap is a special word wrapping algorithm that takes beauty into account. It can be used
directly with QPainter or can return the wrapped text in a QString.

KDateValidator is a QValidator that validates user-entered dates.

KFontUtils::adaptFontSize() is a function that calculates the biggest font size (in points)
that can be used to draw a text centered in a rectangle using word wrapping.

### Keyboard utilities

KKeySequenceRecorder is a utility that records a QKeySequence by listening to key events in a window.
This can be used to let the user modify predefined keyboard shortcuts for example.

### Icon utilities

KIconUtils contains functions to add overlays on top of a QIcon.

### Cursor utilities

KCursorSaver is used to temporarily set a mouse cursor and restore the previous one on destruction.

