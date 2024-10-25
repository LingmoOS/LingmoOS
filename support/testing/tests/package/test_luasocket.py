from tests.package.test_lua import TestLuaBase


class TestLuaLuaSocket(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUASOCKET=y
        """

    def test_run(self):
        self.login()
        self.module_test("ltn12")
        self.module_test("mime")
        self.module_test("socket")


class TestLuajitLuaSocket(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUASOCKET=y
        """

    def test_run(self):
        self.login()
        self.module_test("ltn12")
        self.module_test("mime")
        self.module_test("socket")
