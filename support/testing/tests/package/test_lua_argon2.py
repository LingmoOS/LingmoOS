from tests.package.test_lua import TestLuaBase


class TestLuaLuaArgon2(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUA_ARGON2=y
        """

    def test_run(self):
        self.login()
        self.module_test("argon2")


class TestLuajitLuaArgon2(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUA_ARGON2=y
        """

    def test_run(self):
        self.login()
        self.module_test("argon2")
