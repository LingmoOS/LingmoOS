# KPipeWire

Offers a set of convenient classes to use PipeWire (https://pipewire.org/) in Qt projects.

It is developed in C++ and it's main use target is QML components.

As it's what's been useful, this framework focuses on graphical PipeWire features. If it was necessary, these could be included.

## Features

At the moment we offer two main components:
* KPipeWire: offers the main components to connect to and render PipeWire into your app.
* KPipeWireRecord: using FFmpeg, helps to record a PipeWire video stream into a file.

## Usage

You can refer to the `tests/` subdirectory for two examples of rendering Plasma video streams. One through the internal Plasma Wayland protocol and the other using the standard XDG Desktop Portals screencast API.

## Licence

This project is licenced under LGPL v2.1+, you can find all the information under `LICENSES/`.
