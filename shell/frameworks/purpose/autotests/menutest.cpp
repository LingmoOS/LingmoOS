/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QStandardPaths>
#include <qtest.h>

#include "menutest.h"
#include <purpose/alternativesmodel.h>
#include <purpose/menu.h>

QTEST_MAIN(MenuTest)

static QAction *saveAsAction(Purpose::Menu *menu)
{
    const auto actions = menu->actions();
    for (QAction *action : actions) {
        if (action->property("pluginId") == QLatin1String("saveasplugin")) {
            return action;
        }
    }

    Q_ASSERT(!"Couldn't find the saveas plugin");
    return nullptr;
}

void MenuTest::initTestCase()
{
    // To let ctest exit, we shouldn't start kio_http_cache_cleaner
    qputenv("KIO_DISABLE_CACHE_CLEANER", "yes");
}

void MenuTest::runJobTest()
{
    Purpose::Menu *menu = new Purpose::Menu;
    Purpose::AlternativesModel *model = menu->model();
    model->setDisabledPlugins({});

    const QString tempfile = m_tempDir.path() + QStringLiteral("/purposetest");
    QFile::remove(tempfile);
    const QJsonObject input = QJsonObject{{QStringLiteral("urls"), QJsonArray{QStringLiteral("http://kde.org")}},
                                          {QStringLiteral("mimeType"), QStringLiteral("dummy/thing")},
                                          {QStringLiteral("destinationPath"), QUrl::fromLocalFile(tempfile).url()}};
    model->setInputData(input);
    model->setPluginType(QStringLiteral("Export"));
    menu->reload();

    int error = -1;
    QJsonObject output;
    connect(menu, &Purpose::Menu::finished, menu, [&error, &output](const QJsonObject &_output, int _error, const QString &errorMessage) {
        error = _error;
        output = _output;
        if (error != 0) {
            qDebug() << "job failed with error" << errorMessage;
        }
    });

    QAction *action = saveAsAction(menu);
    QSignalSpy s(menu, &Purpose::Menu::finished);
    action->trigger();
    QVERIFY(s.count() || s.wait());
    QCOMPARE(error, 0);
    QCOMPARE(output.count(), 1);
    QVERIFY(QFile::remove(tempfile));

    delete menu;
}

#include "moc_menutest.cpp"
