# lpm-contrib

This repository contains contributed scripts to lpm.

*Note*: These existed in lpm.git, but were moved out to ease maintenance.

## How to build

```sh
./autogen.sh
./configure --prefix=/usr \
            --sysconfdir=/etc \
            --localstatedir=/var
make
make check
make install DESTDIR="$pkgdir"
```

## Scripts available in this repository

- checkupdates - print a list of pending updates without touching the system
  sync databases (for safety on rolling release distributions).

- paccache - a flexible package cache cleaning utility that allows greater
  control over which packages are removed.

- pacdiff - a simple pacnew/pacsave updater for /etc/.

- paclist - list all packages installed from a given repository. Useful for
  seeing which packages you may have installed from the testing repositories,
  for instance.

- paclog-pkglist - list currently installed packages based lpm's log.

- pacscripts - print out the {pre,post}\_{install,remove,upgrade}
  scripts of a given package.

- pacsearch - a colorized search combining both -Ss and -Qs output. Installed
  packages are easily identified with a `[installed]`, and
  local-only packages are also listed.

- pacsort - concatenate the given files, sort them, and write them to standard
  output.

- pactree - a package dependency tree viewer.

- rankmirrors - rank lpm mirrors by their connection and opening speed.

- updpkgsums - perform an in-place update of the checksums in a build.lpm.
