// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shell.h"
#include "treelandoutputwatcher.h"

#include <DConfig>
#include <QDBusConnection>
#include <QDBusError>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQmlAbstractUrlInterceptor>
#include <memory>
#include <qmlengine.h>

DS_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dsLoaderLog)

class DtkInterceptor : public QObject, public QQmlAbstractUrlInterceptor
{
public:
    DtkInterceptor(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    QUrl intercept(const QUrl &path, DataType type)
    {
        if (type != DataType::QmlFile)
            return path;
        if (path.path().endsWith("overridable/InWindowBlur.qml")) {
            qDebug() << "Override dtk's InWindowBlur";
            return QStringLiteral("qrc:/shell/override/dtk/InWindowBlur.qml");
        }

        return path;
    }
};

Shell::Shell(QObject *parent)
    : QObject(parent)
{
    // Since Wayland has no concept of primaryScreen, it is necessary to rely on wayland private protocols
    // to update the client's primaryScreen information

    auto platformName = QGuiApplication::platformName();
    if (QStringLiteral("wayland") == platformName) {
        new TreelandOutputWatcher(this);
    }
}

void Shell::installDtkInterceptor()
{
    auto engine = DQmlEngine().engine();
    engine->addUrlInterceptor(new DtkInterceptor(this));
}

void Shell::disableQmlCache()
{
    if (qEnvironmentVariableIsEmpty("QML_DISABLE_DISK_CACHE"))
        qputenv("QML_DISABLE_DISK_CACHE", "1");
}

void Shell::setFlickableWheelDeceleration(const int &value)
{
    if (qEnvironmentVariableIsEmpty("QT_QUICK_FLICKABLE_WHEEL_DECELERATION"))
        qputenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", QString::number(value).toLocal8Bit());
}

void Shell::dconfigsMigrate()
{
    std::unique_ptr<Dtk::Core::DConfig> dconfig(Dtk::Core::DConfig::create("org.lingmo.ocean.shell", "org.lingmo.ocean.shell"));
    auto migratedDconfigs = dconfig->value(QStringLiteral("migratedDConfigs"), QVariantHash()).toHash();

    // format appid/subpath/name
    QHash<QString, QString> dconfigMigrationLists = {
        // dock
        {"org.lingmo.ocean.shell//org.lingmo.ds.dock", "org.lingmo.ds.dock//org.lingmo.ds.dock"},
        {"org.lingmo.ocean.shell//org.lingmo.ds.dock.taskmanager", "org.lingmo.ds.dock//org.lingmo.ds.dock.taskmanager"},
        {"org.lingmo.ocean.shell//org.lingmo.ds.dock.tray", "org.lingmo.ds.dock//org.lingmo.ds.dock.tray"},

        // tray-loader
        {"org.lingmo.ocean.tray-loader//org.lingmo.ocean.network", "org.lingmo.ocean.tray.network//org.lingmo.ocean.network"},
        {"org.lingmo.ocean.tray-loader//org.lingmo.ocean.dock", "org.lingmo.ocean.dock//org.lingmo.ocean.dock"},
        {"org.lingmo.ocean.tray-loader//org.lingmo.ocean.dock.plugin.quick-panel", "org.lingmo.ocean.dock//org.lingmo.ocean.dock.plugin.quick-panel"},
        {"org.lingmo.ocean.tray-loader//org.lingmo.ocean.dock.plugin.common", "org.lingmo.ocean.dock//org.lingmo.ocean.dock.plugin.common"},
        {"org.lingmo.ocean.tray-loader//org.lingmo.ocean.dock.plugin.power", "org.lingmo.ocean.dock//org.lingmo.ocean.dock.plugin.power"},
        {"org.lingmo.ocean.tray-loader//org.lingmo.ocean.dock.plugin.sound","org.lingmo.ocean.dock//org.lingmo.ocean.dock.plugin.sound"},

        // launchpad
        {"org.lingmo.ocean.shell//org.lingmo.ds.launchpad","ocean-launchpad//org.lingmo.ocean.launchpad.appsmodel"},
    };

    for (auto it = dconfigMigrationLists.constBegin(); it != dconfigMigrationLists.constEnd(); ++it) {
        auto newConf = it.key();
        auto oldConf = it.value();
        if (migratedDconfigs.contains(newConf) && (migratedDconfigs.value(newConf).toString() == oldConf)) {
            continue;
        }
        if (dconfigMigrate(newConf, oldConf)) {
            migratedDconfigs.insert(newConf, oldConf);
        }
    }

    dconfig->setValue(QStringLiteral("migratedDConfigs"), migratedDconfigs);
}

bool Shell::registerDBusService(const QString &serviceName)
{
    auto bus = QDBusConnection::sessionBus();
    if (!bus.registerService(serviceName)) {
        qCWarning(dsLoaderLog).noquote() << QStringLiteral("Failed to register the dbus service: \"%1\".").arg(serviceName) << bus.lastError().message();
        return false;
    }
    return true;
}

bool Shell::dconfigMigrate(const QString &newConf, const QString &oldConf)
{
    auto newLastIndex = newConf.lastIndexOf('/');
    auto newFirstIndex = newConf.indexOf('/');

    auto oldLastIndex = oldConf.lastIndexOf('/');
    auto oldFirstIndex = oldConf.indexOf('/');

    std::unique_ptr<Dtk::Core::DConfig> newDconfig(Dtk::Core::DConfig::create(newConf.left(newFirstIndex), newConf.mid(newLastIndex + 1), newConf.mid(newFirstIndex + 1, newLastIndex - newFirstIndex - 1)));
    std::unique_ptr<Dtk::Core::DConfig> oldDconfig(Dtk::Core::DConfig::create(oldConf.left(oldFirstIndex), oldConf.mid(oldLastIndex + 1), oldConf.mid(oldFirstIndex + 1, oldLastIndex - newFirstIndex - 1)));

    if (!newDconfig->isValid() || !oldDconfig->isValid()) {
        return false;
    }

    auto keys = newDconfig->keyList();

    for (auto key : keys) {
         // 无效/空/新配置已发生修改 的 key 无需迁移
        if (oldDconfig->isDefaultValue(key) || !newDconfig->isDefaultValue(key)) {
            qCWarning(dsLoaderLog()) << QStringLiteral("Value is default on oldConf or not default for newConf. Do not migrate dconfig from %1 to %2 for key %3").arg(oldConf).arg(newConf).arg(key);
            continue;
        }

        auto value = oldDconfig->value(key);
        if (!value.isValid()) {
            qCWarning(dsLoaderLog()) << QStringLiteral("Value is invaild. Do not migrate dconfig from %1 to %2 for key %3").arg(oldConf).arg(newConf).arg(key);
            continue;
        }

        qCInfo(dsLoaderLog()) << QStringLiteral("migrate dconfig from %1 to %2 for key %3").arg(oldConf).arg(newConf).arg(key);
        newDconfig->setValue(key, value);
    }

    return true;
}

DS_END_NAMESPACE
