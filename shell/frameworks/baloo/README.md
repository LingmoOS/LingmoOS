# Baloo

## Introduction

Baloo is the file indexing and file search framework for KDE Plasma. It focuses 
on speed and a very small memory footprint. It maintains an index of your files 
and optionally their contents which [you can search](./docs/user/searching.md).

## Contributing

Baloo is part of the KDE umbrella and relies on the KDE infrastructure.

**Mailing List:** kde-devel@kde.org ([info page](https://mail.kde.org/mailman/listinfo/kde-devel))
**Bug Tracker:** http://bugs.kde.org  ([new bug](https://bugs.kde.org/enter_bug.cgi?product=frameworks-baloo))
**IRC Channel:** #kde-baloo on Libera Chat

The recommended way of contributing patches is via KDE's [GitLab](https://invent.kde.org/frameworks/baloo) instance.

## Documentation

### Users
* [Searching](./docs/user/searching.md)
* [The Baloo pages on KDE Community Wiki](https://community.kde.org/Baloo) has information on Baloo's command-line tools and how to monitor its operation.

### File Indexing Plugins

Baloo relies on [KFileMetaData](https://api.kde.org/frameworks/kfilemetadata/html/index.html) to extract content and
metadata from files. KFileMetadata uses a number file type specific plugins. In case individual
plugins are packaged separately, we recommend installing all plugins.

Without the indexer plugins, Baloo cannot function to its full potential.

### Developers
[![Build Status](https://build.kde.org/job/Frameworks/job/baloo/job/kf5-qt5%20SUSEQt5.15/badge/icon?subject=SUSE%20Qt5.15)](https://build.kde.org/job/Frameworks/job/baloo/job/kf5-qt5%20SUSEQt5.15/)
[![Build Status](https://build.kde.org/job/Frameworks/job/baloo/job/kf5-qt5%20FreeBSDQt5.15/badge/icon?subject=FreeBSD%20Qt5.15)](https://build.kde.org/job/Frameworks/job/baloo/job/kf5-qt5%20FreeBSDQt5.15/)
* [Build Instructions](@ref build-instructions)
* Baloo follows the [KDE Frameworks coding style](https://community.kde.org/Policies/Frameworks_Coding_Style).

### Distributions
Baloo is developed and tested exclusively for Linux. While it may run on other
unix based systems. It is not recommended, and certainly not tested.

We do not recommend to package Baloo for Windows or OSX as both these operating
systems offer their own file searching solutions which better integrate with
the native system than Baloo ever will.

Baloo may run on 32-bit systems, but it has not been tested and may not work
correctly. Please test and let us know by [filing a bug](https://bugs.kde.org/enter_bug.cgi?product=frameworks-baloo).

**Supported Kernels:** Linux
**Supported Architectures:** x86_64, aarch64
**Supported Filesystems:** ext3/4, Btrfs, XFS

#### Packaging the indexer service

The service is an autostarted, session wide background service. In case KF5 and KF6
versions of the Baloo libraries are installed, both will use the same service and
database.

Only one version of the service autostart file and associated binaries should be
installed at a time, i.e. the `baloo_file.desktop` service definition, the `baloo_file`
service binary, the `baloo_file_extractor` helper, and the `balooctl` tool.

Building of the service can be disabled using the `BUILD_INDEXER_SERVICE` CMake option.
