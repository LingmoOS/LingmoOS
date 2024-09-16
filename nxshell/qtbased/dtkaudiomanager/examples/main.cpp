// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>

#include "daudiomanager.h"
#include "daudiocard.h"
#include "daudioport.h"
#include "daudiodevice.h"
#include "daudiostream.h"

#include <QDebug>
#include <QTime>
#include <QTimer>

DAUDIOMANAGER_USE_NAMESPACE

struct Command
{
    virtual ~Command();
    virtual bool exec() = 0;
    void outputProperties()
    {
        while(props.size() >= 2) {
            qInfo().noquote() << "\t" << props.takeFirst().toString() << ": " << props.takeFirst().toString();
        }
    }
    double changeValue(const double origin, const double flag = 0.5, const double delta = 0.1)
    {
        return origin + (origin > flag ? -delta : +delta);
    }

    QStringList m_argus;
    DAudioManager m_handler;
    QVariantList props;
};
Command::~Command() {}

class CommandManager
{
public:
    using CommandCreator = std::function<Command *()>;
    ~CommandManager()
    {
        m_commands.clear();
        qDeleteAll(m_notExitCommanders);
        m_notExitCommanders.clear();
    }
    void regist(const QString &cmd, const QString &desc, const CommandCreator &creator)
    {
        m_commands.insert(cmd, creator);
        m_descs.insert(cmd, desc);
    }
    bool exec(const QStringList &argument)
    {
        do {
            if (argument.size() <= 1) {
                break;
            }

            auto cmd = argument[1];

            m_argus = argument.mid(1);
            bool exit = true;
            if (execCommands(cmd, exit)){
                return exit;
            }
        } while(false);

        return outputHelp();
    }
    bool outputHelp()
    {
        for (auto iter = m_descs.begin(); iter != m_descs.end(); ++iter) {
            qInfo() << iter.key() << "    " << iter.value();
        }
        return true;
    }
    bool execCommands(const QString &cmdPrefix, bool &exit)
    {
        bool existCommand = false;
        for (auto iter = m_commands.begin(); iter != m_commands.end(); ++iter) {
            const auto cmd = iter.key();
            if (cmd == cmdPrefix || cmd.startsWith(cmdPrefix) || cmdPrefix == "all") {
                existCommand = true;

                auto creator = iter.value();
                auto command = creator();
                command->m_argus = m_argus;
                qInfo() << "************" << m_descs.value(iter.key());
                auto readyExit = command->exec();
                exit &= readyExit;
                if (readyExit) {
                    delete command;
                    command = nullptr;
                } else {
                    m_notExitCommanders << command;
                }
            }
        }
        return existCommand;
    }
    QMap<QString, CommandCreator> m_commands;
    QMap<QString, QString> m_descs;
    QStringList m_argus;
    QVector<Command *> m_notExitCommanders;
};

struct ListCards : public Command
{
    virtual bool exec()
    {
        auto cards = m_handler.cards();
        qInfo() << cards.size() << "card info:";
        for (auto card : cards) {
            qInfo() << card->name();
            props << "type" << card->type();
            if (card->type() == DAudioCard::Bluetooth) {
                if (auto bluetoothCard = qobject_cast<DAudioBluetoothCard *>(card.data())) {
                    props << "mode" << bluetoothCard->mode()
                          << "modeOptions" << bluetoothCard->modeOptions();
                }
            }
            outputProperties();

            qInfo() << "\t" << card->ports().size() << "port info:";
            for (auto port : card->ports()) {
                qInfo() << "\t" << port->name() << port->description();
            }
        }
        return true;
    }
};

struct ListInputDevices : public Command
{
    virtual bool exec()
    {
        qInfo() << "input devices info:";
        for (auto device : m_handler.inputDevices()) {
            qInfo() << device->name();
            props << "mute" << device->mute()
                  << "fade" << device->fade()
                  << "volume" << device->volume()
                  << "balance" << device->balance()
                  << "supportFade" << device->supportFade()
                  << "supportBalance" << device->supportBalance()
                  << "meterVolume" << device->meterVolume();

            outputProperties();
            qInfo() << device->streams().size() << " output streams info:";
            for (auto stream : device->streams()) {
                qInfo() << stream->name();
                props << "mute" << stream->mute()
                      << "fade" << stream->fade()
                      << "volume" << stream->volume()
                      << "balance" << stream->balance();

                outputProperties();
            }
        }
        return true;
    }
};

struct ListOutputDevices : public Command
{
    virtual bool exec()
    {
        qInfo() << "output devices info:";
        for (auto device : m_handler.outputDevices()) {
            qInfo() << device->name();
            props << "mute" << device->mute()
                  << "fade" << device->fade()
                  << "volume" << device->volume()
                  << "balance" << device->balance()
                  << "supportFade" << device->supportFade()
                  << "supportBalance" << device->supportBalance()
                  << "meterVolume" << device->meterVolume();

            outputProperties();
            qInfo() << device->streams().size() << " input streams info:";
            for (auto stream : device->streams()) {
                qInfo() << stream->name();
                props << "mute" << stream->mute()
                      << "fade" << stream->fade()
                      << "volume" << stream->volume()
                      << "balance" << stream->balance();

                outputProperties();
            }
        }
        return true;
    }
};

struct SetOutputDevices : public Command
{
    virtual bool exec()
    {
        qInfo() << "set output devices info:";
        for (auto device : m_handler.outputDevices()) {
            qInfo() << device->name() << "pre";
            props << "mute" << device->mute()
                  << "fade" << device->fade()
                  << "volume" << device->volume()
                  << "balance" << device->balance()
                  << "supportFade" << device->supportFade()
                  << "supportBalance" << device->supportBalance()
                  << "meterVolume" << device->meterVolume();

            outputProperties();

            device->setMute(!device->mute());
            device->setFade(changeValue(device->fade()));
            device->setVolume(changeValue(device->volume()));

            qInfo() << device->name() << "now";
            props << "mute" << device->mute()
                  << "fade" << device->fade()
                  << "volume" << device->volume()
                  << "balance" << device->balance()
                  << "supportFade" << device->supportFade()
                  << "supportBalance" << device->supportBalance()
                  << "meterVolume" << device->meterVolume();

            outputProperties();
        }
        return true;
    }
};

struct MonitorCards : public Command
{
    virtual bool exec()
    {
        m_cards = m_handler.cards();
        print(m_cards);
        QObject::connect(&m_handler, &DAudioManager::cardsChanged, [this]() {
            m_cards = m_handler.cards();
            print(m_cards);
        });
        QObject::connect(&m_handler, &DAudioManager::deviceAdded, [](const QString &name, const bool isInputDevice) {
            qInfo() << "deviceAdded" << name << isInputDevice;
        });
        QObject::connect(&m_handler, &DAudioManager::deviceRemoved, [](const QString &name, const bool isInputDevice) {
            qInfo() << "deviceRemoved" << name << isInputDevice;
        });
        return false;
    }
    void print(const QList<DAudioCardPtr> &cards)
    {
        qInfo() << "card info:";
        for (auto item : cards) {
            qInfo() << item->name();
            for (auto port : item->ports()) {
                qInfo() << port->name() << port->description() << port->direction() << port->available();
            }
        }
    }
    QList<DAudioCardPtr> m_cards;
};

struct MonitorOutputDevice : public Command
{
    virtual bool exec()
    {
        auto devices = m_handler.outputDevices();
        if (devices.isEmpty())
            return false;
        if (m_argus.size() == 1) {
            auto name = m_argus[0];
            m_device = m_handler.outputDevice(name);
            if (!m_device) {
                auto index = m_argus[0].toInt();
                if (index >= 0 && index < devices.size())
                    m_device = devices[index];
            }
        }

        if (!m_device)  {
            m_device = devices.first();
        }

        qInfo()  << m_device->name() << "output device properties info:";
        qInfo() << "mute:" << m_device->mute();
        qInfo() << "fade:" << m_device->fade();
        qInfo() << "volume:" << m_device->volume();
        qInfo() << "activePort:" << m_device->activePort();
        const auto ports = m_device->ports();
        qInfo() << "ports: " << ports.size();
        for (int i = 0; i < ports.size(); i++) {
            qInfo() << i << "port:" << ports[i]->name() << ports[i]->description();
        }

        QObject::connect(m_device.data(), &DAudioOutputDevice::muteChanged, [](bool mute) {
            qInfo() << "mute:" << mute;
        });
        QObject::connect(m_device.data(), &DAudioOutputDevice::fadeChanged, [](double fade) {
            qInfo() << "fade:" << fade;
        });
        QObject::connect(m_device.data(), &DAudioOutputDevice::volumeChanged, [](double volume) {
            qInfo() << "volume:" << volume;
        });
        QObject::connect(m_device.data(), &DAudioOutputDevice::activePortChanged, [](QString port) {
            qInfo() << "activePort:" << port;
        });
        return false;
    }
    DAudioOutputDevicePtr m_device;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    CommandManager manager;
    manager.regist("list-cards", "List cards", [](){ return new ListCards;});
    manager.regist("list-input-devices", "List loaded input devices", [](){ return new ListInputDevices;});
    manager.regist("list-output-devices", "List loaded output devices", [](){ return new ListOutputDevices;});
    manager.regist("set-output-devices", "Change output device property, example: mute, fade, volume", [](){ return new SetOutputDevices;});
    manager.regist("monitor-cards", "Monitor cards changed", [](){ return new MonitorCards;});
    manager.regist("monitor-output-device-properties", "Monitor output device property changed, example: mute, fade", [](){ return new MonitorOutputDevice;});
    QTimer::singleShot(0, qApp, [&manager, &app]() {
        if (manager.exec(app.arguments())) {
            app.exit(0);
        }
    });

    return app.exec();
}
