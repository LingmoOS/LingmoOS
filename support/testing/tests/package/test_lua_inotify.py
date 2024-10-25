from tests.package.test_lua import TestLuaBase


class TestLuaInotify(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUA_INOTIFY=y
        """

    def test_run(self):
        self.login()
        self.module_test("inotify")


class TestLuajitInotify(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUA_INOTIFY=y
        """

    def test_run(self):
        self.login()
        self.module_test("inotify")
