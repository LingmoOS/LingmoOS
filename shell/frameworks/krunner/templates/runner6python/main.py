#!/usr/bin/python3

import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

DBusGMainLoop(set_as_default=True)

objpath = "/runner" # Default value for X-Plasma-DBusRunner-Path metadata property
iface = "org.kde.krunner1"


class Runner(dbus.service.Object):
    def __init__(self):
        dbus.service.Object.__init__(self, dbus.service.BusName("org.kde.%{APPNAMELC}", dbus.SessionBus()), objpath)

    @dbus.service.method(iface, in_signature='s', out_signature='a(sssida{sv})')
    def Match(self, query: str):
        """This method is used to get the matches and it returns a list of tupels"""
        if query == "hello":
            # data, text, icon, type (KRunner::QueryType), relevance (0-1), properties (subtext, category, multiline(bool) and urls)
            return [("Hello", "Hello from %{APPNAME}!", "document-edit", 100, 1.0, {'subtext': 'Demo Subtext'})]
        return []

    @dbus.service.method(iface, out_signature='a(sss)')
    def Actions(self):
        # id, text, icon
        return [("id", "Tooltip", "planetkde")]

    @dbus.service.method(iface, in_signature='ss')
    def Run(self, data: str, action_id: str):
        print(data, action_id)


runner = Runner()
loop = GLib.MainLoop()
loop.run()
