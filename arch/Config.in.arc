choice
	prompt "Target CPU"
	default LINGMO_arc770d
	depends on LINGMO_arc
	help
	    Specific CPU to use

config LINGMO_arc750d
	bool "ARC 750D"

config LINGMO_arc770d
	bool "ARC 770D"

config LINGMO_archs38
	bool "ARC HS38"
	help
	  Generic ARC HS capable of running Linux, i.e. with MMU,
	  caches and 32-bit multiplier. Also it corresponds to the
	  default configuration in older GNU toolchain versions.

config LINGMO_archs38_64mpy
	bool "ARC HS38 with 64-bit mpy"
	help
	  Fully featured ARC HS capable of running Linux, i.e. with
	  MMU, caches and 64-bit multiplier.

	  If you're not sure which version of ARC HS core you build
	  for use this one.

config LINGMO_archs38_full
	bool "ARC HS38 with Quad MAC & FPU"
	help
	  Fully featured ARC HS with additional support for
	   - Dual- and quad multiply and MC oprations
	   - Double-precision FPU

	  It corresponds to "hs38_slc_full" ARC HS template in
	  ARChitect.

config LINGMO_archs4x_rel31
	bool "ARC HS48 rel 31"
	help
	   Build for HS48 release 3.1

config LINGMO_archs4x
	bool "ARC HS48"
	help
	   Latest release of HS48 processor
	   - Dual and Quad multiply and MAC operations
	   - Double-precision FPU

endchoice

# Choice of atomic instructions presence
config LINGMO_ARC_ATOMIC_EXT
	bool "Atomic extension (LLOCK/SCOND instructions)"
	default y if LINGMO_arc770d
	default y if LINGMO_archs38 || LINGMO_archs38_64mpy || LINGMO_archs38_full
	default y if LINGMO_archs4x_rel31 || LINGMO_archs4x

config LINGMO_ARCH
	default "arc"	if LINGMO_arcle
	default "arceb"	if LINGMO_arceb

config LINGMO_NORMALIZED_ARCH
	default "arc"

config LINGMO_arc
	bool
	default y if LINGMO_arcle || LINGMO_arceb

config LINGMO_ENDIAN
	default "LITTLE" if LINGMO_arcle
	default "BIG"	 if LINGMO_arceb

config LINGMO_GCC_TARGET_CPU
	default "arc700" if LINGMO_arc750d
	default "arc700" if LINGMO_arc770d
	default "archs"	 if LINGMO_archs38
	default "hs38"	 if LINGMO_archs38_64mpy
	default "hs38_linux"	 if LINGMO_archs38_full
	default "hs4x_rel31"	 if LINGMO_archs4x_rel31
	default "hs4x"	 if LINGMO_archs4x

config LINGMO_READELF_ARCH_NAME
	default "ARCompact"	if LINGMO_arc750d || LINGMO_arc770d
	default "ARCv2"		if LINGMO_archs38 || LINGMO_archs38_64mpy || LINGMO_archs38_full
	default "ARCv2"		if LINGMO_archs4x_rel31 || LINGMO_archs4x

choice
	prompt "MMU Page Size"
	default LINGMO_ARC_PAGE_SIZE_8K
	help
	  MMU starting from version 3 (found in ARC 770) and now
	  version 4 (found in ARC HS38) allows the selection of the
	  page size during ASIC design creation.

	  The following options are available for MMU v3 and v4: 4kB,
	  8kB and 16 kB.

	  The default is 8 kB (that really matches the only page size
	  in MMU v2).  It is important to build a toolchain with page
	  size matching the hardware configuration. Otherwise
	  user-space applications will fail at runtime.

config LINGMO_ARC_PAGE_SIZE_4K
	bool "4KB"
	depends on !LINGMO_arc750d

config LINGMO_ARC_PAGE_SIZE_8K
	bool "8KB"
	help
	  This is the one and only option available for MMUv2 and
	  default value for MMU v3 and v4.

config LINGMO_ARC_PAGE_SIZE_16K
	bool "16KB"
	depends on !LINGMO_arc750d

endchoice

config LINGMO_ARC_PAGE_SIZE
	string
	default "4K" if LINGMO_ARC_PAGE_SIZE_4K
	default "8K" if LINGMO_ARC_PAGE_SIZE_8K
	default "16K" if LINGMO_ARC_PAGE_SIZE_16K

# vim: ft=kconfig
# -*- mode:kconfig; -*-
