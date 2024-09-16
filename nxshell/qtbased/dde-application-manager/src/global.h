// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QMap>
#include <QDBusConnection>
#include <optional>
#include <QDBusError>
#include <QMetaType>
#include <QMetaClassInfo>
#include <QLocale>
#include <QDir>
#include <QRegularExpression>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QDBusArgument>
#include <QDBusMessage>
#include <unistd.h>
#include <QUuid>
#include <QLoggingCategory>
#include <sys/stat.h>
#include <sys/syscall.h>
#include "constant.h"
#include "config.h"

Q_DECLARE_LOGGING_CATEGORY(DDEAMProf)

using ObjectInterfaceMap = QMap<QString, QVariantMap>;
using ObjectMap = QMap<QDBusObjectPath, ObjectInterfaceMap>;
using QStringMap = QMap<QString, QString>;
using PropMap = QMap<QString, QStringMap>;

Q_DECLARE_METATYPE(ObjectInterfaceMap)
Q_DECLARE_METATYPE(ObjectMap)
Q_DECLARE_METATYPE(QStringMap)
Q_DECLARE_METATYPE(PropMap)

struct SystemdUnitDBusMessage
{
    QString name;
    QString subState;
    QDBusObjectPath objectPath;
};

inline const QDBusArgument &operator>>(const QDBusArgument &argument, QStringMap &map)
{
    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        QString value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        map.insert(key, value);
    }
    argument.endMap();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, QList<SystemdUnitDBusMessage> &units)
{
    argument.beginArray();
    while (!argument.atEnd()) {
        argument.beginStructure();
        QString _str;
        uint32_t _uint;
        QDBusObjectPath _path;
        SystemdUnitDBusMessage unit;
        argument >> unit.name >> _str >> _str >> _str >> unit.subState >> _str >> unit.objectPath >> _uint >> _str >> _path;
        units.push_back(unit);
        argument.endStructure();
    }
    argument.endArray();

    return argument;
}

inline QString getApplicationLauncherBinary()
{
    static const QString bin = []() {
        auto value = qgetenv("DEEPIN_APPLICATION_MANAGER_APP_LAUNCH_HELPER_BIN");
        if (value.isEmpty()) {
            return QString::fromLocal8Bit(ApplicationLaunchHelperBinary);
        }
        qWarning() << "Using app launch helper defined in environment variable DEEPIN_APPLICATION_MANAGER_APP_LAUNCH_HELPER_BIN.";
        return QString::fromLocal8Bit(value);
    }();
    return bin;
}

enum class DBusType { Session = QDBusConnection::SessionBus, System = QDBusConnection::SystemBus, Custom };

template <typename T>
using remove_cvr_t = std::remove_reference_t<std::remove_cv_t<T>>;

template <typename T>
void applyIteratively(QList<QDir> dirs,
                      T &&func,
                      QDir::Filters filter = QDir::NoFilter,
                      QStringList nameFilter = {},
                      QDir::SortFlags sortFlag = QDir::SortFlag::NoSort)
{
    static_assert(std::is_invocable_v<T, const QFileInfo &>, "apply function should only accept one QFileInfo");
    static_assert(std::is_same_v<decltype(func(QFileInfo{})), bool>,
                  "apply function should return a boolean to indicate when should return");
    QList<QDir> dirList{std::move(dirs)};

    while (!dirList.isEmpty()) {
        const auto &dir = dirList.takeFirst();

        if (!dir.exists()) {
            qWarning() << "apply function to an non-existent directory:" << dir.absolutePath() << ", skip.";
            continue;
        }
        const auto &infoList = dir.entryInfoList(nameFilter, filter, sortFlag);

        for (const auto &info : infoList) {
            if (info.isFile() and func(info)) {
                return;
            }

            if (info.isDir()) {
                dirList.append(info.absoluteFilePath());
            }
        }
    }
}

class ApplicationManager1DBus
{
public:
    ApplicationManager1DBus(const ApplicationManager1DBus &) = delete;
    ApplicationManager1DBus(ApplicationManager1DBus &&) = delete;
    ApplicationManager1DBus &operator=(const ApplicationManager1DBus &) = delete;
    ApplicationManager1DBus &operator=(ApplicationManager1DBus &&) = delete;
    [[nodiscard]] const QString &globalDestBusAddress() const { return m_destBusAddress; }
    [[nodiscard]] const QString &globalServerBusAddress() const { return m_serverBusAddress; }
    void initGlobalServerBus(DBusType type, const QString &busAddress = "")
    {
        if (m_initFlag) {
            return;
        }

        m_serverBusAddress = busAddress;
        m_serverType = type;
        m_initFlag = true;
    }

    QDBusConnection &globalServerBus()
    {
        if (m_serverConnection.has_value()) {
            return m_serverConnection.value();
        }

        if (!m_initFlag) {
            qFatal("invoke init at first.");
        }

        switch (m_serverType) {
        case DBusType::Session:
            [[fallthrough]];
        case DBusType::System: {
            m_serverConnection.emplace(QDBusConnection::connectToBus(static_cast<QDBusConnection::BusType>(m_serverType),
                                                                     ApplicationManagerServerDBusName));
            if (!m_serverConnection->isConnected()) {
                qFatal("%s", m_serverConnection->lastError().message().toLocal8Bit().data());
            }
            return m_serverConnection.value();
        }
        case DBusType::Custom: {
            if (m_serverBusAddress.isEmpty()) {
                qFatal("connect to custom dbus must init this object by custom dbus address");
            }
            m_serverConnection.emplace(QDBusConnection::connectToBus(m_serverBusAddress, ApplicationManagerServerDBusName));
            if (!m_serverConnection->isConnected()) {
                qFatal("%s", m_serverConnection->lastError().message().toLocal8Bit().data());
            }
            return m_serverConnection.value();
        }
        }

        Q_UNREACHABLE();
    }
    static ApplicationManager1DBus &instance()
    {
        static ApplicationManager1DBus dbus;
        return dbus;
    }

    QDBusConnection &globalDestBus()
    {
        if (!m_destConnection) {
            qFatal("please set which bus should application manager to use to invoke other D-Bus service's method.");
        }
        return m_destConnection.value();
    }

    void setDestBus(const QString &destAddress = "")
    {
        if (m_destConnection) {
            m_destConnection->disconnectFromBus(ApplicationManagerDestDBusName);
        }

        m_destBusAddress = destAddress;

        if (m_destBusAddress.isEmpty()) {
            m_destConnection.emplace(
                QDBusConnection::connectToBus(QDBusConnection::BusType::SessionBus, ApplicationManagerDestDBusName));
            if (!m_destConnection->isConnected()) {
                qFatal("%s", m_destConnection->lastError().message().toLocal8Bit().data());
            }
            return;
        }

        if (m_destBusAddress.isEmpty()) {
            qFatal("connect to custom dbus must init this object by custom dbus address");
        }

        m_destConnection.emplace(QDBusConnection::connectToBus(m_destBusAddress, ApplicationManagerDestDBusName));
        if (!m_destConnection->isConnected()) {
            qFatal("%s", m_destConnection->lastError().message().toLocal8Bit().data());
        }
    }

private:
    ApplicationManager1DBus() = default;
    ~ApplicationManager1DBus() = default;
    bool m_initFlag{false};
    DBusType m_serverType{};
    QString m_serverBusAddress;
    QString m_destBusAddress;
    std::optional<QDBusConnection> m_destConnection{std::nullopt};
    std::optional<QDBusConnection> m_serverConnection{std::nullopt};
};

bool registerObjectToDBus(QObject *o, const QString &path, const QString &interface);
void unregisterObjectFromDBus(const QString &path);

inline QString getDBusInterface(const QMetaType &meta)
{
    auto name = QString{meta.name()};
    if (name == "ApplicationAdaptor") {
        return ApplicationInterface;
    }

    if (name == "InstanceAdaptor") {
        return InstanceInterface;
    }

    if (name == "APPObjectManagerAdaptor" or name == "AMObjectManagerAdaptor") {
        return ObjectManagerInterface;
    }

    if (name == "ApplicationManager1Service")
        // const auto *infoObject = meta.metaObject();
        // if (auto infoIndex = infoObject->indexOfClassInfo("D-Bus Interface"); infoIndex != -1) {
        //     return infoObject->classInfo(infoIndex).value();
        // }
        qWarning() << "couldn't found interface:" << name;
    return "";
}

inline ObjectInterfaceMap getChildInterfacesAndPropertiesFromObject(QObject *o)
{
    auto childs = o->children();
    ObjectInterfaceMap ret;

    std::for_each(childs.cbegin(), childs.cend(), [&ret](QObject *app) {
        if (app->inherits("QDBusAbstractAdaptor")) {
            auto interface = getDBusInterface(app->metaObject()->metaType());
            QVariantMap properties;
            const auto *mo = app->metaObject();
            for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
                auto prop = mo->property(i);
                properties.insert(prop.name(), prop.read(app));
            }
            ret.insert(interface, properties);
        }
    });

    return ret;
}

inline QStringList getChildInterfacesFromObject(QObject *o)
{
    auto childs = o->children();
    QStringList ret;

    std::for_each(childs.cbegin(), childs.cend(), [&ret](QObject *app) {
        if (app->inherits("QDBusAbstractAdaptor")) {
            ret.append(getDBusInterface(app->metaObject()->metaType()));
        }
    });

    return ret;
}

inline uid_t getCurrentUID()
{
    return getuid();  // current use linux getuid
}

inline QLocale getUserLocale()
{
    return QLocale::system();  // current use env
}

inline QString escapeToObjectPath(const QString &str)
{
    if (str.isEmpty()) {
        return "_";
    }

    auto ret = str;
    QRegularExpression re{R"([^a-zA-Z0-9])"};
    auto matcher = re.globalMatch(ret);
    while (matcher.hasNext()) {
        auto replaceList = matcher.next().capturedTexts();
        replaceList.removeDuplicates();
        for (const auto &c : replaceList) {
            auto hexStr = QString::number(static_cast<uint>(c.front().toLatin1()), 16);
            ret.replace(c, QString{R"(_%1)"}.arg(hexStr));
        }
    }
    return ret;
}

inline QString unescapeFromObjectPath(const QString &str)
{
    auto ret = str;
    for (qsizetype i = 0; i < str.size(); ++i) {
        if (str[i] == '_' and i + 2 < str.size()) {
            auto hexStr = str.sliced(i + 1, 2);
            ret.replace(QString{"_%1"}.arg(hexStr), QChar::fromLatin1(hexStr.toUInt(nullptr, 16)));
            i += 2;
        }
    }
    return ret;
}

inline QString escapeApplicationId(const QString &id)
{
    if (id.isEmpty()) {
        return id;
    }

    auto ret = id;
    QRegularExpression re{R"([^a-zA-Z0-9])"};
    auto matcher = re.globalMatch(ret);
    while (matcher.hasNext()) {
        auto replaceList = matcher.next().capturedTexts();
        replaceList.removeDuplicates();
        for (const auto &c : replaceList) {
            auto hexStr = QString::number(static_cast<uint>(c.front().toLatin1()), 16);
            ret.replace(c, QString{R"(\x%1)"}.arg(hexStr));
        }
    }
    return ret;
}

inline QString unescapeApplicationId(const QString &id)
{
    auto ret = id;
    for (qsizetype i = 0; i < id.size(); ++i) {
        if (id[i] == '\\' and i + 3 < id.size()) {
            auto hexStr = id.sliced(i + 2, 2);
            ret.replace(QString{R"(\x%1)"}.arg(hexStr), QChar::fromLatin1(hexStr.toUInt(nullptr, 16)));
            i += 3;
        }
    }
    return ret;
}

inline QString getRelativePathFromAppId(const QString &id)
{
    QString path;
    auto components = id.split('-', Qt::SkipEmptyParts);
    for (qsizetype i = 0; i < components.size() - 1; ++i) {
        path += QString{"/%1"}.arg(components[i]);
    }
    path += QString{R"(-%1.desktop)"}.arg(components.last());
    return path;
}

inline QString getXDGDataHome()
{
    auto XDGDataHome = QString::fromLocal8Bit(qgetenv("XDG_DATA_HOME"));
    if (XDGDataHome.isEmpty()) {
        XDGDataHome = QString::fromLocal8Bit(qgetenv("HOME")) + QDir::separator() + ".local" + QDir::separator() + "share";
    }
    return XDGDataHome;
}

inline QStringList getXDGDataDirs()
{
    auto XDGDataDirs = QString::fromLocal8Bit(qgetenv("XDG_DATA_DIRS")).split(':', Qt::SkipEmptyParts);

    if (XDGDataDirs.isEmpty()) {
        XDGDataDirs.append("/usr/local/share");
        XDGDataDirs.append("/usr/share");
    }

    auto XDGDataHome = getXDGDataHome();
    if (XDGDataDirs.contains(XDGDataHome)) {
        XDGDataDirs.removeAll(XDGDataHome);
    }

    XDGDataDirs.push_front(XDGDataHome);
    return XDGDataDirs;
}

inline QStringList getDesktopFileDirs()
{
    auto XDGDataDirs = getXDGDataDirs();
    std::for_each(XDGDataDirs.begin(), XDGDataDirs.end(), [](QString &str) {
        if (!str.endsWith(QDir::separator())) {
            str.append(QDir::separator());
        }
        str.append("applications");
    });

    return XDGDataDirs;
}

inline QString getXDGConfigHome()
{
    auto XDGConfigHome = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
    if (XDGConfigHome.isEmpty()) {
        XDGConfigHome = QString::fromLocal8Bit(qgetenv("HOME")) + QDir::separator() + ".config";
    }

    return XDGConfigHome;
}

inline QStringList getXDGConfigDirs()
{
    auto XDGConfigDirs = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_DIRS")).split(':', Qt::SkipEmptyParts);
    if (XDGConfigDirs.isEmpty()) {
        XDGConfigDirs.append("/etc/xdg");
    }

    auto XDGConfigHome = getXDGConfigHome();

    if (XDGConfigDirs.constFirst() != XDGConfigHome) {
        XDGConfigDirs.removeAll(XDGConfigHome);
        XDGConfigDirs.push_front(std::move(XDGConfigHome));  //  guarantee XDG_CONFIG_HOME is first element.
    }

    return XDGConfigDirs;
}

inline QStringList getAutoStartDirs()
{
    auto XDGConfigDirs = getXDGConfigDirs();
    std::for_each(XDGConfigDirs.begin(), XDGConfigDirs.end(), [](QString &str) {
        if (!str.endsWith(QDir::separator())) {
            str.append(QDir::separator());
        }
        str.append("autostart");
    });

    return XDGConfigDirs;
}

inline QString getCurrentDesktop()
{
    auto desktops = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP")).split(';', Qt::SkipEmptyParts);

    if (desktops.size() > 1) {
        qWarning() << "multi-DE is detected, use first value.";
    }

    if (desktops.isEmpty()) {
        return "DDE";
    }

    return desktops.first();
}

struct unitInfo
{
    QString applicationID;
    QString Launcher;
    QString instanceID;
};

inline unitInfo processUnitName(const QString &unitName)
{
    QString instanceId;
    QString launcher;
    QString applicationId;

    qDebug() << "process unit:" << unitName;

    decltype(auto) appPrefix = u8"app-";
    if (!unitName.startsWith(appPrefix)) {
        // If not started by application manager, just remove suffix and take name as app id.
        auto lastDotIndex = unitName.lastIndexOf('.');
        applicationId = unitName.sliced(0, lastDotIndex);
        return {unescapeApplicationId(applicationId), std::move(launcher), std::move(instanceId)};
    }

    auto unit = unitName.sliced(sizeof(appPrefix) - 1);

    if (unit.endsWith(".service")) {
        auto lastDotIndex = unit.lastIndexOf('.');
        auto app = unit.sliced(0, lastDotIndex);  // remove suffix

        if (app.contains('@')) {
            auto atIndex = app.indexOf('@');
            instanceId = app.sliced(atIndex + 1);
            app.remove(atIndex, instanceId.length() + 1);
        }
        auto rest = app.split('-', Qt::SkipEmptyParts);
        if (rest.size() == 2) {
            launcher = rest.takeFirst();
        }
        if (rest.size() == 1) {
            applicationId = rest.takeFirst();
        }
    } else if (unit.endsWith(".scope")) {
        auto lastDotIndex = unit.lastIndexOf('.');
        auto app = unit.sliced(0, lastDotIndex);

        auto components = app.split('-');
        instanceId = components.takeLast();
        applicationId = components.takeLast();
        if (!components.isEmpty()) {
            launcher = components.takeLast();
        }
    } else {
        qDebug() << "it's not service or scope:" << unit << "ignore";
        return {};
    }

    return {unescapeApplicationId(applicationId), std::move(launcher), std::move(instanceId)};
}

template <typename Key, typename Value>
ObjectMap dumpDBusObject(const QHash<Key, QSharedPointer<Value>> &map)
{
    static_assert(std::is_base_of_v<QObject, Value>, "dumpDBusObject only support which derived by QObject class");
    static_assert(std::is_same_v<Key, QString> || std::is_same_v<Key, QDBusObjectPath>,
                  "dumpDBusObject only support QString/QDBusObject as key type");
    ObjectMap objs;

    for (const auto &[key, value] : map.asKeyValueRange()) {
        auto interAndProps = getChildInterfacesAndPropertiesFromObject(value.data());
        if constexpr (std::is_same_v<Key, QString>) {
            objs.insert(QDBusObjectPath{getObjectPathFromAppId(key)}, interAndProps);
        } else if constexpr (std::is_same_v<Key, QDBusObjectPath>) {
            objs.insert(key, interAndProps);
        }
    }

    return objs;
}

struct FileTimeInfo
{
    qint64 mtime;
    qint64 ctime;
    qint64 atime;
};

inline FileTimeInfo getFileTimeInfo(const QFileInfo &file)
{
    auto mtime = file.lastModified().toMSecsSinceEpoch();
    auto atime = file.lastRead().toMSecsSinceEpoch();
    auto ctime = file.birthTime().toMSecsSinceEpoch();
    return {mtime, ctime, atime};
}

inline QByteArray getCurrentSessionId()
{
    constexpr auto graphicalTarget = u8"graphical-session.target";

    auto msg = QDBusMessage::createMethodCall("org.freedesktop.systemd1",
                                              "/org/freedesktop/systemd1/unit/" + escapeToObjectPath(graphicalTarget),
                                              "org.freedesktop.DBus.Properties",
                                              "Get");
    msg << QString{"org.freedesktop.systemd1.Unit"};
    msg << QString{"InvocationID"};
    auto bus = QDBusConnection::sessionBus();
    auto ret = bus.call(msg);
    if (ret.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "get graphical session Id failed:" << ret.errorMessage();
        return {};
    }

    auto id = ret.arguments().first();
    return id.value<QDBusVariant>().variant().toByteArray();
}

inline uint getPidFromPidFd(const QDBusUnixFileDescriptor &pidfd) noexcept
{
    QString fdFilePath = QString{"/proc/self/fdinfo/%1"}.arg(pidfd.fileDescriptor());
    QFile fdFile{fdFilePath};
    if (!fdFile.open(QFile::ExistingOnly | QFile::ReadOnly | QFile::Text)) {
        qWarning() << "open " << fdFilePath << "failed: " << fdFile.errorString();
        return 0;
    }

    auto content = fdFile.readAll();
    QTextStream stream{content};
    QString appPid;
    while (!stream.atEnd()) {
        auto line = stream.readLine();
        if (line.startsWith("Pid")) {
            appPid = line.split(":").last().trimmed();
            break;
        }
    }

    if (appPid.isEmpty()) {
        qWarning() << "can't find pid which corresponding with the instance of this application.";
        return 0;
    }
    bool ok{false};
    auto pid = appPid.toUInt(&ok);
    if (!ok) {
        qWarning() << "AppId is failed to convert to uint.";
        return 0;
    }

    return pid;
}

inline QString getAutostartAppIdFromAbsolutePath(const QString &path)
{
    constexpr decltype(auto) desktopSuffix{u8".desktop"};
    auto tmp = path.chopped(sizeof(desktopSuffix) - 1);
    auto components = tmp.split(QDir::separator(), Qt::SkipEmptyParts);
    auto location = std::find(components.cbegin(), components.cend(), "autostart");
    if (location == components.cend()) {
        return {};
    }

    auto appId = QStringList{location + 1, components.cend()}.join('-');
    return appId;
}

inline QString getObjectPathFromAppId(const QString &appId)
{
    if (!appId.isEmpty()) {
        return QString{DDEApplicationManager1ObjectPath} + "/" + escapeToObjectPath(appId);
    }
    return QString{DDEApplicationManager1ObjectPath} + "/" + QUuid::createUuid().toString(QUuid::Id128);
}

inline int pidfd_open(pid_t pid, uint flags)
{
    return syscall(SYS_pidfd_open, pid, flags);
}

#define safe_sendErrorReply                                                                                                      \
    if (calledFromDBus())                                                                                                        \
    sendErrorReply

#endif
