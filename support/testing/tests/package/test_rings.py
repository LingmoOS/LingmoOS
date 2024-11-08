from tests.package.test_lua import TestLuaBase


class TestLuaRings(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_RINGS=y
        """

    def test_run(self):
        self.login()
        self.module_test("rings")


class TestLuajitRings(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_RINGS=y
        """

    def test_run(self):
        self.login()
        self.module_test("rings")
