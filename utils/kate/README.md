# Kate

Modern text editor built on the KDE Frameworks and Qt.

![Kate window](https://cdn.kde.org/screenshots/kate/kate.png)

## Join Us!

So you want to participate in developing Kate?
Great!
The project is always in need of helping hands.

If you need pointers, please visit the following pages:

* Central hub for information around Kate development: Just head to our website [kate-editor.org](https://kate-editor.org/).

* How to compile Kate on your machine? Read our [Build it!](https://kate-editor.org/build-it/) tutorial.

* How to help out with the project? See our [Join Us!](https://kate-editor.org/join-us/) page.

## Licensing

Contributions to kate.git shall be licensed under [LGPLv2+](LICENSES/LGPL-2.0-or-later.txt) or [MIT](LICENSES/MIT.txt).

All files shall contain a proper "SPDX-License-Identifier: LGPL-2.0-or-later" or "SPDX-License-Identifier: MIT" identifier inside a header like:

```cpp
/*
    SPDX-FileCopyrightText: 2021 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
```

```cpp
/*
    SPDX-FileCopyrightText: 2021 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: MIT
*/
```

## kate.git content

### apps

The apps folder contains both the Kate and KWrite applications itself and their shared implementation.

#### apps/lib

The **apps/lib** directory contains the shared sources of the Kate and KWrite applications.

Close to everything beside the main functions of the two applications is implemented there.

This is installed a shared library kateprivate that can be used by addons, too.
The library isn't BC over releases, no headers to use it are installed to the system.

#### apps/kate

The **apps/kate** directory contains the Kate application sources.

Kate is using KUserFeedback for telemetry starting with release 20.04.

For details of the opt-in send information, see https://community.kde.org/Telemetry_Use

#### apps/kwrite

The **apps/kwrite** directory contains the KWrite application sources.

KWrite is using KUserFeedback for telemetry starting with release 22.08.

For details of the opt-in send information, see https://community.kde.org/Telemetry_Use

### addons

The **addons** directory contains in sub-directories the sources of all bundled plugins.
These plugins are not only used by Kate itself but other KTextEditor compatible applications, like KDevelop.

### doc

The **doc** directory contains the Kate, KWrite & KatePart manuals.

### LICENSES

The **LICENSES** directory contains the license files as referenced in the individual source files.
For any used **SPDX-License-Identifier** the matching license should be located there.

### 3rdparty

The 3rdparty folder contains 3rdparty components we bundle to be used internally.

## Kate's Mascot: Kate the Cyber Woodpecker

Kate's mascot, Kate the Cyber Woodpecker, was designed by [Tyson Tan](https://www.tysontan.com/).

More details can be found on our [mascot page](https://kate-editor.org/mascot/).

![Picture](https://kate-editor.org/images/mascot/electrichearts_20210103_kate_normal.png)
