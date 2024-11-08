from tests.package.test_lua import TestLuaBase


class TestLuaLuaossl(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUAOSSL=y
        """

    def test_run(self):
        self.login()
        self.module_test("openssl")
        self.module_test("_openssl")


class TestLuajitLuaossl(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUAOSSL=y
        """

    def test_run(self):
        self.login()
        self.module_test("openssl")
        self.module_test("_openssl")
