from tests.package.test_lua import TestLuaBase


class TestLuaLzlib(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LZLIB=y
        """

    def test_run(self):
        self.login()
        self.module_test("zlib")
        self.module_test("gzip")
