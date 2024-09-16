# CuteIPC
[![Build Status](https://travis-ci.org/dept2/CuteIPC.svg?branch=master)](https://travis-ci.org/dept2/CuteIPC)

This library adds a facility to use Qt signals and slots across local processes and over TCP.
Based on [QLocalSocket](http://doc.qt.io/qt-5/qlocalsocket.html) and [QTcpSocket](http://doc.qt.io/qt-5/qtcpsocket.html), it is a cross platform IPC solution with native syntax for Qt programs.

## Installation
CuteIPC works with both Qt4 and Qt5 and can be built using cmake or [qbs](doc.qt.io/qbs/).

To build using cmake:
```sh
cmake CMakeLists.txt
make
make install
```

To build using qbs:
```sh
qbs build profile:<profile name> 
qbs install --install-root /usr/local profile:<profile name>
```

Finally, add the `include` directory of CuteIPC to your project.


## Usage

##### Connection

First, create a server and associate it with a desired `QObject`.
The signals and slots of this object will be used in IPC.
``` cpp
CuteIPCService* service = new CuteIPCService;

// setup local server
service->listen("serverName", myObject);
// or TCP server using host and port
service->listenTcp(QHostAddress::Any, 31337, myObject);
```

On the other side, create the interface and connect to the server: 
``` cpp
CuteIPCInterface* interface = new CuteIPCInterface;

// connect locally
interface->connectToServer("myServerName");
// or over TCP
interface->connectToServer(QHostAddress("192.0.2.1"), 31337);
```

##### Direct calls
You can to directly invoke any [invokable](http://doc.qt.io/qt-5/qobject.html#Q_INVOKABLE) method or public slot of the corresponding object.
The syntax is similar to [QMetaObject::invokeMethod](http://doc.qt.io/qt-5/qmetaobject.html#invokeMethod).

* Use `call` method to invoke synchronously and wait for result in the event loop:
``` cpp
int result;
interface->call("remoteMethod", Q_RETURN_ARG(int, result), Q_ARG(QString, methodParameter));
```

* Use `callNoReply` to send invoke request asynchronously and return immediately:
``` cpp
interface->callNoReply("remoteMethod", Q_ARG(QString, methodParameter));
```

##### Signals and slots
* You can natively connect the remote signal to a local slot:
``` cpp
interface->remoteConnect(SIGNAL(remoteSignal(QString)), receiver, SLOT(receiverSlot(QString)));
```

* or local signal to the remote slot:
``` cpp
interface->remoteSlotConnect(senderObject, SIGNAL(localSignal(QString)), SLOT(remoteSlot(QString)));
```

Please note, that all signals are sent asynchronously.

## API reference
A full API reference can be generated using `doxygen` in the source root.

## License
[LGPL 2.1](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)

This library is contributed under LGPL 2.1 license. Feel free to contact us if you're interested in using this library on other terms.
