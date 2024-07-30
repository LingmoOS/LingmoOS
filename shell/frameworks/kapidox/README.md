# KDE Doxygen Tools

## Introduction

This framework contains scripts and data for building API documentation (dox) in
a standard format and style.

https://api.kde.org holds the result, and this very page is handled by KApiDox too: https://api.kde.org/frameworks/kapidox/html/index.html.

The [Doxygen](https://www.doxygen.nl) tool performs the actual documentation extraction and
formatting. This framework provides a wrapper script to make generating the
documentation more convenient (including reading settings from the target
framework or other module) and a standard template for the generated
documentation.

## Installation
Python 3 is required to run the scripts, as well as the `jinja`, `pyyaml`, `requests`, `doxypypy` and `doxyqml` python modules.

The following command creates a venv and installs the tool alongside all its modules:

```
bash ./bootstrap-devenv.sh
```

To generate the dependency diagrams, you need the Graphviz Python bindings.
They are currently not available from pip, but most distributions provide them.
You can get binaries and source archives from
<https://www.graphviz.org/download/>.

## (For maintainers) updating the package dependencies

Run ./requirements-update.sh in this folder, review and test the updated requirements file and commit the changed file.

## Workflow

This document describes two ways to use KApiDox to generate documentation for KDE software: the manual way, and the container way. Both can apply to standalone repositories or to projects built using [kdesrc-build](https://community.kde.org/Get_Involved/development), but the main role of the manual method is mostly to learn how the tool works, whereas the container method should be the cleaner, more convenient way.

### The manual way

With this method, we will be using three directories (kirigami, kapidox and kirigamidocs).

First, clone the desired repository as well as the KApiDox repository. In this example we will use Kirigami:

```bash
git clone https://invent.kde.org/frameworks/kirigami.git
git clone https://invent.kde.org/frameworks/kapidox.git
```

You may use the ssh:// version instead if you have a developer account.

You should now have a `kirigami` and a `kapidox` folder. Make a new folder to store the local test HTML documentation, then switch to the kapidox directory:

```bash
mkdir kirigamidocs
cd kapidox
```

Run the script provided by the KApiDox repository. This only needs to be done once, and it will download and install the needed Python3 modules and generate a proper Python virtual environment:

```bash
bash bootstrap-devenv.sh
```

After the script is done, you should have a hidden folder called `.kapidox_venv/`, inside of which is a `bin/` folder containing the files we will be using.

If the shell you are using is bash or zsh, you can run `source .kapidox_venv/bin/activate` to activate the Python virtual venv. If you are using csh or fish, you may use `source .kapidox_venv/bin/activate.csh` or `source .kapidox_venv/bin/activate.fish`, respectively.

After activating the Python venv, your terminal prompt should change to that of a venv. Switch to the folder we created earlier and run kapidox-generate by pointing to the folder containing the target project repository (in this case, Kirigami):

```bash
cd ../kirigamidocs
kapidox-generate ../kirigami
```

It can take a while. After it is done, for convenience, do not close this venv shell yet.

You should have an `index.html` file which should be the entrypoint of the new documentation. You can open a new instance of your terminal or another tab, then open it directly from your terminal by using `xdg-open index.html` or `kioclient exec index.html`, or whichever other method you'd like.

After that, you may edit the Doxygen-formatted documentation in *.qml or *.h files using Kate or your favorite editor.

After you're finished editing, you can switch back to the venv in the `kirigamidocs/` folder, check that you are in the correct folder with `pwd`, and run the following to regenerate the locally edited documentation:

```bash
pwd # Outputs e.g. ~/kirigamidocs
rm -rf * && kapidox-generate ../kirigami
```

Now that you understand how to generate the venv and how to use `kapidox-generate`, you can easily apply this knowledge to projects built using kdesrc-build. Assuming you're using bash:

```bash
kdesrc-build kapidox
// ...building...
kdesrc-build kirigami
// ...building...
mkdir ~/kirigamidocs
cd ~/kde/src/kapidox
bash bootstrap-devenv.sh
source .kapidox_venv/bin/activate
cd ~/kirigamidocs
kapidox-generate ~/kde/src/kirigami
// ...edit files...
rm -rf * && kapidox-generate ~/kde/src/kirigami
```

### The container way

Although it is possible to use KApiDox directly, using it in a container can be much more convenient once it is set up.

You can build a new image with docker or podman:

```bash
docker build . -t kapidox_generate:latest
// OR
podman build . -t kapidox_generate:latest
```

By running `docker images` or `podman images` you should now see an image called `localhost/kapidox_generate`.

To run `kapidox-generate` with a project that you want to generate the docs from you need an empty folder for the results (`/path/to/build/folder`). The build directory inside the container is set as `BUILD_DIR` in `Dockerfile`, and must stay in sync with what is used as `CONTAINER_BUILD_DIR` in the volume mapping.

For this example we use `libksane` checked out to `/path/to/libksane`:

```bash
export HOST_PROJECT_SRC=/path/to/libksane/folder
export HOST_BUILD_DIR=/path/to/libksane/build/folder
export CONTAINER_PROJECT_SRC=/home/kapidox/libksane
export CONTAINER_BUILD_DIR=/home/kapidox/apidox-build
mkdir $HOST_BUILD_DIR
docker run \
    --volume $HOST_PROJECT_SRC:$CONTAINER_PROJECT_SRC \
    --volume $HOST_BUILD_DIR:$CONTAINER_BUILD_DIR \
    kapidox_generate:latest \
    kapidox-generate $CONTAINER_PROJECT_SRC
```

## Writing documentation

Writing dox is beyond the scope of this documentation -- see the notes at
<https://community.kde.org/Frameworks/Frameworks_Documentation_Policy> and the [doxygen
manual](https://doxygen.nl/manual/docblocks.html).

To allow code to handle the case of being processed by kapidox a C/C++ preprocessor macro
is set as defined when run: `K_DOXYGEN` (since v5.67.0).
For backward-compatibility the definition `DOXYGEN_SHOULD_SKIP_THIS` is also set, but
its usage is deprecated.

The kapidox scripts expects certain things to be present in the directory it is
run on:

### README.md
Most importantly, there should be a `README.md` file that works as the main page, like one where this documentation is written in (backward compatibility also authorize `Mainpage.dox` files).  The first line of this file
is particularly important, as it will be used as the title of the documentation.

### metainfo.yaml
A `metainfo.yaml` file is needed for the library to be generated. It should
contain some meta information about the library itself, its maintainers, where
the sources are, etc.

A very simple example can be:

```yaml
# metainfo.yaml
description: Library doing X
maintainer: gwashington
public_lib: true
logo: libX.png
```

A comprehensive list of the available keys can be found on
[this page](@ref metainfo_syntax).

By default, the source of the public library must be in `src`, if the
documentation refers to any dot files, these should be in `docs/dot`.
Images should be in `docs/pics`, and snippets of example code should be in
`examples`.  See the [Doxygen documentation](https://doxygen.nl/manual/index.html) for help on how to refer to these
files from the dox comments in the source files.

If you need to override any Doxygen settings, put them in a `docs/Doxyfile.local` in your project.
Global settings are defined in `src/kapidox/data/Doxyfile.global`.

## Generating the documentation

The tool for generating dox is `src/kapidox_generate`.
Change to an empty directory, then simply point it at the
folder you want to generate dox for (such as a frameworks checkout).

For example, if you have a checkout of KCoreAddons at
~/kde/src/frameworks/kcoreaddons, you could run

    ~/kde/src/frameworks/kapidox/src/kapidox_generate ~/kde/src/frameworks/kcoreaddons

and it would create a documentation in the current directory, which needs to be empty before executing the command.

kapidox recursively walks through folders, so you can also run it on
`~/kde/src/frameworks` or `~/src`. For a lot of libraries, the generation can last
15-30 minutes and use several hundreds of MB, so be prepared!

Pass the --help argument to see options that control the behaviour of the
script.

Note that on Windows, you will need to run something like

    c:\python\python.exe c:\frameworks\kapidox\src\kapidox_generate c:\frameworks\kcoreaddons

## Specific to frameworks (for now)

You can ask `kgenframeworksapidox` to generate dependency diagrams for all the
frameworks.  To do so, you must first generate Graphviz .dot files for all
frameworks with the `depdiagram-prepare` tool, like this:

    mkdir dot
    ~/kde/src/frameworks/kapidox/src/depdiagram-prepare --all ~/kde/src/frameworks dot

Then call `kgenframeworksapidox` with the `--depdiagram-dot-dir` option, like
this:

    mkdir frameworks-apidocs
    cd frameworks-apidocs
    ~/kde/src/frameworks/kapidox/src/kapidox_generate --depdiagram-dot-dir ../dot ~/kde/src/frameworks

More fine-grained tools are available for dependency diagrams. You can learn
about them in [depdiagrams](@ref depdiagrams).


## Examples of generated pages:

- KDE API documentation: <https://api.kde.org/>

## Licensing

This project is licensed under BSD-2-Clause. But the specific theme used inside KDE
is licensed under AGPL-3.0-or-later. If you find the AGPL too restrictive you can
alternatively use the theme from [Docsy](https://github.com/google/docsy) (APACHE-2.0).
For that you need to replace the style and js script present in `src/kapidox/data/templates/base.html`.

If you need support or licensing clarification, feel free to contact the maintainers.
