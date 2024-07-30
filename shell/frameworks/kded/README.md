# KDED

Central daemon of KDE work spaces

## Introduction

KDED stands for KDE Daemon which isn't very descriptive.
KDED runs in the background and performs a number of small tasks.
Some of these tasks are built in, others are started on demand.

### Built in tasks

* Checking for newly installed software and updating ksycoca when new
  software is detected. Updating of ksycoca is done by the program kbuildsycoca
  which gets started by kded. When kded is first started it always runs
  kbuildsycoca to ensure that ksycoca is up to date.

* Checking for newly installed update files. Applications can install
  \*.upd update files. These \*.upd files are used to update configuration files
  of users, primarily when new versions of applications are installed with
  (slightly) different configuration file formats. Updating of configuration
  files is done by kconf\_update. kded starts kconf\_update when it detects a
  new update file. When kded is first started it always runs kconf\_update to
  ensure that it has not missed any update files. kconf\_update keeps track
  of which update files have been processed already in the config-file
  kconf\_updaterc. It only performs a certain update once.

### Configuration of built in tasks.

The built in tasks have some configuration options that can be changed by
editing the kdedrc configuration file. Changes need to be made with a text
editor, there is no GUI available. All options are listed under the [General]
group:

**HostnamePollInterval**: This can be used to adjust the time interval at which
the hostname is checked for changes. The time is specified in milliseconds
and has a default of 5000 (5 seconds).

**CheckSycoca**: This option can be used to disable checking for new software.
ksycoca will still be built when kded starts up and when applications
explicitly request a rebuild of the ksycoca database. The user can
also manually rebuild ksycoca by running the kbuildsycoca program.
The default value of this option is "true". Checking can be disabled by
setting this option to "false".

**CheckUpdates**: This option can be used to disable checking for update files.
kconf\_update will still be run when kded starts up.
The default value of this option is "true". Checking can be disabled by
setting this option to "false".

**CheckHostname**: This option can be used to disable checking for hostname
changes. The default value of this option is "true". Checking can be
disabled by setting this option to "false".

Example kdedrc file with default values:

    [General]
    HostnamePollInterval=5000
    CheckSycoca=true
    CheckUpdates=true
    CheckHostname=true
    CheckFileStamps=true
    DelayedCheck=false

If FAM or DNOTIFY is not available, the filesystem will be polled at regular interval for any changes. Under the [DirWatch] group in the kdeglobals file
the following options are available to adjust the polling frequency:

**PollInterval**: This can be used to adjust the time interval at which the local
filesystem is checked for new software or update files. The time is specified
in milliseconds and has a default of 500 (0.5 seconds).

**NFSPollInterval**: This can be used to adjust the time interval at which remote
filesystems, such as NFS or Samba, are ebing checked for new software or
update files. The time is specified in milliseconds and has a default of 5000
(5 seconds).

The above options are not used when FAM is used to watch for changes in the
filesystem, or when DNOTIFY is used. Specifying larger intervals may reduce
the CPU load and/or network traffic. Shorter intervals are not recommended.

Please note that in previous versions of KDE these options where listed in
the kderc file.

Example kdeglobals fragment:

    [DirWatch]
    PollInterval=500
    NFSPollInterval=5000

### KDED modules

Some things can be greatly simplified if they can be coordinated from a
central place. KDED has support for modules that will be demand-loaded
whenever an application attempts to make DBus call to the module.
This can be useful for central administration tasks.

See docs/HOWTO.
