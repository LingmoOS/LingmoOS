from tests.package.test_lua import TestLuaBase


class TestLuaLuaGD(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUA_GD=y
        LINGMO_PACKAGE_FONTCONFIG=y
        LINGMO_PACKAGE_JPEG=y
        LINGMO_PACKAGE_LIBPNG=y
        """

    def test_run(self):
        self.login()
        self.module_test("gd")


class TestLuajitLuaGD(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUA_GD=y
        LINGMO_PACKAGE_FONTCONFIG=y
        LINGMO_PACKAGE_JPEG=y
        LINGMO_PACKAGE_LIBPNG=y
        """

    def test_run(self):
        self.login()
        self.module_test("gd")
