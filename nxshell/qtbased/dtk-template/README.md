# dtk
------------
Qt-based development template library.

## Features
1. Use `cmake` for project management.
2. Use `doxygen` for documentation.
3. Use `gtest` for unit testing.
4. Use `reuse` for open source protocol checking.
5. Support generating unit test coverage reports.
6. Both `g++` and `clang++` compilers are supported.
7. Compatible with `Qt6` and `Qt5`.
8. Support for compiling as a `debian` platform installer.
9. Use the script to configure the project with one click, see [Usage](# Usage)

## Usage
1. Clone this repository；
2. Config project name:
```shell
bash config.sh -n <name>
bash config.sh -N <namespace>
```
3. Modify other configuration files, e.g. README.md

## Catalog Specification
 **Catalog**           | **Description**
------------------|---------------------------------------------------------
 .github/         | github related files, generally used to store workflows
 .reuse/          | license declaration file, the project should use the reuse tool for license declaration
 .gitignore       | The git filter list, used to filter files that don't need to be pushed to the repository
 .clang-format    | Formatting declaration file, which is used for formatting code to maintain formatting uniformity
 README.md        | Project Overview Document
 LICENSE          | License agreement file, the file to github such repository use, the project should use reuse tool, but the agreement must be unified, generally LGPL-v3
 CMakeLists.txt   | Project files can be placed in the outermost
 [debian/]        | Required files for debian packaging
 docs/            | Store document-related content, currently used as a directory for doxygen-generated documents
 include/         | Public headers directory, all open headers should be placed in this directory
 LINCENSES/       | License agreement directory, where all license agreements for the project are stored
 misc/            | Store project-independent resources or configuration files, such as .service .conf .in, etc.
 src/             | Store source code files, such as: .h .cpp
 [src/3rdparty/]  | Store third-party libraries or components. **Please note that if the repository contains a corresponding project, please use the version in the repository and do not allow to provide a separate version to be included in the project**
 [qml/]           | Specialized qml source files, such as .qml
 [translations/ ] | Store localization-related files, such as .ts
 [tests/]         | Unit test related code storage directory
 [tools/]         | Tools storage directory
 [examples/]      | Examples storage directory

## Style Guide
This style guide follows the [deepin-styleguide](https://github.com/linuxdeepin/deepin-styleguide/releases) and is refined and split on its basis to form a style guide applicable to development libraries.

### Basic
1. The development library must be based on `Qt` and can depend on `dtkcore`, but is not allowed to depend on `dtkgui` and `dtkwidget`, in order to minimize dependencies, and should not depend on `dtkcore` if it does not need to.
2. Try to use pre-declarations, unlike [deepin-tyleguide](https://github.com/linuxdeepin/deepin-styleguide/releases) here, as a development library, to try to minimize dependencies on header files.

### Naming

#### General Rules
1. Use descriptive naming whenever possible, use full word combinations whenever possible, and be sure not to use abbreviations or shorthand.

#### Project Name
The project name should start with the three letters `dtk` and be all lowercase, no ligatures are allowed, e.g. `dtkpower`, `dtkpowermanager`.

#### File Name
1. The header and source files need to make the words all lowercase and start with `d`, e.g.: `dpower.h`, `dpowermanager.h`, and, if necessary, use an underscore combination (generally referring to the `private` class), e.g.: `dpowermanager_p.h`.
2. The configuration file should be named using the project name + suffix, e.g. `dtkpower.conf` or `dtkpowermanager.conf`
#### Namespace And Class
The namespace needs to have two layers.<br>
The top-level namespace is unified as `Dtk`, and the secondary namespace is the project namespace, such as: `Dtk::Power` or `Dtk::PowerManager`.<br>
Classes should be named with `D` as the first letter, such as: `DPower`,`DPowerManager`.

## Doxygen And Reuse
1. The project needs to be documented using `doxygen` and as many examples as possible. Examples can be written in `docs/*.md` and comments can be written in `docs/*.dox`.
2. The project uses the `SPDX` specification open source protocol, please use the `reuse` tool to check it, refer to [here](https://spdx.org/licenses/).
