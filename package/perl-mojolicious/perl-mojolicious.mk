################################################################################
#
# perl-mojolicious
#
################################################################################

PERL_MOJOLICIOUS_VERSION = 9.35
PERL_MOJOLICIOUS_SOURCE = Mojolicious-$(PERL_MOJOLICIOUS_VERSION).tar.gz
PERL_MOJOLICIOUS_SITE = $(LINGMO_CPAN_MIRROR)/authors/id/S/SR/SRI
PERL_MOJOLICIOUS_LICENSE = Artistic-2.0
PERL_MOJOLICIOUS_LICENSE_FILES = LICENSE
PERL_MOJOLICIOUS_DISTNAME = Mojolicious

$(eval $(perl-package))
