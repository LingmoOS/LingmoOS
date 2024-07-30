from tests.package.test_lua import TestLuaBase


class TestLuajitTurbolua(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_TURBOLUA=y
        """

    def test_run(self):
        self.login()
        self.module_test("turbo")
