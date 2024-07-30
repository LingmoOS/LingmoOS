from tests.package.test_lua import TestLuaBase


class TestLuaLuacURL(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUA_CURL=y
        """

    def test_run(self):
        self.login()
        self.module_test("cURL")
        self.module_test("lcurl")


class TestLuajitLuacURL(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUA_CURL=y
        """

    def test_run(self):
        self.login()
        self.module_test("cURL")
        self.module_test("lcurl")
