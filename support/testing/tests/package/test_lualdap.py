from tests.package.test_lua import TestLuaBase


class TestLuaLuaLdap(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUALDAP=y
        """

    def test_run(self):
        self.login()
        self.module_test("lualdap")


class TestLuajitLuaLdap(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUALDAP=y
        """

    def test_run(self):
        self.login()
        self.module_test("lualdap")
