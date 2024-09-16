@~chinese
\mainpage

## 项目介绍

dtkaudiomanager提供对系统音频管理的功能，内部适配不同环境下的音频服务，包括含有dde-daemon的系统环境、pulseaudio提供的音频服务及pipewire提供的音频服务。此模块的的主要功能：
- 音频管理，管理音频设备、提供音频硬件设备信息，及提供与音频有关的设备状态获取。
- 音频流管理，音频流的音量、声道平衡值控制，及提供音频流对应的应用信息、使用的硬件信息

## 如何使用项目

如果要使用此项目，可以阅读相关文档再参照以下示例进行调用

## 使用示例

可参考以下示例：

- 在qmake管理的项目*.pro工程文件里添加`dtkaudiomanager`模块引用

```pro
QT += dtkaudiomanager
```

- 在cmake管理的项目CMakeLists.txt文件里添加

```CMakeLists.txt
find_package(dtkaudiomanager)
target_include_directories(${PROJECT_NAME} PUBLIC ${dtkaudiomanager_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${dtkaudiomanager_LIBRARIES})
```

- 输出所有的声卡及声卡含有的端口列表

```cpp
#include <DAudioManager>
#include <DAudioPort>
#include <QDebug>
DAUDIOMANAGER_USE_NAMESPACE

void listCards()
{  
    // 初始化音频管理类
    DAudioManager handler;
    // 获取所有声卡对象
    auto cards = handler.cards();
    qInfo() << cards.size() << "card info:";
    for (auto card : cards) {
        qInfo() << card->name();
        qInfo() << "type" << card->type();
        if (card->type() == DAudioCard::Bluetooth) {
            if (auto bluetoothCard = qobject_cast<DAudioBluetoothCard *>(card.data())) {
                qInfo() << "mode" << bluetoothCard->mode()
                        << "modeOptions" << bluetoothCard->modeOptions();
            }
        }

        qInfo() << "\t" << card->ports().size() << "port info:";
        // 获取此声卡下所有音频端口对象
        for (auto port : card->ports()) {
            qInfo() << "\t" << port->name() << port->description();
        }
    }
}
```

- 输出所有的音频输出设备及其对应的音频流

```cpp
#include <DAudioManager>
#include <DAudioStream>
#include <QDebug>
DAUDIOMANAGER_USE_NAMESPACE

void listOutputDevicesAndStreams()
{  
    DAudioManager handler;
    qInfo() << "output devices info:";
    for (auto device : handler.outputDevices()) {
        qInfo() << device->name();
        qInfo() << "mute" << device->mute()
                << "fade" << device->fade()
                << "volume" << device->volume()
                << "balance" << device->balance()
                << "supportFade" << device->supportFade()
                << "supportBalance" << device->supportBalance()
                << "meterVolume" << device->meterVolume();

        qInfo() << device->streams().size() << " input streams info:";
        for (auto stream : device->streams()) {
            qInfo() << stream->name();
            qInfo() << "mute" << stream->mute()
                    << "fade" << stream->fade()
                    << "volume" << stream->volume()
                    << "balance" << stream->balance();
        }
    }
}
```
