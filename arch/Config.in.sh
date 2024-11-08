choice
	prompt "Target Architecture Variant"
	default LINGMO_sh4
	depends on LINGMO_sh
	help
	  Specific CPU variant to use

config LINGMO_sh4
	bool "sh4 (SH4 little endian)"
config LINGMO_sh4eb
	bool "sh4eb (SH4 big endian)"
config LINGMO_sh4a
	bool "sh4a (SH4A little endian)"
config LINGMO_sh4aeb
	bool "sh4aeb (SH4A big endian)"
endchoice

config LINGMO_ARCH
	default "sh4"		if LINGMO_sh4
	default "sh4eb"		if LINGMO_sh4eb
	default "sh4a"		if LINGMO_sh4a
	default "sh4aeb"	if LINGMO_sh4aeb

config LINGMO_NORMALIZED_ARCH
	default "sh"

config LINGMO_ENDIAN
	default "LITTLE"	if LINGMO_sh4 || LINGMO_sh4a
	default "BIG"		if LINGMO_sh4eb || LINGMO_sh4aeb

config LINGMO_READELF_ARCH_NAME
	default "Renesas / SuperH SH"

# vim: ft=kconfig
# -*- mode:kconfig; -*-
