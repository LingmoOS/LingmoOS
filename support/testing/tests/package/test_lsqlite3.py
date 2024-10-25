from tests.package.test_lua import TestLuaBase


class TestLuaLsqlite3(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LSQLITE3=y
        """

    def test_run(self):
        self.login()
        self.module_test("lsqlite3")


class TestLuajitLsqlite3(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LSQLITE3=y
        """

    def test_run(self):
        self.login()
        self.module_test("lsqlite3")
