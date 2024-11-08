import os

import infra.basetest


class TestTesseractOcr(infra.basetest.BRTest):
    config = infra.basetest.BASIC_TOOLCHAIN_CONFIG + \
        """
        LINGMO_PACKAGE_FREETYPE=y
        LINGMO_PACKAGE_GHOSTSCRIPT_FONTS=y
        LINGMO_PACKAGE_GRAPHICSMAGICK=y
        LINGMO_PACKAGE_TESSERACT_OCR=y
        LINGMO_PACKAGE_TESSERACT_OCR_LANG_ENG=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv5",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()

        msg = "Hello from Buildroot runtime test."
        img_file = "image.pgm"
        txt_basename = "text"
        txt_file = f"{txt_basename}.txt"

        # Check the program execute.
        self.assertRunOk("tesseract --version")

        # Generate an image file including a text message.
        cmd = f"gm convert -pointsize 16 label:'{msg}' {img_file}"
        self.assertRunOk(cmd)

        # Perform the character recognition.
        cmd = f"tesseract {img_file} {txt_basename}"
        self.assertRunOk(cmd, timeout=30)

        # Check the decoded text matches the original message.
        cmd = f"grep -F '{msg}' {txt_file}"
        self.assertRunOk(cmd)
