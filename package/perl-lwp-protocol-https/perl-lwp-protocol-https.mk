################################################################################
#
# perl-lwp-protocol-https
#
################################################################################

PERL_LWP_PROTOCOL_HTTPS_VERSION = 6.12
PERL_LWP_PROTOCOL_HTTPS_SOURCE = LWP-Protocol-https-$(PERL_LWP_PROTOCOL_HTTPS_VERSION).tar.gz
PERL_LWP_PROTOCOL_HTTPS_SITE = $(LINGMO_CPAN_MIRROR)/authors/id/O/OA/OALDERS
PERL_LWP_PROTOCOL_HTTPS_LICENSE = Artistic-1.0-Perl or GPL-1.0+
PERL_LWP_PROTOCOL_HTTPS_LICENSE_FILES = LICENSE
PERL_LWP_PROTOCOL_HTTPS_CPE_ID_VENDOR = lwp\:\:protocol\:\:https_project
PERL_LWP_PROTOCOL_HTTPS_CPE_ID_PRODUCT = lwp\:\:protocol\:\:https
PERL_LWP_PROTOCOL_HTTPS_DISTNAME = LWP-Protocol-https

$(eval $(perl-package))
