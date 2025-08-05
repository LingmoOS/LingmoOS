/* Placeholder stubs.h file for bootstrapping.

   When bootstrapping a GCC/GLIBC pair, GCC requires that the GLIBC
   headers be installed, but we can't fully build GLIBC without that
   GCC.  So we run the command:

      make install-headers install-bootstrap-headers=yes

   to install the headers GCC needs, but avoid building certain
   difficult headers.  The <gnu/stubs.h> header depends, via the
   GLIBC subdir 'stubs' make targets, on every .o file in GLIBC, but
   an empty stubs.h like this will do fine for GCC.  */
