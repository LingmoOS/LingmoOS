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
#  Authors: Yunhe Liu <liuyunhe@kylinos.cn>
# 
# 

# -*- coding: UTF-8 -*- 
from base64 import decode
from email.utils import getaddresses
from time import sleep
from traceback import print_tb
from urllib.request import urlopen
import requests
import json
import re
import dbus
import dbus.service
import dbus.mainloop.glib
from gi.repository import GObject as gobject


class AddressBus(dbus.service.Object):

    def __init__(self, conn=None, object_path=None):
        dbus.service.Object.__init__(self, conn, object_path)
        self._last_input = None

    def getaddr(self):
        text = str(urlopen("http://txt.go.sohu.com/ip/soip").read())
        ip = re.findall(r'\d+.\d+.\d+.\d+',text)
        return ip[0]

    @dbus.service.method('com.lingmosdk.location', out_signature='s')
    def getAddress(self):
        url = "http://api.map.baidu.com/location/ip"
        myParams = {"ak":"4DPAeozisfSCUgSLuC14Ux4iXjeEYNGa","ip":self.getaddr(),"coor":"bd09ll"}
        res = requests.get(url=url,params=myParams)
        if res.status_code != 200:
            return "Error Code" + res.status_code
        decoded_u = res.text.encode('utf-8').decode('unicode_escape')
        data = json.loads(decoded_u)
        if data["status"] == 0 :
            return decoded_u
        else:
            return data["message"]

if __name__ == '__main__' :
    dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)
    system_bus = dbus.SystemBus()
    name = dbus.service.BusName("com.lingmosdk.location",system_bus)
    object_path = "/com/lingmosdk/location"
    obj = AddressBus(system_bus, object_path)
    mainloop = gobject.MainLoop()
    mainloop.run()
