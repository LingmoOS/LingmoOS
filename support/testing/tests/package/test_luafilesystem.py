from tests.package.test_lua import TestLuaBase


class TestLuaLuaFileSystem(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUAFILESYSTEM=y
        """

    def test_run(self):
        self.login()
        self.module_test("lfs")


class TestLuajitLuaFileSystem(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUAFILESYSTEM=y
        """

    def test_run(self):
        self.login()
        self.module_test("lfs")
