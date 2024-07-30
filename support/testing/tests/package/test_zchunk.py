from tests.package.test_compressor_base import TestCompressorBase


class TestZchunk(TestCompressorBase):
    __test__ = True
    config = TestCompressorBase.config + \
        """
        LINGMO_PACKAGE_ZCHUNK=y
        LINGMO_PACKAGE_ZSTD=y
        """
    compress_cmd = "zck"

    def check_integrity_test(self):
        # Do nothing for the integrity test, because "zck" does not
        # implement this feature.
        pass
