# DWayland

DWayland is a Qt-style API to interact with the wayland-client and wayland-server API.

## Introduction

DWayland provides two libraries:

- DWayland::Client
- DWaylandServer

As the names suggest they implement a Client respectively a Server API for the Wayland
protocol. The API is Qt-styled removing the needs to interact with a for a Qt developer
uncomfortable low-level C-API. For example the callback mechanism from the Wayland API
is replaced by signals; data types are adjusted to be what a Qt developer expects, e.g.
two arguments of int are represented by a QPoint or a QSize.

## DWayland Server

### Head-less API

The server library can be used to implement a Wayland server with Qt. The API is head-less
meaning it does not perform any output and does not restrict on the way how one wants to
render. This allows to easily integrate in existing rendering code based on e.g. OpenGL or
QPainter. Applications built on top of DWayland Server integrated the graphics with the
following technologies:

- OpenGL over DRM/KMS
- OpenGL over X11
- OpenGL over Wayland
- OpenGL over Android's hwcomposer enabled through libhybris
- QPainter over DRM/KMs
- QPainter over fbdev
- QPainter over X11
- QPainter over Wayland
- QWidget
- QtQuick

Although the library does not perform any output, it makes it very easy to enable rendering.
The representation for a [Buffer](@ref DWaylandServer::BufferInterface) allows easy conversion
to a (memory-shared) QImage in case the buffer represents a shared memory buffer. This QImage
can be used for rendering in a QPainter based API or to generate an OpenGL texture.

### Easy usage of Wayland API

The library hides many Wayland implementation details. For all Wayland interfaces which have
double buffered state the classes always only provide access to the committed state. The pending
state is an internal detail. On commit of the pending state Qt signals are emitted about what
changed.

Buffers are ref-counted and automatically released if it is no longer referenced allowing the
client to reuse it. This happens fully automatically when a surface no longer references a buffer.
As long as a buffer is attached surface, the surface has it referenced and the user of the API can
access the buffer without needing to care about referencing it.

The API of DWayland is hand-crafted to make usage easier. The representation of a
[Surface](@ref DWaylandServer::SurfaceInterface) combines multiple aspects about a Surface even
if in Wayland API it is added to other elements. E.g. a Surface contains all
[SubSurfaces](@ref DWaylandServer::SubSurfaceInterface) attached to it instead of the user
having to monitor for which Surface a SubSurface got created.

Similar the representation of a [Seat](@ref DWaylandServer::SeatInterface) combines all aspects of
the Seat. A user of the API only needs to interact with the Seat, there is no need to track all the
created [keyboards](@ref DWaylandServer::KeyboardInterface), [pointers](@ref DWaylandServer::PointerInterface), etc. The
representation of Seat tracks which keyboards are generated and is able to forward events to the
proper focus surface, send enter and leave notifications when needed without the user of the API
to care about it.

### Handling input events

Just like with output the server API does not restrict on how to get input events. This allows to
integrate with existing input handlers and also allows to easily filter the input before it is passed
to the server and from there delegated to the client. By that one can filter out e.g. global touch
gestures or keyboard shortcuts without having to implement handlers inside DWayland. The SeatInterface
provides a very easy to use API to forward events which can be easily integrated with Qt's own
input event system, e.g. there is a mapping from Qt::MouseButton to the Linux input code.

Applications built on top of DWayland Server integrated input events with the following technologies:

- libinput
- X11
- Wayland
- Android's inputstack enabled through libhybris
- QInputEvent

### Private IPC with child processes

DWayland Server is well suited for having a private IPC with child processes. The [Display](@ref DWaylandServer::Display) can be
setup in a way that it doesn't create a public socket but only allows connections through socket
pairs. This allows to create a socketpair, pass one file descriptor to DWayland server and the other
to the forked process, e.g. through the WAYLAND_SOCKET environment variable. Thus a dedicated IPC
is created which can be used even for running your own custom protocol. For example KDE Plasma uses
such a dedicated parent-child Wayland server in it's screen locker architecture.

Of course private sockets can be added at any time in addition to a publicly available socket. This
can be used to recognize specific clients and to restrict access to interfaces for only some dedicated
clients.

## DWayland Client

The idea around DWayland Client is to provide a drop-in API for the Wayland client library which at
the same time provides convenience Qt-style API. It is not intended to be used as a replacement for
the QtWayland QPA plugin, but rather as a way to interact with Wayland in case one needs Qt to use
a different QPA plugin or in combination with QtWayland to allow a more low-level interaction without
requiring to write C code.

### Convenience API

The convenience API in DWayland Client provides one class wrapping a Wayland object. Each class can
be casted into the wrapped Wayland type. The API represents events as signals and provides simple
method calls for requests.

Classes representing global Wayland resources can be created through the [Registry](@ref DWayland::Client::Registry). This class eases
the interaction with the Wayland registry and emits signals whenever a new global is announced or gets
removed. The Registry has a list of known interfaces (e.g. common Wayland protocols like `wl_compositor`
or `wl_shell`) which have dedicated announce/removed signals and objects can be factored by the Registry
for those globals.

Many globals function as a factory for further resources. E.g. the Compositor has a factory method for
Surfaces. All objects can also be created in a low-level way interacting directly with the Wayland API,
but provide convenience factory methods in addition. This allows both an easy usage or a more low level
control of the Wayland API if needed.

### Integration with QtWayland QPA

If the QGuiApplication uses the QtWayland QPA, DWayland allows to integrate with it. That is one does
not need to create a new connection to the Wayland server, but can reuse the one used by Qt. If there
is a way to get a Wayland object from Qt, the respective class provides a static method normally called
`fromApplication`. In addition the API allows to get the Surface from a QWindow.

## Using DWayland in your application

### With CMake

DWayland installs a CMake Config file which allows to use DWayland as imported targets. There is
one library for Client and one for Server.

To find the package use for example:

    find_package(DWayland CONFIG)
    set_package_properties(DWayland PROPERTIES TYPE OPTIONAL )
    add_feature_info("DWayland" DWayland_FOUND "Required for the awesome Wayland on Qt demo")

Now to link against the Client library use:

    add_executable(exampleApp example.cpp)
    target_link_libraries(exampleApp Deepin::WaylandClient)

To link against the Server library use:

    add_executable(exampleServer exampleServer.cpp)
    target_link_libraries(exampleServer Deepin::DWaylandServer)

### With QMake

DWayland installs .pri files for the Client and Server library allowing easy usage in QMake based
applications.

Just use:

    QT += DWaylandClient

Respectively:

    QT += DWaylandServer

Please make sure that your project is configured with C++11 support:

    CONFIG += c++11
