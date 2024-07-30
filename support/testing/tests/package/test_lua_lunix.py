from tests.package.test_lua import TestLuaBase


class TestLuaLunix(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUA_LUNIX=y
        """

    def test_run(self):
        self.login()
        self.module_test("unix")


class TestLuajitLunix(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUA_LUNIX=y
        """

    def test_run(self):
        self.login()
        self.module_test("unix")
