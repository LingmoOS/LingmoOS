# kernel-wedge, an industrial strength kernel splitting tool

kernel-wedge is used to generate kernel module udebs for the debian
installer.

kernel-wedge is now used as part of the build process of the linux and
kfreebsd-* source packages, but can also be used separately.

## Configuration directory layout

The default-configuration directory contains the following files:

modules/*<BR>
package-list

The environment variable `$KW_DEFCONFIG_DIR` must be set, naming the
default-configuration directory.

Each configuration directory contains the following files:

kernel-versions<BR>
modules/*arch*/\*<BR>
exclude-packages (optional)<BR>
package-list

The environment variable `$KW_CONFIG_DIR` may be set, naming the
configuration directory; otherwise the default value is "`.`".
Typically there is a separate configuration directory per
architecture, but this is not required.  The configuration directory
may be the same as the default-configuration directory.

## Use in a kernel source package

### Configuration files

The kernel-versions file lists the kernel flavours to produce packages
for.  There are 6 whitespace separated fields per line, and it looks
something like this:

    # arch version flavour installedname suffix build-depends
    i386   -       686     -             -      -
    i386   -       686-pae -             -      -

The arch column must be set to the target Debian architecture and the
flavour to the kernel flavour name that appears in the package and
file names.

The version, installedname and build-depends columns must all be set
to "`-`" in this case.

The suffix column is either `y`, `-` or `-`*suffix*; if it is `y` then
the kernel image inside the udeb will include the version and flavour
in its name, if it is `-`*suffix* then it will include this. (So will
the System.map).

The udebs are named with the version of the kernel in their package name,
since that can be useful during kernel transitions. The string `-di` is
appended to prevent any posible collisions in names with real debs.

The package-list and (optional) exclude-packages configuration files
are used by `gen-control`, as explained in its online help.

Finally, you need a modules/*arch* directory, or directories. These are
used to list the modules that go in a package. For example, for i386, we
want to have some nic drivers, so we create a modules/i386/nic-modules
listing them:

    # My short list of nic modules.
    8139too
    e100
    natsemi
    ne2k-pci
    tulip
    winbond-840
    3c59x

But for Linux, drivers are organised into subdirectories by type, and
we can instead include (most) Ethernet drivers using a wildcard:

    drivers/net/ethernet/**

Suppose we want a different set of modules in the 686 flavour kernel.
Then create a `modules/`*arch*-*flavour*`/nic-modules` instead, and it
will be used by preference. One udeb will be created for each modules
list file, containing the listed modules. The names of the files
should match the names of the various modules listed in the
package-list file in the default-configuration directory.

You will also want a special modules list file for the kernel-image udeb.
If you need to include no modules with your kernel, it can be an empty file.
but you should have a modules/*arch*/kernel-image.

`kernel-wedge` will copy the listed modules into each package. If a listed 
module does not exist, it will fail by default. Sometimes you might want to
make a module be included but it's not fatal for it not to be available.
To indicate this, follow the module with a space and a "`?`".

Module list files can also include other module lists by reference. This
works similar to cpp includes, and can be used to include module lists
distributed as part of kernel-wedge, or others. For example:

    # kernel-wedge's standard set of cdrom modules
    #include <cdrom-core-modules>
    # my own list
    #include "../../includes/cdrom-modules"

The `<name>` syntax includes a file from the modules subdirectory of the
default-configuration directory.

A final capability of the module list files is the ability to include a
module list and then override parts of it. Follow a module name with " `-`" to
remove it from the list if it was previously listed, as by an include:

    # kernel-wedge's standard set of cdrom modules
    #include <cdrom-core-modules>
    # but not this one
    sr_mod -

### Debian directory

To add the udeb packages to the control file, run for each
architecture:

`kernel-wedge gen-control `*version*` >> debian/control`

Where *version* is the kernel ABI/version that appears in package
names.

In case you use the same kernel flavour name on multiple
architectures, you will need to merge the output from multiple
invocations of `kernel-wedge gen-control`.

## Use in a separate source package

### Configuration files

The kernel-versions file looks something like this:

    # arch   version  flavour       installedname        suffix    build-depends
    i386     2.4.25-1 386           2.4.25-1-386         -         kernel-image-2.4.25-1-386, kernel-pcmcia-modules-2.4.25-1-386
    i386     2.4.24   speakup       2.4.24-speakup       -         kernel-image-2.4.24-speakup

In this case we are building two different flavours of i386 kernels.
We could just as easily be building kernels for different architectures.

The arch, flavour and suffix columns are set as above.

The version is the kernel version, or can be "`-`", in which case a
version must be specified when running the `gen-control` and
`install-files` commands.

The installedname column gives the directory/filename that the kernel and
modules are installed using in the kernel `.deb`. If it is "`-`", it's
generated from the version and flavor.

The build-depends are the package that is split up by kernel-wedge.

The other configuration files are set in the same way as normal.

### Debian directory

Your source package will be very simple, looking something like this:

debian/copyright<BR>
debian/changelog<BR>
debian/rules<BR>
debian/control.stub

Let's get the easy files out of the way. The changelog is a
changelog, like any other, as is the copyright. The debian/rules
can be something as simple as this:

    #!/usr/bin/make -f
    include /usr/share/kernel-wedge/generic-rules

Or you can use the `kernel-wedge` command directly in your own rules file.
Run it for some semblance of usage help.

`debian/control.stub` is the top of a control file. Something like this:

    Source: linux-kernel-di-i386
    Section: debian-installer
    Priority: optional
    Maintainer: Debian Install System Team <debian-boot@lists.debian.org>
    Build-Depends: kernel-wedge

The rest of the control file will be generated for you.

Once you have your source package set up, run:

	kernel-wedge gen-control > debian/control

You only need to do this once. Then build it like any other package.
Note that you will have to have the kernels listed as build dependencies
installed normally; but see "`kernel-wedge help build-all`" for an
alternative.

## Misc other info

The "`kernel-wedge copy-modules`" command does the actual copying of modules
to the temp dirs that are used to build the package. It is complicated by
the need to make sure that all of a module's dependencies are included in
its package, or in a package on which it depends. If necessary, extra
modules will be added to a package to make sure the dependencies are
satisfied.

The dependency information from modules.dep is compared with the output of
the "`kernel-wedge gen-deps`" command, which extracts dependency information
from package-lists, and formats it in a form used by tsort. If any
additional modules are needed to meet dependencies of modules in the udebs,
they will automatically be added. Since that might result in modules being
copied to multiple udebs, the "`kernel-wedge find-dups`" command is used to
scan for duplicates of modules. When you have duplicates, you need to find
(or make) a package that both packages depend on, and move the duplicate
modules to it. Alternatively, as a quick hack you can just touch a file
named ignore-dups in the root of your package and duplicate modules will
stop being a fatal error.

`SOURCEDIR` can be set to a directory containing a tree of kernel modules and
kernel image, and it will be used instead of the usual tree. That directory
will need to have a modules.dep file already generated for this to work
though -- but the modules.dep can even be copied from another system and
it'll work. This can be useful if you cannot install the required
kernel-image package, or are cross-compiling.
