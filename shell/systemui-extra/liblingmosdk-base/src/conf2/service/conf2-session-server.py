#!/usr/bin/python3
# 
#  liblingmosdk-system's Library
#  
#  Copyright (C) 2023, KylinSoft Co., Ltd.
# 
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 3 of the License, or (at your option) any later version.
# 
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this library.  If not, see <https://www.gnu.org/licenses/>.
# 
#  Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
# 
# 

import os
import logging
import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib
import conf2Utils

logger = None
destination_file = '~/.config/lingmo-config/user.db'

class KYSettingsDBusSessionService(dbus.service.Object):
    def __init__(self, bus_name, object_path):
        logger.info('Session DBus Init')
        dbus.service.Object.__init__(self, bus_name, object_path)

        conn = dbus.SystemBus()
        conn.add_signal_receiver(self._reload_handler, 
                            signal_name='updated',
                            dbus_interface='com.lingmo.lingmosdk.conf2',
                            bus_name='com.lingmo.lingmosdk.conf2',
                            path="/com/lingmo/lingmosdk/conf2")
        
        conn.add_signal_receiver(self._key_changed_handler, 
                    signal_name='key_changed',
                    dbus_interface='com.lingmo.lingmosdk.conf2',
                    bus_name='com.lingmo.lingmosdk.conf2',
                    path="/com/lingmo/lingmosdk/conf2")
        
        self._converter = conf2Utils.Converter(logger)

        self.Init()
        logger.info('Session DBus end')

    def _reload_handler(self):
        logger.info('On update')
        self.Init()
        self.updated()

    def _key_changed_handler(self, id, version, key):
        logger.info('On key changed')
        self._converter.update_user_dict(destination_file)
        self.key_changed(id, version, key)

    # key值改变信号
    @dbus.service.signal("com.lingmo.lingmosdk.conf2", signature='sss')
    def key_changed(self, id, version, key):
        logger.info(f'emit {key} changed')

    # 重载信号
    @dbus.service.signal("com.lingmo.lingmosdk.conf2", signature='')
    def updated(self):
        logger.info('emit updated')

    @dbus.service.method("com.lingmo.lingmosdk.conf2")
    def Init(self):
        self._converter.update_user_db(destination_file)

if __name__ == "__main__":
    destination_file = os.path.expanduser(destination_file)
    if not os.path.exists(os.path.dirname(destination_file)):
        os.makedirs(os.path.dirname(destination_file), mode=0o777, exist_ok=True)

    logging.basicConfig(filename=destination_file.replace('user.db', 'sessionbus.log'), 
                        level=logging.DEBUG, 
                        format='%(asctime)s.%(msecs)03d - %(name)s - %(levelname)s - %(message)s', 
                        datefmt='%Y-%m-%d %H:%M:%S')
    logger = logging.getLogger()
    
    # 初始化DBus主循环
    DBusGMainLoop(set_as_default=True)

    # 连接到DBus系统总线
    bus_session = dbus.SessionBus()

    # 注册一个DBus服务
    object_path = '/com/lingmo/lingmosdk/conf2'
    bus_sesssion_name = dbus.service.BusName("com.lingmo.lingmosdk.conf2", bus = bus_session)
    my_dbus_session_service = KYSettingsDBusSessionService(bus_sesssion_name, object_path)

    # 开始DBus主循环
    mainloop = GLib.MainLoop()
    mainloop.run()