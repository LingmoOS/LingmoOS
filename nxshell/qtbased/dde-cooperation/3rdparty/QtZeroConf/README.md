
QZeroConf is a Qt wrapper class for ZeroConf libraries across various platforms.

* Windows (requires iTunes or [Apple Print Services](https://support.apple.com/kb/DL999))
* Mac
* Linux
* Android
* iOS

QZeroConf wraps avahi-client on Linux, Network Discovery Service (java) on Android, and dnssd on Mac, iOS and Windows.

### Building

QZeroConf can be built directly into your project if your project is [LGPL3](http://www.gnu.org/licenses/lgpl-3.0.en.html) compatible.  If your project is closed source, you can build QZeroConf as a dynamic library and link against it.

#### Building into your project

1. Clone or download QZeroConf.  If you download, unzip.
2. Copy the qtzeroconf directory to be under your project's directory.
3. Include the qtzeroconf.pri file in your projects .pro file

    include(qtzeroconf/qtzeroconf.pri)

4. Add QZEROCONF_STATIC define in your projects .pro file

    DEFINES= QZEROCONF_STATIC

#### Compiling as a dynamic library

1. Clone or download QZeroConf.  If you download, unzip.
2. Enter the qtzeroconf directory, run qmake and then make.

#### Building with CMake
Use `BUILD_SHARED_LIBS` to control whether QZeroConf should be built as static (`-DBUILD_SHARED_LIBS=OFF`) or as shared (`-DBUILD_SHARED_LIBS=ON`) library.
The default is `OFF`.

You can also build the included example project by setting `BUILD_EXAMPLE` to `ON`.
The default for this is `OFF`

#### Android

Prior to Android api 30, QtZeroConf used AvaliCore.  AvaliCore no longer works >= api 30 as bind() to netlink sockets was disabled in Android.  QtZeroConf now uses the Android java Network Discovery Services.  NDS is slightly buggy, but more or less gets the job done.  A common issue with NDS is that if the app is in sleep mode and a service is removed on another device, the app does not get notified the service was removed when it wakes back up.  ANDROID_PACKAGE_SOURCE_DIR must be added to your app's .pro file.

```
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
```
QZeroConfNsdManager.java must then be copied or linked to $$PWD/android/src/QZeroConfNsdManager.java  Notice the extra src/   ...gradle expects this.

### API

#### Service Publishing

(See the example included with the source)

1) Include header

```c++
#include "qzeroconf.h"
```
2) Create an instance of QZeroConf

```c++
QZeroConf zeroConf;
```
It is recommend, but not required, that you connect a slot to QZeroConf's error() signal and servicePublished() signal.

3) If you want to add one or more txt records to the service, call
```c++
zeroConf.addServiceTxtRecord("name", "value");
```
or
```c++
zeroConf.addServiceTxtRecord("nameOnly");
```
before calling startServicePublish()

4) Call startServicePublish() with the name, type, domain and port of your service.

```c++
zeroConf.startServicePublish("Test", "_test._tcp", "local", 12345);
```
QZeroConf will emit servicePublished() if successful, or the error() signal if registration fails.

Service publishing can be stopped by calling stopServicePublish().
Only one service can be published per instance of QZeroConf.

#### Service Discovery

(See the example included with the source)

1) Include header

```c++
#include "qzeroconf.h"
```
2) Create an instance of QZeroConf

```c++
QZeroConf zeroConf;
```
It is recommend, but not required, that you connect a slot to QZeroConf's error() signal.

3) Connect a slot to QZeroConf's serviceAdded() signal.  When serviceAdded() is emitted, it passes the QZeroConfService recently discovered.  QZeroConfServices are [shared objects](http://doc.qt.io/qt-5/implicit-sharing.html).  They are safe to use between threads.

4) Optionally connect a slot to QZeroConf's serviceRemoved() signal to received status when the service is unpublished.   ServiceRemoved() passes the QZeroConfService being removed.

5) Call startBrowser() with the type of the service to browse for and optionally the protocol to use.

```c++
startBrowser("_test._tcp");
```
If you are browsing for services published using both ipv4 and ipv6 ( QAbstractSocket::AnyIPProtocol) you should also connect a slot to QzeroConf's serviceUpdated() signal.  When the IP address of the first protocol is resolved,  serviceAdded() is emitted, when the IP address of the second protocol is resolved,  serviceUpdated() is emitted.

Only one browser can be in use per instance of QzeroConf.

**Txt records** are placed into a QMap called txt within the discovered service. For example, the value of txt record "Qt=The Best!" can be retrieved with the code... 

```c++
qDebug() << zcs->txt["Qt"];
```
**QML**

QZeroConf can be used in QML applications


### Build Dependencies

Qt5 or Qt6

On Linux, libavahi-client-dev and libavahi-common-dev

### Distribution Requirements


Distributing Software / Apps that use QtZeroConf must follow the requirements of the LGPLv3.  Some of my interpretations of the LGPLv3


* LGPLv3 text or a link to the LGPLv3 text must be distributed with the binary.  My preferred way to do this is in the App's "About" section.
* An offer of source code with a link to QtZeroConf must be distributed with the binary.  My preferred way to do this is in the App's "About" section
* For Android and iOS apps only, instructions on how to re-link or re-package the App or a link to instructions must be distributed with the binary.  My preferred way to do this is in the App's "About" section.


All of the above must be shown to the user at least once, separate from the EULA, with a method for the user to acknowledge they have seen it (an ok button).  Ideally all of the above is also listed in the description of the App on the App Store.

### Apple App Store deployment

Publishing closed source Apps that use a LGPLv3 library in the Apple App Store must provide a method for the end user to 1. update the library in the app and 2. run the new version of the app with the updated library.  Qt on iOS further complicates this by using static linking.  Closed source Apps on iOS using QtZeroConf must provide the apps object files along with clear step by step instructions on how to re-link the app with a new / different version of QtZeroConf (obligation 1).  iOS end uses can run the re-linked App on their device by creating a free iOS developer account and use the time limited signing on that account for their device.  (obligation 2)  I consider this an poor way to meet obligation 2, but as long as Apple has this mechanism, obligation 2 is meet.  I will not pursue copyright infringement as long as the individual / organization is meeting obligation 1 and 2 and the Distribution Requirements above.

### iOS device sleep

When iOS puts the device to sleep, it breaks the DNS-SD browser and service publisher.  The only way around this is to call stopServicePublish() and stopBrowser() when the application state changes to Qt::ApplicationSuspended (sleep) and then call startPublish() and startBrowser() when the application state changes to Qt::ApplicationActive (wake).  See appStateChanged() in example.

### iOS 14 and up

iOS 14 and up requires apps to have permissions to access the local network.  See [this video](https://developer.apple.com/videos/play/wwdc2020/10110/) Two keys must be added to the info.plist....

```xml
1. <key>NSLocalNetworkUsageDescription</key>
   <string>This app will need access to the local network for Discovery services.</string>
2. <key>NSBonjourServices</key>
       <array>
           <string>_myservice1._tcp</string>
           <string>_myservice2._tcp</string>
       </array>
```

