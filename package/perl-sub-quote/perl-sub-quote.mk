################################################################################
#
# perl-sub-quote
#
################################################################################

PERL_SUB_QUOTE_VERSION = 2.006008
PERL_SUB_QUOTE_SOURCE = Sub-Quote-$(PERL_SUB_QUOTE_VERSION).tar.gz
PERL_SUB_QUOTE_SITE = $(LINGMO_CPAN_MIRROR)/authors/id/H/HA/HAARG
PERL_SUB_QUOTE_LICENSE = Artistic or GPL-1.0+
PERL_SUB_QUOTE_LICENSE_FILES = LICENSE
PERL_SUB_QUOTE_DISTNAME = Sub-Quote

$(eval $(perl-package))
