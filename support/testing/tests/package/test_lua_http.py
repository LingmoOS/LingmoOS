from tests.package.test_lua import TestLuaBase


class TestLuaHttp(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUA=y
        LINGMO_PACKAGE_LUA_HTTP=y
        """

    def test_run(self):
        self.login()
        self.module_test("http.version", script="local ver=require[[http.version]]; print(ver.name, ver.version)")
        self.module_test("http.client")
        self.module_test("http.cookie")
        self.module_test("http.proxies")
        self.module_test("http.server")
        self.module_test("http.socks")
        self.module_test("http.websocket")


class TestLuajitHttp(TestLuaBase):
    config = TestLuaBase.config + \
        """
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUA_HTTP=y
        """

    def test_run(self):
        self.login()
        self.module_test("http.version", script="local ver=require[[http.version]]; print(ver.name, ver.version)")
        self.module_test("http.client")
        self.module_test("http.cookie")
        self.module_test("http.proxies")
        self.module_test("http.server")
        self.module_test("http.socks")
        self.module_test("http.websocket")
