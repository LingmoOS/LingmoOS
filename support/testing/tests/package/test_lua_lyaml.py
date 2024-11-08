from tests.package.test_lua import TestLuaBase


class TestLuaLuaLyaml(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUA_LYAML=y
        """

    def test_run(self):
        self.login()
        self.module_test("yaml")
        self.module_test("lyaml")


class TestLuajitLuaLyaml(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUA_LYAML=y
        """

    def test_run(self):
        self.login()
        self.module_test("yaml")
        self.module_test("lyaml")
