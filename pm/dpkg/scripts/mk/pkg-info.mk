# This Makefile fragment (since dpkg 1.16.1) defines the following package
# information variables:
#
# For a source package with version 1:2.3-4, the output for the following
# variables is shown as an example, although notice that the epoch and
# revision parts can be omitted depending on the packaging.
#
#   DEB_SOURCE: Source package name.
#   DEB_VERSION: Package's full version.
#     Format: [epoch:]upstream-version[-revision]
#     Example: 1:2.3-4
#   DEB_VERSION_EPOCH: Package's epoch without the upstream-version or the
#     Debian revision (since dpkg 1.22.12). The variable can be empty if
#     there is no epoch.
#     Format: [epoch]
#     Example: 1
#   DEB_VERSION_EPOCH_UPSTREAM: Package's version without the Debian revision.
#     Format: [epoch:]upstream-version
#     Example: 1:2.3
#   DEB_VERSION_UPSTREAM_REVISION: Package's version without the Debian epoch.
#     Format: upstream-version[-revision]
#     Example: 2.3-4
#   DEB_VERSION_UPSTREAM: Package's upstream version without the Debian epoch
#     or revision.
#     Format: upstream-version
#     Example: 2.3
#   DEB_VERSION_REVISION: Package's Debian revision without the Debian epoch
#     or the upstream-version (since dpkg 1.22.12). The variable can be empty
#     if there is no revision.
#     Format: [revision]
#     Example: 4
#   DEB_DISTRIBUTION: Distribution(s) listed in the current debian/changelog
#     entry.
#   DEB_TIMESTAMP: Source package release date as seconds since the epoch as
#     specified in the latest debian/changelog entry (since dpkg 1.22.9),
#     although you are probably looking for SOURCE_DATE_EPOCH instead.
#
#   SOURCE_DATE_EPOCH: Source release date as seconds since the epoch, as
#     specified by <https://reproducible-builds.org/specs/source-date-epoch/>
#     (since dpkg 1.18.8).
#     If it is undefined, the date of the latest changelog entry is used.
#     In both cases, the value is exported.
#
# Note:
# - Only documented variables are considered public interfaces.
# - Expects to be included from the source tree root directory.

ifndef dpkg_pkg_info_mk_included
dpkg_pkg_info_mk_included = yes

dpkg_late_eval ?= $(if $(filter undefined,$(flavor DPKG_CACHE_$(1))),$(eval DPKG_CACHE_$(1) := $(shell $(2)))$(value DPKG_CACHE_$(1)),$(value DPKG_CACHE_$(1)))

DEB_SOURCE = $(call dpkg_late_eval,DEB_SOURCE,dpkg-parsechangelog -SSource)
DEB_VERSION = $(call dpkg_late_eval,DEB_VERSION,dpkg-parsechangelog -SVersion)
DEB_VERSION_EPOCH = $(call dpkg_late_eval,DEB_VERSION_EPOCH,echo '$(DEB_VERSION)' | sed -e 's/^\([0-9]\):.*$$/\1/')
DEB_VERSION_EPOCH_UPSTREAM = $(call dpkg_late_eval,DEB_VERSION_EPOCH_UPSTREAM,echo '$(DEB_VERSION)' | sed -e 's/-[^-]*$$//')
DEB_VERSION_UPSTREAM_REVISION = $(call dpkg_late_eval,DEB_VERSION_UPSTREAM_REVISION,echo '$(DEB_VERSION)' | sed -e 's/^[0-9]*://')
DEB_VERSION_UPSTREAM = $(call dpkg_late_eval,DEB_VERSION_UPSTREAM,echo '$(DEB_VERSION_EPOCH_UPSTREAM)' | sed -e 's/^[0-9]*://')
DEB_VERSION_REVISION = $(call dpkg_late_eval,DEB_VERSION_REVISION,echo '$(DEB_VERSION)' | sed -e 's/^.*-\([^-]*\)$$/\1/')
DEB_DISTRIBUTION = $(call dpkg_late_eval,DEB_DISTRIBUTION,dpkg-parsechangelog -SDistribution)
DEB_TIMESTAMP = $(call dpkg_late_eval,DEB_TIMESTAMP,dpkg-parsechangelog -STimestamp)

SOURCE_DATE_EPOCH ?= $(DEB_TIMESTAMP)
export SOURCE_DATE_EPOCH

endif # dpkg_pkg_info_mk_included
