/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KFileWidget>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QTest>

class KFileDialogQml_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        m_engine = new QQmlEngine;
    }

    void cleanupTestCase()
    {
        delete m_engine;
    }

    void testShowDialogParentless()
    {
        KFileWidget *fw;
        {
            QQmlComponent component(m_engine);
            component.loadUrl(QUrl::fromLocalFile(QFINDTESTDATA("qml/filedialog_parentless.qml")));
            QScopedPointer<QObject> object(component.create());
            QVERIFY(!object.isNull());

            fw = findFileWidget();
            QVERIFY(fw);
            QCOMPARE(fw->isVisible(), true);
            fw->slotCancel();
        }
    }

    void testShowDialogWithParent()
    {
        KFileWidget *fw;
        {
            QQmlComponent component(m_engine);
            component.loadUrl(QUrl::fromLocalFile(QFINDTESTDATA("qml/filedialog_withparent.qml")));
            QScopedPointer<QObject> object(component.create());
            QVERIFY(!object.isNull());

            fw = findFileWidget();
            QVERIFY(fw);
            QCOMPARE(fw->isVisible(), true);
            fw->slotCancel();
        }
    }

private:
    static KFileWidget *findFileWidget()
    {
        QList<KFileWidget *> widgets;
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            KFileWidget *fw = widget->findChild<KFileWidget *>();
            if (fw) {
                widgets.append(fw);
            }
        }
        Q_ASSERT(widgets.count() == 1);
        return (widgets.count() == 1) ? widgets.first() : nullptr;
    }

    QQmlEngine *m_engine = nullptr;
};

QTEST_MAIN(KFileDialogQml_UnitTest)

#include "kfiledialogqml_unittest.moc"
