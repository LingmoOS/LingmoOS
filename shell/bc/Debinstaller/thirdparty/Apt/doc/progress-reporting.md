Install-progress reporting 
--------------------------

If the apt options `APT::Status-Fd` is set, apt will send status
reports to that fd. The status information is separated with a '`:`',
there are the following status conditions:

* pmstatus
* dlstatus
* conffile-prompt
* error
* media-change

The reason for using a fd instead of an OpProgress class is that many
apt front-end fork a (vte) terminal for the actual installation.

The reason to do the mapping and l10n of the dpkg states to human
readable (and translatable) strings is that this way the translation
needs to be done only once for all front-ends.


pmstatus
--------
Status of the package manager (dpkg). This is send when packages
are installed/removed.
	pmstatus:pkgname:TotalPercentage:action-description

* PkgName = the name of the package
* TotalPercentage = the total progress between [0..100]
* description = a i18ned human readable description of the current action

Example:

	# ./apt-get install -o APT::Status-Fd=2 3dchess >/dev/null
	pmstatus:3dchess:20:Preparing 3dchess
	pmstatus:3dchess:40:Unpacking 3dchess
	pmstatus:3dchess:60:Preparing to configure 3dchess
	pmstatus:3dchess:80:Configuring 3dchess
	pmstatus:3dchess:100:Installed 3dchess

pmerror
-------
	pmerror:deb:TotalPercentage:error string

Example:

	pmerror: /var/cache/apt/archives/krecipes_0.8.1-0ubuntu1_i386.deb : 75% : trying to overwrite `/usr/share/doc/kde/HTML/en/krecipes/krectip.png', which is also in package krecipes-data


pmconffile
----------
	pmconffile:conffile:percent:'current-conffile' 'new-conffile' useredited distedited


media-change
------------
	media-change:medium:drive:human-readable string

Example:

	media-change: Ubuntu 5.10 _Breezy Badger_ - Alpha i386 (20050830):/cdrom/:Please insert the disc labeled: 'Ubuntu 5.10 _Breezy Badger_ - Alpha i386 (20050830)' in the drive '/cdrom/' and press enter.


dlstatus
--------
	dlstatus:AlreadDownloaded:TotalPercentage:action-description

* AlreadyDownloaded = the number of already downloaded packages
* TotalPercentage = the total progress between [0..100]
* description = a i18ned human readable description of the current action

Example:

	dlstatus:1:9.05654:Downloading file 1 of 3 (4m40s remaining)
	dlstatus:1:9.46357:Downloading file 1 of 3 (4m39s remaining)
	dlstatus:1:9.61022:Downloading file 1 of 3 (4m38s remaining)
