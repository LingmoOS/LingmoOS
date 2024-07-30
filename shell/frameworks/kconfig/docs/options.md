KConfig Entry Options            {#options}
=====================

KConfig provides various options that can alter how it interprets configuration
entries on a per-entry, per-group or per-file basis.  Note that these are not
supported by other configuration frameworks, and so should not be used in files
that are intended to be used by applications that do not use KConfig (such as
application desktop files).


Immutable Entries
-----------------

KConfig's cascading configuration mechanism usually causes values from earlier,
"global" configuration files to be overridden by later, "local" ones.
Typically, the system administrator might set global defaults, and a user might
override them in their local configuration files.

However, KConfig provides a way to lock down configuration values, so that the
global settings override the local ones.  This allows system administrators to
restrict the values a user can set for an entry, group of entries or an entire
configuration file.

This is important for Kiosk authorization (see the KAuthorized namespace),
which allows parts of the user interface to be locked down.

Configuration entries can be marked as immutable with the `$i` option.  This can
be done on a per-entry basis:

    [MyGroup]
    someKey[$i]=42

on a per-group basis (which will prevent any attempts to modify entries in the
group at all in later files):

    [MyGroup][$i]
    someKey=42

or for an entire file by putting `[$i]` at the start of the file:

    [$i]
    [MyGroup]
    someKey=42
    [MyOtherGroup]
    someOtherKey=11

Once this is done, the immutable entries or groups cannot be overridden by later
files of the same name (and, if the file is immutable, later files will be
ignored entirely).

Note that a similar effect to file immutability can be achieved by using file
system permissions to prevent the user from writing to their local versions of
the configuration file, although (since this is normally a setup error), the
user will be warned that the configuration file is not writable.  This warning
can be suppressed by adding the following setting to either the relevant
configuration file or the `kdeglobals` file:

    [General]
    warn_unwritable_config=true

However, using file system permissions like this can potentially be circumvented
by the user if they have write access to the containing directory or can modify
environment variables (and `XDG_CONFIG_HOME` in particular).



Shell Expansion
---------------

If an entry is marked with `$e`, environment variables will be expanded.

    Name[$e]=$USER

When the "Name" entry is read `$USER` will be replaced with the value of the
`$USER` environment variable.

Note that the application will replace `$USER` with its
expanded value after saving. To prevent this combine the `$e` option
with `$i` (immmutable) option.  For example:

    Name[$ei]=$USER

This will make that the "Name" entry will always return the value of the `$USER`
environment variable. The user will not be able to change this entry.

The following syntax for environment variables is also supported:

    Name[$ei]=${USER}

There are three environment variables that have a fallback strategy if the
environment variable is not set. They instead map to a location from QStandardPaths.
They are:

* `$QT_CACHE_HOME` - QStandardPaths::GenericCacheLocation
* `$QT_CONFIG_HOME` - QStandardPaths::GenericConfigLocation
* `$QT_DATA_HOME` - QStandardPaths::GenericDataLocation
