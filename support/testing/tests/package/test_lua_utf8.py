from tests.package.test_lua import TestLuaBase


class TestLuaUtf8(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUA_UTF8=y
        """

    def test_run(self):
        self.login()
        self.module_test("lua-utf8")


class TestLuajitUtf8(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUA_UTF8=y
        """

    def test_run(self):
        self.login()
        self.module_test("lua-utf8")
