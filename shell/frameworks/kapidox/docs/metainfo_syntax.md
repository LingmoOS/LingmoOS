# Description of the metainfo.yml syntax {#metainfo_syntax}
The name is the folder name.
The fancyname is the project name used in CMake.

Each group must contain one and only one library with the `group_info` block. If
more are defined, the behavior is unknown.

**Be sure to set `public_lib` to true, or the library will be ignored.** Most of
the other keys are optionals. See below for a simpler example.

## All possible keys.

~~~{.yaml}
description: Library doing X and Y
fancyname: The FancyName # optional, else replaced by the `project()` value of the `CMakeLists.txt`
repo_id: klib  # optional, defaults to base name of directory with the metainfo.yaml file
maintainer: ochurlaud  # optional, replaced by The KDE Community
group: frameworks # optional
subgroup: tier 1 # optional, a group must be defined
type: functional  # optional
logo: relative/path/to/logo.png # Used only if not part of a group, defaults to logo.png if present
platforms:  # optional, name accepted: all, Linux, FreeBSD, Windows, macOS, Android
    - name: Linux
      note: Functional only with running BlueZ 5 # optional
public_lib: true  # if not defined, the library is ignored
public_source_dirs:  # optional, default to src, must be a list
  - src1
  - src2
public_doc_dir: docs  # optional, default to docs
public_example_dirs: examples  # optional, default to examples, must be a list

portingAid: true/false  # optional, default to false
deprecated: true/false  # optional, default to false
libraries: KF5::MyLib  # optional

libraries:  # optional
  - qmake: BluezQt
    cmake: KF5::BluezQt
    license: LGPL-2.1-only OR LGPL-3.0-only # optional, SPDX expression that states outbound license of library
cmakename: KF5BluezQt  # optional

irc: kde  # optional, overwrite group field, if both not defined, default to kde-devel
mailinglist: mylib-dev  # optional, overwrite group field, if both not defined, default to kde-devel

group_info: # optional, only once per group
  name: frameworks # optional, else the name is defined by the 'group' key above.
  fancyname: The KDE Frameworks
  maintainer: dfaure
  irc: kde-devel
  mailinglist: kde-core-devel
  platforms:
    - Linux
    - Windows (partial)
    - macOS (partial)
  description: Providing everything from simple utility classes to ..
  logo: relative/path/to/logo.png # optional, defaults to logo.png if present
  long_description: # optional, one item per paragraph, html authorized
    - The KDE Frameworks build on the <a href="https://www.qt.io">Qt
      framework</a>, providing everything from simple utility classes (such as
      those in KCoreAddons) to integrated solutions for common requirements of
      desktop applications (such as KNewStuff, for fetching downloadable add-on
      content in an application, or the powerful KIO multi-protocol file access
      framework).
    - ....
  subgroups: # every subgroup must be declared here or will be ignored
    - name: Tier 1
      description: Tier 1 frameworks depend only on Qt (and possibly a small number of other third-party libraries), so can easily be used by an Qt-based project.
    - name: Tier 2
      description: Tier 2 frameworks additionally depend on tier 1 frameworks, but still have easily manageable dependencies.
    - ..
~~~

## Simple example 1

In this case, the library is not part of a group, and the source is in `src`. If
there is no logo, a default one is used.

~~~{.yaml}
description: Library doing X
maintainer: ochurlaud
public_lib: true
logo: libX.png
~~~

## Simple example 2

In this case, the libraries X and Y are part of the same group `XY`, and the
source is in `src`.

### libX

~~~{.yaml}
description: Library doing X
maintainer: ochurlaud
public_lib: true
platforms:
  - name: Linux
  - name: Windows
    note: doesn't work with Windows10
group: XY

group_info:
  fancyname: XY Libraries
  maintainer: arandomguy
  platforms:
    - Linux
    - Windows (partial)
  description: XY provides ...
  logo: logo_XY.png
  long_description:
    - XY does this and this
    - If you want to contribute please write to ...
~~~

### libY

~~~{.yaml}
description: Library doing Y
maintainer: otherrandomguy
public_lib: true
platforms:
  - name: Linux
group: XY
~~~
