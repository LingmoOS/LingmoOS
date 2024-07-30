#!/usr/bin/env python3
"""
SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
SPDX-License-Identifier: MIT
"""

# For FreeBSD CI which only has python 3.9
from __future__ import annotations

import logging
import os
import subprocess
import sys
import time
import unittest
from typing import Final

from gi.repository import Gio, GLib

CMAKE_RUNTIME_OUTPUT_DIRECTORY: Final = os.environ.get("CMAKE_RUNTIME_OUTPUT_DIRECTORY", "/usr/libexec")
PORTAL_SERVICE_NAME: Final = "org.freedesktop.impl.portal.desktop.kde"
GROUP: Final = "org.freedesktop.appearance"
KEYS: Final = ("accent-color", "color-scheme")
EXPECTED_SIGNATURES: Final = ("(ddd)", "u")


def name_has_owner(session_bus: Gio.DBusConnection, name: str) -> bool:
    """
    Whether the given name is available on session bus
    """
    message: Gio.DBusMessage = Gio.DBusMessage.new_method_call("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "NameHasOwner")
    message.set_body(GLib.Variant("(s)", [name]))
    reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
    return reply and reply.get_signature() == 'b' and reply.get_body().get_child_value(0).get_boolean()


class OrgFreeDesktopAppearanceTests(unittest.TestCase):
    """
    Tests for org.freedesktop.appearance
    """

    portal_process: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
        portal_launched = name_has_owner(session_bus, PORTAL_SERVICE_NAME)
        if not portal_launched:
            debug_environ = os.environ.copy()
            debug_environ["QT_LOGGING_RULES"] = "*.debug=true"
            cls.portal_process = subprocess.Popen([os.path.join(CMAKE_RUNTIME_OUTPUT_DIRECTORY, "xdg-desktop-portal-kde")], env=debug_environ, stdout=sys.stderr, stderr=sys.stderr)
            for _ in range(10):
                if name_has_owner(session_bus, PORTAL_SERVICE_NAME):
                    portal_launched = True
                    break
                logging.info("waiting for portal to appear on the DBus session")
                time.sleep(1)
        assert portal_launched

    @classmethod
    def tearDownClass(cls) -> None:
        if cls.portal_process is not None:
            cls.portal_process.terminate()

    def test_bug476592_signature(self) -> None:
        """
        Make sure the signature is correct
        """
        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)

        for key, expected_signature in zip(KEYS, EXPECTED_SIGNATURES):
            message: Gio.DBusMessage = Gio.DBusMessage.new_method_call(PORTAL_SERVICE_NAME, "/org/freedesktop/portal/desktop", "org.freedesktop.impl.portal.Settings", "Read")
            message.set_body(GLib.Variant("(ss)", [GROUP, key]))
            reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
            self.assertIsNotNone(reply)
            self.assertEqual(reply.get_signature(), 'v', f"Wrong signature: {reply.get_signature()}")
            self.assertEqual(reply.get_body().get_child_value(0).get_variant().get_type_string(), expected_signature)


if __name__ == '__main__':
    logging.getLogger().setLevel(logging.DEBUG)
    unittest.main()
