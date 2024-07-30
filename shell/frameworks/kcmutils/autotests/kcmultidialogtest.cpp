/*
    SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCMultiDialog>

#include <QObject>
#include <QTest>
#include <QTimer>

class KCMultiDialogTest : public QObject
{
    Q_OBJECT
    const KPluginMetaData fakekcm{QStringLiteral("plasma/kcms/systemsettings_qwidgets/fakekcm")};
    void sleep(int ms)
    {
        QEventLoop l;
        QTimer::singleShot(ms, this, [&l]() {
            l.quit();
        });
        l.exec();
    }
private Q_SLOTS:
    void testClear()
    {
        KCMultiDialog dialog;
        dialog.addModule(fakekcm);
        // Just verify that it doesn't crash
        dialog.clear();
    }
    void testLoadKcm()
    {
        KCMultiDialog dialog;
        // Simple property to check how often the load method was called
        QCOMPARE(dialog.property("loadcalled").toInt(), 0);

        // For the first KCM, it should be called once
        auto page1 = dialog.addModule(fakekcm);
        sleep(1);

        // For the newly instantiated KCM, load should be called after we change the page
        // Because it does not have a higher weight, it is not shown initially
        auto page2 = dialog.addModule(fakekcm);
        dialog.setCurrentPage(page2);
        sleep(1);
        QCOMPARE(dialog.property("loadcalled").toInt(), 2);

        // Go back to page 1, load should not be called again
        dialog.setCurrentPage(page1);
        sleep(1);
        QCOMPARE(dialog.property("loadcalled").toInt(), 2);
    }
};

QTEST_MAIN(KCMultiDialogTest)
#include "kcmultidialogtest.moc"
