# deepin-turbo

deepin-turbo is a deepin project that derives from Applauncherd.

Applauncherd is a daemon that helps to launch applications faster by preloading dynamically linked libraries and caching stuff. It also saves memory, because all launched applications share certain resources.

we built a booster type called dtkwidget-booster in this project to support launching dtk-based Apps faster.

## Build from source code
```bash
    mkdir build
    cd build
    cmake ..
    make -j`nproc`
```

## Technical overview

Booster daemons (written using the provided library) are started as part of the user session. The booster is responsible for forking the will-be-application before knowing which application is going to be launched next. There can be different kinds of boosters optimized for different kinds of
applications, e.g. Qt or QML.

In the current architecture boosters are implemented as seperate processes, using the provided support library. Each booster process waits for launch commands.

The user uses the launcher always through a special invoker program. The invoker (/usr/bin/invoker) tells booster process to load an application binary via a socket connection. 

The application to be launched via applauncherd should be compiled as a shared library or a position independent executable (-pie) and it should always export main(). There's also a "booster" for all applications. In that case exec() is used.



## Technical details

Loading the binary is done with dlopen(), and therefore the application needs to be compiled and linked as a shared library or a position independent executable. The booster process also sets the environment variables. Finally, it finds the main function in the application binary with dlsym() and calls the main() with the command line arguments given by the invoker.

Booster processes do some initializations that cannot be shared among other processes and therefore have to be done after forking. This allows, for instance, instantiating a application before knowing the
name of the application. Then the booster process waits for a connection from the invoker with the information about which application should be launched. 



## Maintainers

- [@zccrs](https://github.com/zccrs)
- [@hualet](https://github.com/hualet)



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

## Getting help

Any usage issues can ask for help via

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [WiKi](https://wiki.deepin.org)
* [Forum](https://bbs.deepin.org)
* [Developer Center](https://github.com/linuxdeepin/developer-center/issues) 

## Getting involved

We encourage you to report issues and contribute changes

- [**Contribution guide for developers**](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License
deepin-turbo is licensed under [GPL-3.0-or-later](LICENSE).
