// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ThreadExecXrandr.h"
#include "commonfunction.h"
#include "DDLog.h"

#include <QProcess>
#include <QLoggingCategory>
#include <QDBusInterface>
#include <QDBusReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <DApplication>

#include <DeviceManager.h>
#include<QDateTime>
#ifdef OS_BUILD_V23
const QString DISPLAY_SERVICE_NAME = "org.deepin.dde.Display1";
const QString DISPLAY_SERVICE_PATH = "/org/deepin/dde/Display1";
const QString DISPLAY_INTERFACE = "org.deepin.dde.Display1";

const QString DISPLAY_DAEMON_SERVICE_NAME = "org.deepin.dde.Display1";
const QString DISPLAY_DAEMON_SERVICE_PATH = "/org/deepin/dde/Display1";
const QString DISPLAY_DAEMON_INTERFACE = "org.deepin.dde.Display1";
const QString DISPLAY_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";
const QString DISPLAY_MONITOR_INTERFACE = "org.deepin.dde.Display1.Monitor";
#else
const QString DISPLAY_SERVICE_NAME = "com.deepin.system.Display";
const QString DISPLAY_SERVICE_PATH = "/com/deepin/system/Display";
const QString DISPLAY_INTERFACE = "com.deepin.system.Display";

const QString DISPLAY_DAEMON_SERVICE_NAME = "com.deepin.daemon.Display";
const QString DISPLAY_DAEMON_SERVICE_PATH = "/com/deepin/daemon/Display";
const QString DISPLAY_DAEMON_INTERFACE = "com.deepin.daemon.Display";
const QString DISPLAY_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";
const QString DISPLAY_MONITOR_INTERFACE = "com.deepin.daemon.Display.Monitor";
#endif
using namespace DDLog;
ThreadExecXrandr::ThreadExecXrandr(bool gpu, bool isDXcbPlatform)
    : m_Gpu(gpu), m_isDXcbPlatform(isDXcbPlatform)
{

}

void ThreadExecXrandr::run()
{
    if (m_Gpu) {
        getGpuInfoFromXrandr();
    } else {           
       if(Common::boardVendorType() == "PGUV") {
           QList<QMap<QString, QString>> lstMap;
           getResolutionRateFromDBus(lstMap);
       }else 
            getMonitorInfoFromXrandrVerbose();
    }
}

void ThreadExecXrandr::runCmd(QString &info, const QString &cmd)
{
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();
}

void ThreadExecXrandr::loadXrandrInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd)
{
    QString deviceInfo;
    runCmd(deviceInfo, cmd);
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        if (line.startsWith("Screen")) {
            lstMap.append(QMap<QString, QString>());
            QRegExp re(".*([0-9]{1,5}\\sx\\s[0-9]{1,5}).*([0-9]{1,5}\\sx\\s[0-9]{1,5}).*([0-9]{1,5}\\sx\\s[0-9]{1,5}).*");
            if (re.exactMatch(line)) {
                lstMap[lstMap.count() - 1].insert("minResolution", re.cap(1));
                lstMap[lstMap.count() - 1].insert("curResolution", re.cap(2));
                lstMap[lstMap.count() - 1].insert("maxResolution", re.cap(3));
            }
            continue;
        }

        if (line.startsWith("HDMI")) {
            lstMap[lstMap.count() - 1].insert("HDMI", "Enable");
        } else if (line.startsWith("VGA")) {
            lstMap[lstMap.count() - 1].insert("VGA", "Enable");
        } else if (line.startsWith("DP") || line.startsWith("DisplayPort")) {
            lstMap[lstMap.count() - 1].insert("DP", "Enable");
        } else if (line.startsWith("eDP")) {
            lstMap[lstMap.count() - 1].insert("eDP", "Enable");
        } else if (line.startsWith("DVI")) {
            lstMap[lstMap.count() - 1].insert("DVI", "Enable");
        } else if (line.startsWith("DigitalOutput")) {
            lstMap[lstMap.count() - 1].insert("DigitalOutput", "Enable");
        }
    }
}

void ThreadExecXrandr::loadXrandrVerboseInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd)
{
    QString deviceInfo;
    runCmd(deviceInfo, cmd);

    QStringList lines = deviceInfo.split("\n");
    QStringList::iterator it = lines.begin();
    for (; it != lines.end(); ++it) {
        if ((*it).startsWith("Screen"))
            continue;

        //获取 HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm
        QRegExp reg("^[A-Za-z].*");
        if (reg.exactMatch(*it) && !(*it).contains("disconnected")) {
            // 新的显示屏
            QMap<QString, QString> newMap;
            newMap.insert("mainInfo", (*it).trimmed());
            lstMap.append(newMap);
            continue;
        }

        if (lstMap.size() < 1)
            continue;
        QMap<QString, QString> &last = lstMap.last();
        // 获取edid信息
        QString edid;
        QRegExp reEdid("^[\\t]{2}([0-9a-f]{32}).*");
        if (reEdid.exactMatch(*it)) {
            while (true) {
                edid.append(reEdid.cap(1));
                edid.append("\n");
                if (++it == lines.end())
                    return;
                if (!reEdid.exactMatch(*it)) {
                    last.insert("edid", edid);
                    break;
                }
            }
            continue;
        }

        // 获取当前频率
        if ((*it).contains("*current")) {
            if ((it += 2) >= lines.end())
                return;
            QRegExp regRate(".*([0-9]{1,5}\\.[0-9]{1,5}Hz).*");
            if (regRate.exactMatch(*it))
                last.insert("rate", regRate.cap(1));
        }
    }
}

void ThreadExecXrandr::getRefreshRateFromDBus(QList<QMap<QString, QString> > &lstMap)
{
    QDBusInterface displayInterface(DISPLAY_SERVICE_NAME, DISPLAY_SERVICE_PATH, DISPLAY_INTERFACE, QDBusConnection::systemBus());
    if (!displayInterface.isValid())
        return;
    QDBusReply<QString> reply = displayInterface.call("GetConfig");
    if (!reply.isValid())
        return;
    QString info = reply.value();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(info.toLatin1());
    if (!jsonDoc.isObject())
        return;
    QJsonObject object = jsonDoc.object();
    QStringList keys = object.keys();

    //读取Config
    if (!keys.contains("Config"))
        return;
    QJsonValue configValue = object.value("Config");
    if (!configValue.isObject())
        return;
    QJsonObject configObject = configValue.toObject();
    QStringList configKeys = configObject.keys();

    //读取Screens
    if (!configKeys.contains("Screens"))
        return;
    QJsonValue screensValue = configObject.value("Screens");
    if (!screensValue.isObject())
        return;
    QJsonObject screensObject = screensValue.toObject();
    QStringList screensKeys = screensObject.keys();

    for (auto screenKey : screensKeys) {
        QJsonValue screenValue = screensObject.value(screenKey);
        if (!screenValue.isObject())
            continue;
        QJsonObject screenObject = screenValue.toObject();
        QStringList screenKeys = screenObject.keys();

        if (!screenKeys.contains("Extend"))
            continue;
        QJsonValue extendValue = screenObject.value("Extend");
        if (!extendValue.isObject())
            continue;
        QJsonObject extendObject = extendValue.toObject();
        QStringList extendKeys = extendObject.keys();

        if (!extendKeys.contains("Monitors"))
            continue;
        QJsonValue monitorsValue = extendObject.value("Monitors");
        if (!monitorsValue.isArray())
            continue;
        QJsonArray monitorsObject = monitorsValue.toArray();

        for (auto monitor : monitorsObject) {
            if (!monitor.isObject())
                continue;

            QJsonObject monitorObject = monitor.toObject();
            QStringList monitorKeys = monitorObject.keys();
            if (!monitorKeys.contains("Name") || !monitorKeys.contains("RefreshRate"))
                continue;

            QJsonValue ss = monitorObject.value("RefreshRate");
            QMap<QString, QString> infoMap;
            infoMap.insert("Name", monitorObject.value("Name").toString());
            QString refreshRate = QString::number(monitorObject.value("RefreshRate").toDouble(), 'f', 2);
            infoMap.insert("RefreshRate", refreshRate);
            lstMap.append(infoMap);
        }
    }
}

void ThreadExecXrandr::getMonitorInfoFromXrandrVerbose()
{
    QList<QMap<QString, QString>> lstMap;
    loadXrandrVerboseInfo(lstMap, "xrandr --verbose");

    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setMonitorInfoFromXrandr((*it)["mainInfo"], (*it)["edid"], (*it)["rate"]);
    }
}

struct MonitorResolution {
    uint32_t index;
    uint16_t width;
    uint16_t height;
    double refreshRate;
};

const QDBusArgument &operator>>(const QDBusArgument &argument, MonitorResolution &resolution)
{
    argument.beginStructure();
    argument >> resolution.index;
    argument >> resolution.width;
    argument >> resolution.height;
    argument >> resolution.refreshRate;
    argument.endStructure();
    return argument;
}
void ThreadExecXrandr::getResolutionFromDBus(QMap<QString, QString> &lstMap)
{
    QDBusInterface displayInterface(DISPLAY_DAEMON_SERVICE_NAME, DISPLAY_DAEMON_SERVICE_PATH, DISPLAY_DAEMON_INTERFACE, QDBusConnection::sessionBus());
    if (!displayInterface.isValid())
        return;

    QVariant monitors = displayInterface.property("Monitors");

    if (!monitors.isValid())
        return;
    m_monitorLst.clear();
    QList<QDBusObjectPath> monitorList = monitors.value<QList<QDBusObjectPath> >();
    int maxResolutionWidth = -1, maxResolutionHeight = -1, minResolutionWidth = -1, minResolutionHeight = -1;
    for (auto monitor : monitorList) {
        if (monitor.path().isEmpty())
            continue;

        m_monitorLst << monitor.path();
        QDBusInterface monitorEnabledInterface(DISPLAY_DAEMON_SERVICE_NAME, monitor.path(), DISPLAY_MONITOR_INTERFACE, QDBusConnection::sessionBus());
        if (!monitorEnabledInterface.isValid())
            continue;

        QVariant enbaled = monitorEnabledInterface.property("Enabled");
        if (!enbaled.isValid() || !enbaled.toBool())
            continue;

        QDBusInterface monitorInterface(DISPLAY_DAEMON_SERVICE_NAME, monitor.path(), DISPLAY_PROPERTIES_INTERFACE, QDBusConnection::sessionBus());
        if (!monitorInterface.isValid())
            continue;

        QDBusMessage replay = monitorInterface.call("Get", DISPLAY_MONITOR_INTERFACE, "Modes");
        QVariant v =  replay.arguments().first();
        qCDebug(appLog) << v.value<QDBusVariant>().variant();
        QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();
        arg.beginArray();
        int curMaxResolutionWidth = -1, curMaxResolutionHeight = -1;
        while (!arg.atEnd()) {
            MonitorResolution resolution;
            arg >> resolution;
            if (curMaxResolutionWidth == -1) {
                curMaxResolutionWidth = resolution.width;
                curMaxResolutionHeight = resolution.height;
            } else {
                int curResolution = static_cast<int>(resolution.width) * static_cast<int>(resolution.height);
                if (curResolution > (curMaxResolutionWidth * curMaxResolutionHeight)) {
                    curMaxResolutionWidth = resolution.width;
                    curMaxResolutionHeight = resolution.height;
                }
            }
            if (minResolutionWidth == -1) {
                minResolutionWidth = resolution.width;
                minResolutionHeight = resolution.height;
            } else {
                int curResolution = static_cast<int>(resolution.width) * static_cast<int>(resolution.height);
                if (curResolution < (minResolutionWidth * minResolutionHeight)) {
                    minResolutionWidth = resolution.width;
                    minResolutionHeight = resolution.height;
                }
            }
        }
        if (maxResolutionWidth == -1) {
            maxResolutionWidth = curMaxResolutionWidth;
            maxResolutionHeight = curMaxResolutionHeight;
        } else {
            maxResolutionWidth += curMaxResolutionWidth;
            maxResolutionHeight += curMaxResolutionHeight;
        }
    }

    if (maxResolutionWidth != -1) {
        lstMap.insert("maxResolution", QString("%1 x %2").arg(maxResolutionWidth).arg(maxResolutionHeight));
        lstMap.insert("minResolution", QString("%1 x %2").arg(minResolutionWidth).arg(minResolutionHeight));
    }
}

void ThreadExecXrandr::getGpuInfoFromXrandr()
{
    QList<QMap<QString, QString>> lstMap;
    loadXrandrInfo(lstMap, "xrandr");

    // 通过dbus获取最大最小分辨率
    QMap<QString, QString> dbusMap;
    getResolutionFromDBus(dbusMap);
    for (auto &lstInfo : lstMap) {
        if (dbusMap.contains("minResolution")) {
            lstInfo["minResolution"] = dbusMap["minResolution"];
        }
        if (dbusMap.contains("maxResolution")) {
            lstInfo["maxResolution"] = dbusMap["maxResolution"];
        }
    }

    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setGpuInfoFromXrandr(*it);
    }
}

void ThreadExecXrandr::getResolutionRateFromDBus(QList<QMap<QString, QString> > &lstMap)
{
    QDBusInterface displayInterface(DISPLAY_DAEMON_SERVICE_NAME, DISPLAY_DAEMON_SERVICE_PATH, DISPLAY_DAEMON_INTERFACE, QDBusConnection::sessionBus());
    if (!displayInterface.isValid())
        return;

    QVariant monitors = displayInterface.property("Monitors");

    if (!monitors.isValid())
        return;

    QList<QDBusObjectPath> monitorList = monitors.value<QList<QDBusObjectPath> >();
    for (auto monitor : monitorList) {
        if (monitor.path().isEmpty())
            continue;

        QDBusInterface monitorEnabledInterface(DISPLAY_DAEMON_SERVICE_NAME, monitor.path(), DISPLAY_MONITOR_INTERFACE, QDBusConnection::sessionBus());
        if (!monitorEnabledInterface.isValid())
            continue;

        QVariant enbaled = monitorEnabledInterface.property("Enabled");
        if (!enbaled.isValid() || !enbaled.toBool())
            continue;
        QVariant tconnected = monitorEnabledInterface.property("Connected");
        QVariant tmanufacture = monitorEnabledInterface.property("Manufacturer");
        QVariant tname = monitorEnabledInterface.property("Name");

        // QVariant rep_currentRes = monitorEnabledInterface.property("CurrentMode");

        QDBusInterface monitorInterface(DISPLAY_DAEMON_SERVICE_NAME, monitor.path(), DISPLAY_PROPERTIES_INTERFACE, QDBusConnection::sessionBus());
        if (!monitorInterface.isValid())
            continue;

        QDBusMessage replay = monitorInterface.call("Get", DISPLAY_MONITOR_INTERFACE, "CurrentMode");   // "com.deepin.daemon.Display.Monitor","CurrentMode"
        QVariant v =  replay.arguments().first();
        qCDebug(appLog) << v.value<QDBusVariant>().variant();
        QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();

        int curResolutionWidth = -1, curResolutionHeight = -1;
        double resRate = 0;
        {
           MonitorResolution resolution;
           arg >> resolution;

            curResolutionWidth = resolution.width;
            curResolutionHeight = resolution.height;
            resRate = resolution.refreshRate;
            QMap<QString,QString>infoMap;
            QString tmpS = QString("%1 x %2 @").arg(curResolutionWidth).arg(curResolutionHeight)  + QString::number(resRate, 'f', 2);
            infoMap.insert("CurResolution", tmpS + "Hz");
            infoMap.insert("Name", tname.toString());
            infoMap.insert("Display Input", tname.toString());
            infoMap.insert("Manufacture", tmanufacture.toString());
            lstMap.append(infoMap);
        }  //end of while
    }  //end of for

    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setMonitorInfoFromDbus(*it);
    }
}
