import os
import infra

BASIC_CONFIG = \
    """
    LINGMO_TARGET_ROOTFS_CPIO=y
    # LINGMO_TARGET_ROOTFS_TAR is not set
    """


def has_broken_links(path):
    for root, dirs, files in os.walk(path):
        for f in files:
            fpath = os.path.join(root, f)
            if not os.path.exists(fpath):
                return True
    return False


class TestExternalToolchain(infra.basetest.BRTest):
    def common_check(self):
        # Check for broken symlinks
        for d in ["lib", "usr/lib"]:
            path = os.path.join(self.builddir, "staging", d)
            self.assertFalse(has_broken_links(path))
            path = os.path.join(self.builddir, "target", d)
            self.assertFalse(has_broken_links(path))

        with open(os.path.join(self.builddir, ".config"), 'r') as configf:
            configlines = [line.strip() for line in configf.readlines()]

        if "LINGMO_BINFMT_ELF=y" in configlines:
            interp = infra.get_elf_prog_interpreter(self.builddir,
                                                    self.toolchain_prefix,
                                                    "bin/busybox")
            interp_path = os.path.join(self.builddir, "target", interp[1:])
            self.assertTrue(os.path.exists(interp_path))


class TestExternalToolchainLinaroArm(TestExternalToolchain):
    config = BASIC_CONFIG + \
        """
        LINGMO_arm=y
        LINGMO_cortex_a8=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_LINARO_ARM=y
        """
    toolchain_prefix = "arm-linux-gnueabihf"

    def test_run(self):
        TestExternalToolchain.common_check(self)

        # Check the architecture variant
        arch = infra.get_file_arch(self.builddir,
                                   self.toolchain_prefix,
                                   "lib/libc.so.6")
        self.assertEqual(arch, "v7")
        isa = infra.get_elf_arch_tag(self.builddir,
                                     self.toolchain_prefix,
                                     "lib/libc.so.6",
                                     "Tag_THUMB_ISA_use")
        self.assertEqual(isa, "Thumb-2")

        # Boot the system
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", img])
        self.emulator.login()


class TestExternalToolchainBuildrootMusl(TestExternalToolchain):
    config = BASIC_CONFIG + \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_CUSTOM=y
        LINGMO_TOOLCHAIN_EXTERNAL_DOWNLOAD=y
        LINGMO_TOOLCHAIN_EXTERNAL_URL="http://autobuild.buildroot.org/toolchains/tarballs/br-arm-cortex-a9-musl-2017.05-1078-g95b1dae.tar.bz2"
        LINGMO_TOOLCHAIN_EXTERNAL_GCC_6=y
        LINGMO_TOOLCHAIN_EXTERNAL_HEADERS_4_12=y
        LINGMO_TOOLCHAIN_EXTERNAL_CUSTOM_MUSL=y
        LINGMO_TOOLCHAIN_EXTERNAL_CXX=y
        """
    toolchain_prefix = "arm-linux"

    def test_run(self):
        TestExternalToolchain.common_check(self)
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", img])
        self.emulator.login()


class TestExternalToolchainCtngMusl(TestExternalToolchain):
    config = BASIC_CONFIG + \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_CUSTOM=y
        LINGMO_TOOLCHAIN_EXTERNAL_DOWNLOAD=y
        LINGMO_TOOLCHAIN_EXTERNAL_URL="http://autobuild.buildroot.net/toolchains/tarballs/arm-ctng-linux-musleabihf.tar.xz"
        LINGMO_TOOLCHAIN_EXTERNAL_CUSTOM_PREFIX="arm-ctng-linux-musleabihf"
        LINGMO_TOOLCHAIN_EXTERNAL_GCC_7=y
        LINGMO_TOOLCHAIN_EXTERNAL_HEADERS_3_10=y
        LINGMO_TOOLCHAIN_EXTERNAL_CUSTOM_MUSL=y
        LINGMO_TOOLCHAIN_EXTERNAL_CXX=y
        """
    toolchain_prefix = "arm-ctng-linux-musleabihf"

    def test_run(self):
        TestExternalToolchain.common_check(self)
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", img])
        self.emulator.login()


class TestExternalToolchainBuildrootuClibc(TestExternalToolchain):
    config = BASIC_CONFIG + \
        """
        LINGMO_arm=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_CUSTOM=y
        LINGMO_TOOLCHAIN_EXTERNAL_DOWNLOAD=y
        LINGMO_TOOLCHAIN_EXTERNAL_URL="http://autobuild.buildroot.org/toolchains/tarballs/br-arm-full-2017.05-1078-g95b1dae.tar.bz2"
        LINGMO_TOOLCHAIN_EXTERNAL_GCC_4_9=y
        LINGMO_TOOLCHAIN_EXTERNAL_HEADERS_3_10=y
        LINGMO_TOOLCHAIN_EXTERNAL_LOCALE=y
        # LINGMO_TOOLCHAIN_EXTERNAL_HAS_THREADS_DEBUG is not set
        LINGMO_TOOLCHAIN_EXTERNAL_CXX=y
        """
    toolchain_prefix = "arm-linux"

    def test_run(self):
        TestExternalToolchain.common_check(self)
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", img])
        self.emulator.login()


class TestExternalToolchainCCache(TestExternalToolchainBuildrootuClibc):
    extraconfig = \
        """
        LINGMO_CCACHE=y
        LINGMO_CCACHE_DIR="{builddir}/ccache-dir"
        """

    def __init__(self, names):
        super(TestExternalToolchainBuildrootuClibc, self).__init__(names)
        self.config += self.extraconfig.format(builddir=self.builddir)
