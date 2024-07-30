# Plasma 5 Firewall KCM

This is the repository for the Plasma 5 Firewall KCM.

# Requirements
Plasma 5 Firewall has few dependencies mainly from Qt5 and KF5 with minimum required version being
- Qt                              >= 5.10.0
- KDE Workspace         >= 5.8.0
- Plasma Framework    >= 5.32

Currently the list of Dependencies include :
- KF5CoreAddons (required version >= 5.60.0), Qt addon library with a collection of non-GUI utilities.
-  KF5KCMUtils (required version >= 5.60.0), KF5 Utilities for KDE System Settings modules library.
- KF5I18n (required version >= 5.60.0), KF5 text internationalization library.
- KF5 Plasma Framework >= 5.32
- UFW or Firewalld

**To setup the environment first time for any project in KDE you can have a look at [here](https://community.kde.org/Get_Involved/development)**



## Building the Project
The next step is to get Plasma5 Firewall sources and build them.

```bash
cd plasma-firewall
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLIB_INSTALL_DIR=lib -DKDE_INSTALL_USE_QT_SYS_PATHS=ON ../
make
```
The following command will install the Plasma5 Firewall system wide.

```bash
sudo make install
```
## Issues
The Plasma5 Firewall is open to any contribution and feedback. If you find problems with the contents of this repository please create an issue.

## Contributing and Getting Involved
Development can be followed on [Plasma Firewall tasks board](https://invent.kde.org/plasma/plasma-firewall/-/boards). Code contributions have to be submitted via [GitLab](https://invent.kde.org/plasma/plasma-firewall). You can follow generic user instructions for [GitLab and a merge request workflow](https://community.kde.org/Infrastructure/GitLab).

## License
SPDX-License-Identifier: BSD-3-Clause
SPDX-FileCopyrightText: 2020 Tomaz Canabrava tcanabrava@kde.org

