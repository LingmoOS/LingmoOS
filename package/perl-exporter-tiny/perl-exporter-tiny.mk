################################################################################
#
# perl-exporter-tiny
#
################################################################################

PERL_EXPORTER_TINY_VERSION = 1.006002
PERL_EXPORTER_TINY_SOURCE = Exporter-Tiny-$(PERL_EXPORTER_TINY_VERSION).tar.gz
PERL_EXPORTER_TINY_SITE = $(LINGMO_CPAN_MIRROR)/authors/id/T/TO/TOBYINK
PERL_EXPORTER_TINY_LICENSE = Artistic or GPL-1.0+
PERL_EXPORTER_TINY_LICENSE_FILES = LICENSE
PERL_EXPORTER_TINY_DISTNAME = Exporter-Tiny

$(eval $(perl-package))
