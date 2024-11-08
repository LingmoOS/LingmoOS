### 目录结构
```shell
.
├── event-listener
│ ├── pom.xml
│ ├── src
│ │ ├── main
│ │ │ ├── java
│ │ │ │ ├── cn
│ │ │ │ │ └── lingmo
│ │ │ │ │     └── dbus
│ │ │ │ │         ├── client
│ │ │ │ │         │ └── DBusClient.java  dbus连接器
│ │ │ │ │         ├── enums
│ │ │ │ │         │ └── EventTypeEnum.java  事件枚举
│ │ │ │ │         ├── event
│ │ │ │ │         │ ├── PresenceEvent.java  
│ │ │ │ │         │ ├── ScreenSaverEvent.java
│ │ │ │ │         │ ├── UserNameEvent.java
│ │ │ │ │         │ └── UserSessionEvent.java
│ │ │ │ │         └── listener
│ │ │ │ │             ├── DBusListener.java 监听器
│ │ │ │ │             ├── DBusListenerSupport.java 监听器扩展
│ │ │ │ │             ├── EventResult.java 监听器返回值
│ │ │ │ │             ├── PresnceStatusChangedListener.java 键盘监听器
│ │ │ │ │             ├── ScreenSaverLockListener.java 锁屏监听器
│ │ │ │ │             ├── ScreenSaverUnLockListener.java 解屏监听器
│ │ │ │ │             ├── UserSessionNewListener.java 登录监听器
│ │ │ │ │             └── UserSessionRemoveListener.java 注销监听器
│ │ │ │ ├── com
│ │ │ │ │ └── lingmo
│ │ │ │ │     └── SessionManager
│ │ │ │ │         └── User.java
│ │ │ │ └── org
│ │ │ │     ├── freedesktop
│ │ │ │     │ └── login1
│ │ │ │     │     └── Manager.java
│ │ │ │     ├── gnome
│ │ │ │     │ └── SessionManager
│ │ │ │     │     └── Presence.java
│ │ │ │     └── lingmo
│ │ │ │         └── ScreenSaver.java
│ │ │ └── resources
│     └── test
│         └── java
│             └── cn
│                 └── lingmo
│                     └── dbus
│                         └── EventTest.java
│ └── target
├── package
│ ├── build.sh
│ └── settings.xml
├── pom.xml
└── README.md

```


### 编译
```shell
cd package
./build.sh
```

### 使用
```java
        //执行dubus操作
        new PresenceEvent().StatusChanged();
        new ScreenSaverEvent().Lock();
        new UserSessionEvent().SessionNew();
        //监听事件
        DBusListenerSupport listenerSupport = new DBusListenerSupport();
        EventResult lock= listenerSupport.triggerEvent(EventTypeEnum.LOCK);
        EventResult unlock=  listenerSupport.triggerEvent(EventTypeEnum.UNLOCK);
        EventResult changed=  listenerSupport.triggerEvent(EventTypeEnum.KEY_STATUS_CHANGED);
```

### 注意事项
> 所有继承DBusInterface的接口实现，必须跟dbus的路径和方法名一致，大小写强校验
