# Lingmo AppMotor

Optimize the speed of starting lingmo apps.

## Dependencies

Debian/Ubuntu Dependencies:

```shell
sudo apt install cmake qtbase5-dev qtdeclarative5-dev qtquickcontrols2-5-dev libsystemd-dev libcap-dev libdbus-1-dev
```

## Build

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

## Introduction

Applauncherd is a daemon that helps to launch applications faster by
preloading dynamically linked libraries and caching stuff.
It also saves memory, because all launched applications share certain resources.

Applauncherd also provides support for fast single instance launches.

Some technical details are explained below.

Install applauncherd-doc for the Doxygen-based user documentation.
See INSTALL on how to build applauncherd and the documentation.

Booster daemons (written using the provided library) are started as part of the
user session. The booster is responsible for forking the will-be-application
before knowing which application is going to be launched next. There can be
different kinds of boosters optimized for different kinds of
applications, e.g. Qt or QML.

In the current architecture boosters are implemented as seperate processes,
using the provided support library. Each booster process waits for launch
commands.

The user uses the launcher always through a special invoker program. The
invoker (/usr/bin/invoker) tells booster process to load an application
binary via a socket connection. 

The application to be launched via applauncherd should be compiled as a
shared library or a position independent executable (-pie) and it should
always export main(). There's also a "booster" for all applications.
In that case exec() is used.

## Technical details

Loading the binary is done with dlopen(), and therefore the
application needs to be compiled and linked as a shared library or a
position independent executable. The booster process also sets the
environment variables. Finally, it finds the main function in the
application binary with dlsym() and calls the main() with the command
line arguments given by the invoker.

Booster processes do some initializations that cannot be shared among
other processes and therefore have to be done after forking. This allows,
for instance, instantiating a application before knowing the
name of the application. Then the booster process waits for a connection
from the invoker with the information about which application should be
launched. 

## Contributors

People who have contributed to mapplauncherd:

Robin Burchell
John Brooks
Thomas Perl

People who have contributed to meegotouch-applauncherd:

Olli Leppänen
Jussi Lind
Juha Lintula
Pertti Kellomäki
Antti Kervinen
Nimika Keshri
Alexey Shilov
Oskari Timperi

Some parts of the code is based on the maemo-launcher by:

Guillem Jover
Michael Natterer

## License

This project has been licensed by LGPL-2.1.
