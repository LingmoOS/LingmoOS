# KWayland

KWayland is a Qt-style API to interact with the wayland-client API.

## Introduction

The API is Qt-styled removing the needs to interact with a for a Qt developer
uncomfortable low-level C-API. For example the callback mechanism from the Wayland API
is replaced by signals; data types are adjusted to be what a Qt developer expects, e.g.
two arguments of int are represented by a QPoint or a QSize.

## KWayland Client

The idea around KWayland Client is to provide a drop-in API for the Wayland client library which at
the same time provides convenience Qt-style API. It is not intended to be used as a replacement for
the QtWayland QPA plugin, but rather as a way to interact with Wayland in case one needs Qt to use
a different QPA plugin or in combination with QtWayland to allow a more low-level interaction without
requiring to write C code.

### Convenience API

The convenience API in KWayland Client provides one class wrapping a Wayland object. Each class can
be casted into the wrapped Wayland type. The API represents events as signals and provides simple
method calls for requests.

Classes representing global Wayland resources can be created through the [Registry](@ref KWayland::Client::Registry). This class eases
the interaction with the Wayland registry and emits signals whenever a new global is announced or gets
removed. The Registry has a list of known interfaces (e.g. common Wayland protocols like `wl_compositor`
or `wl_shell`) which have dedicated announce/removed signals and objects can be factored by the Registry
for those globals.

Many globals function as a factory for further resources. E.g. the Compositor has a factory method for
Surfaces. All objects can also be created in a low-level way interacting directly with the Wayland API,
but provide convenience factory methods in addition. This allows both an easy usage or a more low level
control of the Wayland API if needed.

### Integration with QtWayland QPA

If the QGuiApplication uses the QtWayland QPA, KWayland allows to integrate with it. That is one does
not need to create a new connection to the Wayland server, but can reuse the one used by Qt. If there
is a way to get a Wayland object from Qt, the respective class provides a static method normally called
`fromApplication`. In addition the API allows to get the Surface from a QWindow.

## Using KWayland in your application

### With CMake

KWayland installs a CMake Config file which allows to use KWayland as imported targets.

To find the package use for example:

    find_package(KWayland CONFIG)
    set_package_properties(KWayland PROPERTIES TYPE OPTIONAL )
    add_feature_info("KWayland" KWayland_FOUND "Required for the awesome Wayland on Qt demo")

Now to link against the Client library use:

    add_executable(exampleApp example.cpp)
    target_link_libraries(exampleApp Plasma::KWaylandClient)

Please make sure that your project is configured with C++11 support:

    CONFIG += c++11
