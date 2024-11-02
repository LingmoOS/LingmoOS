TEMPLATE = subdirs

!system_libpng {
	SUBDIRS += zlib libpng
	libpng.depends += zlib
}
