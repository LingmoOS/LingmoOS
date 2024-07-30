# Plasma Thunderbolt

This repository contains a Plasma Sytem Settings module and a KDED module to
handle authorization of Thunderbolt devices connected to the computer. There's
also a shared library (libkbolt) that implements common interface between the
modules and the system-wide bolt daemon, which does the actual hard work of
talking to the kernel.

# Compiling

The project has no special build-time dependencies other than Qt and reasonably-new
KDE Frameworks.

# Runtime Dependencies

This software depends on the [Bolt daemon](https://gitlab.freedesktop.org/bolt/bolt) to
be present and running on the system.

# Contributing

To contribute, please follow this [Get Involved](https://community.kde.org/Get_Involved)
guide on KDE Community wiki.

# Maintainers

Daniel Vrátil <dvratil@kde.org>


