// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QDebug>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QThread>
#include <DWayland/Client/connection_thread.h>
#include <DWayland/Client/registry.h>
#include <DWayland/Client/dpms.h>
#include <DWayland/Client/output.h>
#include <QString>
#include <QTimer>
#include <QTime>

using namespace KWayland::Client;

const QString VERSION = "1.0.1";
static Dpms::Mode mode = Dpms::Mode::Off;

void showDpms(const Dpms::Mode& mode )
{
    if (mode == Dpms::Mode::On) {
        qDebug() << "DPMS mode: on";
    } else if (mode == Dpms::Mode::Off) {
        qDebug() << "DPMS mode: Off";
    } else if (mode == Dpms::Mode::Standby) {
        qDebug() << "DPMS mode: Standby";
    } else if (mode == Dpms::Mode::Suspend) {
        qDebug() << "DPMS mode: Suspend";
    } else {
        qDebug() << "DPMS mode: unknow";
    }
}

const QStringList ModeString = {"On", "Standby", "Suspend", "Off"};

inline QString mode2string(Dpms::Mode mode) {
    return ModeString[int(mode)];
}

inline Dpms::Mode string2Mode(QString str) {
    return Dpms::Mode(ModeString.indexOf(str));
}    

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("wayland"));
    QGuiApplication a(argc, argv);
    // app name
    QCoreApplication::setApplicationName("dpms");
    // app version
    QCoreApplication::setApplicationVersion(VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Set or get dpms information."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption getOption(QStringList() << "g" << "get", QCoreApplication::translate("main", "Get dpms."));
    parser.addOption(getOption);
    QCommandLineOption setOption(QStringList() << "s" << "set", QCoreApplication::translate("main", "Set dpms to On/Standby/Suspend/Off"));
    setOption.setValueName("param");
    parser.addOption(setOption);
    parser.process(a);

    bool isGet = false;
    if (parser.isSet(getOption)) {
        isGet = true;
    } else if (parser.isSet(setOption)) {
        isGet = false;
        QString strSetOption = parser.value(setOption);
        if (strSetOption.compare(QString("On"), Qt::CaseInsensitive) == 0) {
            mode = Dpms::Mode::On;
        } else if (strSetOption.compare(QString("Off"), Qt::CaseInsensitive) == 0) {
            mode = Dpms::Mode::Off;
        } else if (strSetOption.compare(QString("Standby"), Qt::CaseInsensitive) == 0) {
            mode = Dpms::Mode::Standby;
        } else if (strSetOption.compare(QString("Suspend"), Qt::CaseInsensitive) == 0) {
            mode = Dpms::Mode::Suspend;
        } else {
            parser.showHelp();
            qApp->quit();
        }
    } else {
        parser.showHelp();
        qApp->quit();
    }

    ConnectionThread *conn = ConnectionThread::fromApplication();
    Registry registry;
    registry.create(conn);

    QObject::connect(&registry, &Registry::interfacesAnnounced, qApp, [&registry, isGet] {
        const bool hasDpms = registry.hasInterface(Registry::Interface::Dpms);

        DpmsManager *dpmsManager = nullptr;
        if (hasDpms) {
            const auto dpmsData = registry.interface(Registry::Interface::Dpms);
            dpmsManager = registry.createDpmsManager(dpmsData.name, dpmsData.version);
        }

        // get all Outputs
        const auto outputs = registry.interfaces(Registry::Interface::Output);
        static int applyCount = 0;
        static int changeCount = 0;
        for (auto o : outputs) {
            Output *output = registry.createOutput(o.name, o.version, &registry);
            if (auto dpms = dpmsManager->getDpms(output)) {
                QObject::connect(dpms, &Dpms::supportedChanged, [dpms, outputs, isGet]() {
                    if (!dpms->isSupported()) {
                        qDebug() << "output not support dpms!";
                        qApp->quit();
                    }

                    if (isGet) {
                        showDpms(dpms->mode());
                        ++applyCount;
                        if (applyCount == outputs.size()) {
                            qApp->quit();
                        }
                    } else {
                        qDebug() << QTime::currentTime().toString(QString::fromLatin1("hh:mm:ss.zzz"))
                                 << "set dpms mode:" << mode2string(mode);

                        if (dpms->mode() == mode) {
                            qDebug() << QTime::currentTime().toString(QString::fromLatin1("hh:mm:ss.zzz"))
                                     << "new dpms equal the old, no need to set.";
                            ++applyCount;
                            if (applyCount == outputs.size()) {
                                qApp->quit();
                            }
                            return;
                        }

                        ++changeCount;
                        dpms->requestMode(mode);
                    }
                });

                QObject::connect(dpms, &Dpms::modeChanged, [dpms, outputs, isGet] {
                    Q_UNUSED(isGet)
                    if (!isGet) {
                        if (changeCount > 0 && dpms->mode() == mode) {
                            qDebug() << "set dpms mode:" << mode2string(mode) << "success.";
                            --changeCount;
                            if (changeCount == 0) {
                                qApp->quit();
                            }
                        }
                    }
                });
            }
        }
    }, Qt::QueuedConnection
    );

    QObject::connect(conn, &ConnectionThread::failed, [conn] {
        qDebug() << "connect failed to wayland at socket:" << conn->socketName();
    });

    registry.setup();

    return a.exec();
}
