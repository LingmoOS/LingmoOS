#!/usr/bin/python3
#

import dbus
import dbus.service
import dbus.mainloop.glib
from gi.repository import GLib


#dbus调用封装
class MyDbusCall:
    def __init__(self, servername, objectpath, type = 0) -> None:
        self.__servername = servername
        self.__path = objectpath
        self.__dbus = None
        if 0 == type:
            self.__dbus = dbus.SessionBus()
        else:
            self.__dbus = dbus.SystemBus()
        self.__proxy = self.__dbus.get_object(servername, objectpath)

    def getdbus(self):
        return self.__dbus
    
    #调用服务提供接口
    #params ---- type(list)
    def callmethod(self, dbus_interface, function, params):
        res = None
        try:
            res = self.__proxy.get_dbus_method(function, dbus_interface=dbus_interface)(*params)
        except Exception as e:
            res = e
        return res
    
    def getAttr(self, attr_interface, dbus_interface, function):
        res = None
        try:
           interface = dbus.Interface(self.__proxy, attr_interface)
           res = interface.Get(dbus_interface, function)
        except Exception as e:
            res = e
        return res
    
    def addSignal(self, path, dbus_interface, signal_name, callback):
        self.__dbus.add_signal_receiver(callback, dbus_interface = dbus_interface, 
                                        signal_name = signal_name, bus_name=self.__servername, 
                                        path = path)


ServiceName = "org.freedesktop.login1"
ObjectName = "/org/freedesktop/login1"
InterfaceName = ServiceName + ".Manager"
InterfaceName1 = "org.freedesktop.DBus.Properties"
InterfaceName2 = ServiceName + ".Session"
methed_ListSession = "ListSessions"

s = None

class Sess():
    def __init__(self, sid, uid, user, seat, objectpath) -> None:
        self.sid = sid
        self.uid = uid
        self.user = user
        self.seat = seat
        self.objectpath = objectpath
                
        self.__dbus = MyDbusCall(ServiceName, self.objectpath, 1)
        self.Remote = self.__dbus.getAttr(InterfaceName1, InterfaceName2, "Remote")
        self.Active = self.__dbus.getAttr(InterfaceName1, InterfaceName2, "Active")
        self.Service = self.__dbus.getAttr(InterfaceName1, InterfaceName2, "Service")
        if user is None:
            self.user = self.__dbus.getAttr(InterfaceName1, InterfaceName2, "Name")
        
        self.__dbus.addSignal(self.objectpath, InterfaceName1, "PropertiesChanged", self.PropertiesChanged)
        print("sess[{}]: user<{}> Remote<{}> Active<{}> Service<{}>".format(
            self.sid, self.user, self.Remote, self.Active, self.Service))

    def PropertiesChanged(self, v1, v2, v3):
        if "Active" in v2:
            self.Active = v2['Active']
        print("sess[{}]: user<{}> Remote<{}> Active<{}> Service<{}>".format(
            self.sid, self.user, self.Remote, self.Active, self.Service))
        s.UpdateActiveUser()
    
    def CalcValue(self):
        if self.Active != True:
            return False
        if self.Remote:
            return False
        if self.sid.isdigit() != True:
            return False
        return True
    
class SessMng(dbus.service.Object):
    def __init__(self, bus) -> None:
        self.__sesses = dict()
        self.activeUser = None
        self.path = '/com/lingmo/test'
        bus_name = dbus.service.BusName("org.lingmo.test", bus)
        dbus.service.Object.__init__(self, bus_name, self.path)
        

    def createSess(self, sid, uid, user, seat, objectpath):
        print("createSess : ", sid)
        if sid in self.__sesses:
            return False
        
        self.__sesses[sid] = Sess(sid, uid, user, seat, objectpath)
        print("sess num : ", len(self.__sesses))
        return True
    
    def delSess(self, sid):
        print("del: ", sid)
        if sid in self.__sesses:
            self.__sesses.pop(sid)
        print("sess num : ", len(self.__sesses))
    
    def getSess(self, sid):
        return self.__sesses(sid, None)

    def UpdateActiveUser(self):
        last = None
        for sess in self.__sesses.values():
            if sess.CalcValue() is True:
                last = sess
                break
        if last is not None and self.activeUser != last.user:
            self.activeUser = last.user
            self.ActiveUserChange(self.activeUser)
            print("emit ActiveUserChange signal")
            
        print("activeUser : ", self.activeUser)

    @dbus.service.signal('com.lingmo.test', signature='s')
    def ActiveUserChange(self, activeUser):
        # nothing else to do so...
        pass
    
    @dbus.service.method("com.lingmo.test",
        in_signature='',
        out_signature='s',
        sender_keyword='sender')
    def GetActiveUser(self, sender):
        print("caller: ", sender)
        return self.activeUser

def SessNew(sid, objectpath):
    s.createSess(sid, None, None, None, objectpath)
    s.UpdateActiveUser()
    
def SessRemove(sid, objectpath):
    s.delSess(sid)
    s.UpdateActiveUser()


if  __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    
    mydbus = MyDbusCall(ServiceName, ObjectName, 1)
    sesses = mydbus.callmethod(InterfaceName, methed_ListSession, [])
    s = SessMng(mydbus.getdbus())
    for sess in sesses:
        #print(sess)
        if len(sess) != 5:
            continue
        s.createSess(*sess)

    print(mydbus.getdbus().get_unique_name())
    mydbus.addSignal(ObjectName, InterfaceName, "SessionNew", SessNew)
    mydbus.addSignal(ObjectName, InterfaceName, "SessionRemoved", SessRemove)

    s.UpdateActiveUser()
    
    mainloop = GLib.MainLoop()
    mainloop.run()