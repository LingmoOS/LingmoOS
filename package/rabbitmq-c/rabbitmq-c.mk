################################################################################
#
# rabbitmq-c
#
################################################################################

RABBITMQ_C_VERSION = 0.13.0
RABBITMQ_C_SITE = $(call github,alanxz,rabbitmq-c,v$(RABBITMQ_C_VERSION))
RABBITMQ_C_LICENSE = MIT
RABBITMQ_C_LICENSE_FILES = LICENSE README.md
RABBITMQ_C_CPE_ID_VALID = YES
RABBITMQ_C_INSTALL_STAGING = YES
RABBITMQ_C_CONF_OPTS = \
	-DBUILD_API_DOCS=OFF \
	-DBUILD_TOOLS_DOCS=OFF

# 0001-Add-option-to-read-username-password-from-file.patch
RABBITMQ_C_IGNORE_CVES += CVE-2023-35789

# BUILD_SHARED_LIBS is handled in pkg-cmake.mk as it is a generic cmake variable
ifeq ($(LINGMO_SHARED_STATIC_LIBS),y)
RABBITMQ_C_CONF_OPTS += -DBUILD_STATIC_LIBS=ON
else ifeq ($(LINGMO_SHARED_LIBS),y)
RABBITMQ_C_CONF_OPTS += -DBUILD_STATIC_LIBS=OFF
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
RABBITMQ_C_CONF_OPTS += -DENABLE_SSL_SUPPORT=ON
RABBITMQ_C_DEPENDENCIES += openssl
else
RABBITMQ_C_CONF_OPTS += -DENABLE_SSL_SUPPORT=OFF
endif

ifeq ($(LINGMO_PACKAGE_POPT),y)
RABBITMQ_C_CONF_OPTS += -DBUILD_TOOLS=ON
RABBITMQ_C_DEPENDENCIES += popt
else
RABBITMQ_C_CONF_OPTS += -DBUILD_TOOLS=OFF
endif

$(eval $(cmake-package))
