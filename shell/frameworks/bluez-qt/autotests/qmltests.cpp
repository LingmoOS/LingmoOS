/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "autotests.h"

#include <QDBusObjectPath>

#include <QQmlEngine>
#include <QtQuickTest> // krazy:exclude=includes

#ifdef IMPORT_EXTENSIONS_QML_PLUGIN
#include <QPluginLoader>
Q_IMPORT_PLUGIN(BluezQtExtensionPlugin)
#endif

class FakeBluezObject : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void start()
    {
        FakeBluez::start();
    }

    void stop()
    {
        FakeBluez::stop();
    }

    bool isRunning()
    {
        return FakeBluez::isRunning();
    }

    void runTest(const QString &testName)
    {
        FakeBluez::runTest(testName);
    }

    static void processProperties(QVariantMap &properties)
    {
        const QStringList &toDBusObjectPath = properties.value(QStringLiteral("_toDBusObjectPath")).toStringList();
        for (const QString &name : toDBusObjectPath) {
            const QString &val = properties.value(name).toString();
            properties[name] = QVariant::fromValue(QDBusObjectPath(val));
        }
        properties.remove(QStringLiteral("_toDBusObjectPath"));

        QMapIterator<QString, QVariant> it(properties);
        while (it.hasNext()) {
            it.next();
            if (it.key() == QLatin1String("UUIDs")) {
                properties[it.key()] = it.value().toStringList();
            } else if (it.value().userType() == QMetaType::QVariantMap) {
                QVariantMap props = it.value().toMap();
                processProperties(props);
                properties[it.key()] = props;
            }
        }
    }

    void runAction(const QString &object, const QString &actionName, QVariantMap properties = QVariantMap())
    {
        processProperties(properties);
        FakeBluez::runAction(object, actionName, properties);
    }
};

static QObject *fakebluez_singleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new FakeBluezObject;
}

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

int main(int argc, char *argv[])
{
    qmlRegisterSingletonType<FakeBluezObject>("org.kde.bluezqt.fakebluez", 1, 0, "FakeBluez", fakebluez_singleton);

    BluezQt::bluezqt_initFakeBluezTestRun();

    qputenv("QML2_IMPORT_PATH", QByteArrayLiteral(BLUEZQT_QML_IMPORT_PATH));
    const QString &testsDir = QFileInfo(QFINDTESTDATA("qml/tst_device.qml")).absolutePath();

    return quick_test_main(argc, argv, "qmltests", testsDir.toUtf8().constData());
}

#include "qmltests.moc"
