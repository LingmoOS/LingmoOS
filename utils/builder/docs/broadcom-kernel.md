# Broadcom driver for the amd64 Live kernel

CI runs `bash tools/ci/download-broadcom.sh` before preparing the build tree.
The script downloads Debian's `broadcom-sta-dkms_6.30.223.271-32_amd64.deb`
into `scripts/lingmo-config/common/packages.chroot/`. The existing copy step
then stages it in `build/lingmo-config/common/packages.chroot/`.

The package URL, version, architecture and SHA-256 are fixed. Download goes to
a temporary file; checksum and dpkg-deb metadata must match before it replaces
the destination. Any error aborts CI. Existing polkit packages are untouched.
No Debian sid/testing sources are added to the target system. The package
list still requests broadcom-sta-dkms, supplied by this newer local package.

The -26 package from Trixie failed during DKMS compilation with 6.18.62-lingmo.
The -32 source has been compiled locally against those headers to produce
wl.ko. This does not certify hardware operation or Secure Boot enrollment.
Full ISO construction and boot validation remain necessary.

For local builds, run the same script before copying scripts/ into the build
workspace. An optional argument changes the staging directory for tests.
The download is intentionally not checked into Git. Update the version, URL
and hash together when changing drivers. Only amd64 CI is covered.

Validation: `python3 tests/test_broadcom_download.py` covers step ordering,
repeat staging, failed downloads, hash/metadata mismatch, temporary cleanup
and preservation of an existing package. No root or network is used by tests.

## Target toolchain and failed-build evidence

Run 43172239517 selected the staged -32 package correctly, but its chroot
package log contains cpp-14 and gcc-14-base, not the GCC compiler. DKMS 3.2.2
only Recommends `gcc | c-compiler`; live-build is configured with
`--apt-recommends false`. The headers list therefore explicitly includes
build-essential and, for amd64's 6.18.62-lingmo kernel, gcc-14. Installing a
compiler in the outer CI container does not install one in the target chroot.

After a failure CI collects build.log, apt/dpkg logs, DKMS make.log files and
compiler/package version diagnostics into the build-diagnostics artifact.
It does not collect DKMS signing keys. This is needed to establish the precise
compiler error if a new build still fails; the previous run did not retain it.
`python3 tests/test_build_diagnostics.py` exercises collection and toolchain
configuration without using the host package manager or a real chroot.
