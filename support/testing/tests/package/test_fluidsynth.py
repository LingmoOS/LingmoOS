import os

import infra.basetest


class TestFluidsynth(infra.basetest.BRTest):
    # infra.basetest.BASIC_TOOLCHAIN_CONFIG cannot be used as it is
    # armv5 and based on qemu versatilepb which is limited to 256MB of
    # RAM.  The test needs 1GB of RAM (larger initrd and soundfont is
    # loaded in memory).
    config = \
        """
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="5.15.86"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_PACKAGE_AUBIO=y
        LINGMO_PACKAGE_FLUIDSYNTH=y
        LINGMO_PACKAGE_FLUIDSYNTH_LIBSNDFILE=y
        LINGMO_PACKAGE_FLUID_SOUNDFONT=y
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_MIDIUTIL=y
        LINGMO_ROOTFS_OVERLAY="{}"
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """.format(
           # overlay to add helper test scripts
           infra.filepath("tests/package/test_fluidsynth/rootfs-overlay"))

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["console=ttyAMA0"],
                           options=["-M", "virt", "-cpu", "cortex-a57", "-m", "1G", "-initrd", img])
        self.emulator.login()

        # Test the binary executes
        self.assertRunOk("fluidsynth --version")

        # Create a simple MIDI file programmatically
        self.assertRunOk("/root/gen_midi_file.py /tmp/output.mid")

        # Convert the MIDI file to a WAV file
        cmd = "fluidsynth"
        cmd += " -F /tmp/output.wav"
        cmd += " /usr/share/soundfonts/FluidR3_GM.sf2"
        cmd += " /tmp/output.mid"
        self.assertRunOk(cmd)

        # Extract notes in the WAV file with Aubio
        self.assertRunOk("aubionotes /tmp/output.wav > /tmp/notes.txt")

        # Check the extracted notes are the expected ones
        self.assertRunOk("/root/check_notes.py < /tmp/notes.txt")
