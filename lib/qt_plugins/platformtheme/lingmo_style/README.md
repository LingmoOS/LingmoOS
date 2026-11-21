# Vendored Darkly Qt Widgets style

This directory contains the Qt Widgets style implementation copied from
`Darkly/kstyle` and its `animations` directory. The source is kept under the
original `Darkly` namespace and retains the upstream GPL notices.

Only the style implementation, its animation engines, generated KConfig
inputs, and the shared box-shadow renderer are included. Darkly's KDE control
module, debug widget explorer sources, colors, desktop theme, KWin decoration,
and tests are intentionally excluded from this repository.

The parent CMake project builds this source as an optional Qt 6 style plugin
when ECM and the required KDE Frameworks development packages are available.
On systems without those packages, the ordinary Lingmo platform theme still
builds and this vendored source remains available for a later packaging step.
