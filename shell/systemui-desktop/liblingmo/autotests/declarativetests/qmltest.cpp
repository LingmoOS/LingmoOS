// SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QQmlEngine>
#include <QtQuickTest>

class Setup : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Setup)

public:
    explicit Setup()
    {
    }

public Q_SLOTS:
    void applicationAvailable()
    {
    }

    void qmlEngineAvailable(QQmlEngine *engine)
    {
        // Initialization requiring the QQmlEngine to be constructed
        engine->setProperty("_lingmouiTheme", QStringLiteral("LingmoUILingmoStyle"));
    }

    void cleanupTestCase()
    {
    }
};

QUICK_TEST_MAIN_WITH_SETUP(QuickTest, Setup)

#include "qmltest.moc"
