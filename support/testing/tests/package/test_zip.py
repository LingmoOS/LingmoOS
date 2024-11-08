from tests.package.test_compressor_base import TestCompressorBase


class TestZip(TestCompressorBase):
    __test__ = True
    config = TestCompressorBase.config + \
        """
        LINGMO_PACKAGE_BUSYBOX_SHOW_OTHERS=y
        LINGMO_PACKAGE_ZIP=y
        LINGMO_PACKAGE_UNZIP=y
        """
    compress_cmd = "/bin/sh -c 'zip $1.zip $1' /bin/sh"
    decompress_cmd = "unzip"
    check_integrity_cmd = "unzip -t"
    compressed_file_ext = ".zip"
