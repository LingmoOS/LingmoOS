<!--
SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>

SPDX-License-Identifier: LGPL-2.1-or-later
-->

# Lingmo Pass

Lingmo Pass is a Lingmo applet to access password from [pass, the standard UNIX password manager](https://www.passwordstore.org).

More details and a video in [my blog post about Lingmo Pass](https://www.dvratil.cz/2018/05/lingmo-pass/).

You don't need the *pass* utility installed on your system in order for Lingmo Pass to work,
only GnuPG is needed in order to be able to decrypt the password.

Lingmo Pass looks for the password directory by default in `$HOME/.password-store`, but
it can be customized through `PASSWORD_STORE_DIR` environment variable.

## Build Instructions

1) Install necessary dependencies

Fedora:

```
dnf install qt5-qtbase-devel qt5-qtdeclarative-devel kf5-lingmo-devel kf5-ki18n-devel kf5-kitemmodels-devel liboath-devel qgpgme-devel liboath-devel
```

Debian/Ubuntu:

```
apt-get install qtbase5-dev qtdeclarative5-dev libkf5lingmo-dev libkf5i18n-dev libkf5itemmodels-dev liboauth-dev libgpgmepp-dev liboath-dev
```

2) Clone source code:

```
git clone https://invent.kde.org/lingmo/lingmo-pass.git
```

3) Compile:

```
cd lingmo-pass
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=YOURPREFIX ..
make
make install
```
(replace `YOURPREFIX` with where your Lingmo is installed)

4) Restart Lingmo:

```
lingmoshell --replace &
```

Alternatively you can also view the applet with `plasmoidviewer`:

```
plasmoidviewer -a org.kde.lingmo.pass
```
