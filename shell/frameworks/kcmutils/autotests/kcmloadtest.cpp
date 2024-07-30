/*
    SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCModuleLoader>

#include <QObject>
#include <QQmlEngine>
#include <QTest>

class KCMTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoadQmlPlugin()
    {
        auto mod = KCModuleLoader::loadModule(KPluginMetaData(QStringLiteral("plasma/kcms/systemsettings/kcm_testqml")));
        QVERIFY(mod);
        QCOMPARE(mod->metaObject()->className(), "KCModuleQml");
    }

    void testFallbackKCM()
    {
        auto modFail = KCModuleLoader::loadModule(KPluginMetaData(QStringLiteral("nonexistent_kcm")));
        QVERIFY(modFail);
        QCOMPARE(modFail->metaObject()->className(), "KCMError");
    }
};

QTEST_MAIN(KCMTest)
#include "kcmloadtest.moc"
