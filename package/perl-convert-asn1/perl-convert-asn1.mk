################################################################################
#
# perl-convert-asn1
#
################################################################################

PERL_CONVERT_ASN1_VERSION = 0.34
PERL_CONVERT_ASN1_SOURCE = Convert-ASN1-$(PERL_CONVERT_ASN1_VERSION).tar.gz
PERL_CONVERT_ASN1_SITE = $(LINGMO_CPAN_MIRROR)/authors/id/T/TI/TIMLEGGE
PERL_CONVERT_ASN1_LICENSE = Artistic-1.0-Perl or GPL-1.0+
PERL_CONVERT_ASN1_LICENSE_FILES = LICENSE
PERL_CONVERT_ASN1_DISTNAME = Convert-ASN1

$(eval $(perl-package))
