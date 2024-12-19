[![Build Status](https://github.com/LingmoOS/lingmo-appstore/badges/main/pipeline.svg)](https://github.com/LingmoOS/lingmo-appstore/pipelines)

# Appstore

[Appstore](https://apps.lingmo.org/Appstore) allows users to easily find,
discover and install apps. It also keeps their OS, apps and devices up to date
without them having to think about it, and gives them confidence that their
system is up to date. It supports popular distributions, subject to those
distributions maintaining their own distro-specific integration code.

The specific use cases that Appstore covers are [documented in more detail](./doc/use-cases.md).

# Features

A plugin system is used to access software from different sources.
Plugins are provided for:
 - Traditional package installation via PackageKit (e.g. Debian package, RPM).
 - Next generation packages: [Flatpak](https://flatpak.org/) and [Snap](https://snapcraft.io/).
 - Firmware updates.
 - Ratings and reviews using [ODRS](https://odrs.lingmo.org/).

Appstore supports showing metadata that closely matches the [AppStream](https://www.freedesktop.org/wiki/Distributions/AppStream/) format.

Appstore runs as a background service to provide update notifications and be a search provider for [LINGMO Shell](https://github.com/LingmoOS/lingmo-shell/).

# Contact

For questions about how to use Appstore, ask on [Discourse](https://discourse.lingmo.org/tag/lingmo-appstore).

Bug reports and merge requests should be filed on [LINGMO GitLab](https://github.com/LingmoOS/lingmo-appstore).

For development discussion, join us on `#lingmo-appstore:lingmo.org` on [Matrix](https://matrix.to/#/#lingmo-appstore:lingmo.org).

# Documentation for app developers and vendors

Specific documentation is available for app developers who wish to test
how their apps appear in LINGMO Appstore; and for distribution vendors
who wish to customise how LINGMO Appstore appears in their distribution:
 * [Tools in LINGMO Appstore for app developers](./doc/app-developers.md)
 * [Vendor customisation of LINGMO Appstore](./doc/vendor-customisation.md)

# Running a nightly build

A [flatpak bundle](https://docs.flatpak.org/en/latest/single-file-bundles.html)
of Appstore can be built on demand here by running the ‘flatpak bundle’ CI job.
It is not fully functional, but is useful for development and testing of
upcoming UI changes to Appstore. It may become more functional over time. It
is not an official or supported release.

The CI job saves the bundle in its artifacts list as `lingmo-appstore-dev.flatpak`.
This can be installed and run locally by downloading it and running:
```
$ flatpak install --bundle ./lingmo-appstore-dev.flatpak
$ flatpak run org.lingmo.AppstoreDevel
```

# Building locally

Appstore uses a number of plugins and depending on your operating system you
may want to disable or enable certain ones. For example on Fedora Silverblue
you'd want to disable the packagekit plugin as it wouldn't work. See the list
in `meson_options.txt` and use e.g. `-Dpackagekit=false` in the `meson` command
below.

Build locally with:
```
$ meson --prefix $PWD/install build/
$ ninja -C build/ all install
$ killall lingmo-appstore
# On Fedora, RHEL, etc:
$ XDG_DATA_DIRS=install/share:$XDG_DATA_DIRS LD_LIBRARY_PATH=install/lib64/:$LD_LIBRARY_PATH ./install/bin/lingmo-appstore
# On Debian, Ubuntu, etc:
$ XDG_DATA_DIRS=install/share:$XDG_DATA_DIRS LD_LIBRARY_PATH=install/lib/x86_64-linux-gnu/:$LD_LIBRARY_PATH ./install/bin/lingmo-appstore
```

# Debugging

See [the debugging documentation](./doc/debugging.md).

# Maintainers

Appstore is maintained by several co-maintainers, as listed in `lingmo-appstore.doap`.
All changes to Appstore need to be reviewed by at least one co-maintainer (who
can’t review their own changes). Larger decisions need input from at least two
co-maintainers.
