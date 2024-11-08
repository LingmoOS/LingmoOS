from tests.package.test_lua import TestLuaBase


class TestLuaLuaExpat(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUAEXPAT=y
        """

    def test_run(self):
        self.login()
        self.module_test("lxp")


class TestLuajitLuaExpat(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUAEXPAT=y
        """

    def test_run(self):
        self.login()
        self.module_test("lxp")
