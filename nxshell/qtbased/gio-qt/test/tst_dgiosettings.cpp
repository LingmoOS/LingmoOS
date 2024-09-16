// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QString>
#include <QtTest>

#include <dgiofile.h>
#include <dgiofileinfo.h>

#include "dgiosettings.h"

class DGioSettingsTest : public QObject {
    Q_OBJECT
public:
    DGioSettingsTest() {}

private Q_SLOTS:
    void testCase_Settings() {
        if (DGioSettings::isSchemaInstalled("com.deepin.dde.appearance") == false) {
            qWarning() << "com.deepin.dde.appearance" << "invalid";
            return;
        }

        DGioSettings settings("com.deepin.dde.appearance", this);
        QVERIFY(settings.value("background-uris").isValid());
        QVERIFY(settings.value("extra-picture-uris").isValid());
        const QStringList& tmpValue = settings.value("extra-picture-uris").toStringList();
        settings.setValue("extra-picture-uris", QStringList());
        settings.setValue("extra-picture-uris", QStringList() << "ddd");
        settings.sync();
        QVERIFY(settings.value("extra-picture-uris").toStringList() == QStringList() << "ddd");
        settings.setValue("extra-picture-uris", tmpValue, true);
        QVERIFY(settings.value("extra-picture-uris").toStringList() == tmpValue);

        QVERIFY(settings.keys().contains("extra-picture-uris"));
    }
};

QTEST_APPLESS_MAIN(DGioSettingsTest)

#include "tst_dgiosettings.moc"
