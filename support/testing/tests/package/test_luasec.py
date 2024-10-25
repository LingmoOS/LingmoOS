from tests.package.test_lua import TestLuaBase


class TestLuaLuaSec(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUASEC=y
        """

    def test_run(self):
        self.login()
        self.module_test("ssl")


class TestLuajitLuaSec(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUASEC=y
        """

    def test_run(self):
        self.login()
        self.module_test("ssl")
