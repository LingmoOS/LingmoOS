/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QJsonArray>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QSignalSpy>
#include <QStandardPaths>
#include <qtest.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include "alternativesmodeltest.h"
#include <purpose/alternativesmodel.h>
#include <purpose/configuration.h>
#include <purpose/job.h>

QTEST_GUILESS_MAIN(AlternativesModelTest)

static int saveAsRow(Purpose::AlternativesModel *model)
{
    for (int i = 0, c = model->rowCount(); i < c; ++i) {
        QString pluginId = model->index(i).data(Purpose::AlternativesModel::PluginIdRole).toString();
        if (pluginId == QLatin1String("saveasplugin")) {
            return i;
        }
    }

    Q_ASSERT(!"Couldn't find the saveas plugin");
    return -1;
}

void AlternativesModelTest::initTestCase()
{
    // To let ctest exit, we shouldn't start kio_http_cache_cleaner
    qputenv("KIO_DISABLE_CACHE_CLEANER", "yes");

    if (qEnvironmentVariableIsSet("QT_LOGGING_RULES")) { // as is the case in CI
        // CopyJob debug output is too noisy because of the huge data URL we're using in bigBufferTest.
        qputenv("QT_LOGGING_RULES", QByteArray(qgetenv("QT_LOGGING_RULES") + QByteArrayLiteral(";kf.kio.core.copyjob.debug=false")));
    }
}

void AlternativesModelTest::runJobTest()
{
    Purpose::AlternativesModel model;

    const QString tempfile = m_tempDir.path() + QStringLiteral("/purposetest");
    QFile::remove(tempfile);
    QJsonObject input =
        QJsonObject{{QStringLiteral("urls"), QJsonArray{QStringLiteral("http://kde.org")}}, {QStringLiteral("mimeType"), QStringLiteral("dummy/thing")}};
    model.setInputData(input);

    model.setPluginType(QStringLiteral("Export"));
    model.setDisabledPlugins({});

    Purpose::Configuration *conf = model.configureJob(saveAsRow(&model));
    QVERIFY(!conf->isReady());
    QVERIFY(!conf->createJob());
    input.insert(QStringLiteral("destinationPath"), QUrl::fromLocalFile(tempfile).url()), conf->setData(input);
    QVERIFY(conf->isReady());
    Purpose::Job *job = conf->createJob();
    QVERIFY(job);
    QSignalSpy sOutput(job, &Purpose::Job::outputChanged);
    QVERIFY2(job->exec(), qPrintable(job->errorString()));
    QCOMPARE(sOutput.count(), 1);
    QVERIFY(QFile::remove(tempfile));
}

void AlternativesModelTest::bigBufferTest()
{
    Purpose::AlternativesModel model;

    const QByteArray payload(1920 * 1080 * 4, 'x');
    const QString uri = QStringLiteral("data:text/plain;base64,") + QString::fromLatin1(payload.toBase64());

    const QString tempfile = m_tempDir.path() + QStringLiteral("/purposetest");
    QFile::remove(tempfile);
    const QJsonObject input = {{QStringLiteral("urls"), QJsonArray{uri}},
                               {QStringLiteral("mimeType"), QStringLiteral("dummy/thing")},
                               {QStringLiteral("destinationPath"), QUrl::fromLocalFile(tempfile).url()}};
    model.setInputData(input);
    model.setPluginType(QStringLiteral("Export"));
    model.setDisabledPlugins({});

    Purpose::Configuration *conf = model.configureJob(saveAsRow(&model));
    QVERIFY(conf->isReady());
    conf->setUseSeparateProcess(false);
    Purpose::Job *job = conf->createJob();
    QVERIFY(job);
    QSignalSpy sOutput(job, &Purpose::Job::outputChanged);
    QVERIFY2(job->exec(), qPrintable(job->errorString()));
    QCOMPARE(sOutput.count(), 1);

    QFileInfo fi(tempfile);
    QCOMPARE(fi.size(), payload.size());
    QVERIFY(QFile::remove(tempfile));
}

void AlternativesModelTest::disablePluginTest()
{
    const auto listPlugins = [] {
        QStringList plugins;
        Purpose::AlternativesModel model;
        QJsonObject input =
            QJsonObject{{QStringLiteral("urls"), QJsonArray{QStringLiteral("http://kde.org")}}, {QStringLiteral("mimeType"), QStringLiteral("dummy/thing")}};
        model.setInputData(input);
        model.setPluginType(QStringLiteral("Export"));
        model.setDisabledPlugins({});

        for (int i = 0; i < model.rowCount(); ++i) {
            plugins << model.index(i).data(Purpose::AlternativesModel::PluginIdRole).toString();
        }
        return plugins;
    };

    auto plugins = listPlugins();
    QVERIFY(plugins.contains(QStringLiteral("saveasplugin")));
    QVERIFY(plugins.contains(QStringLiteral("emailplugin")));

    QStandardPaths::setTestModeEnabled(true);
    auto config = KSharedConfig::openConfig(QStringLiteral("purposerc"));
    auto group = config->group(QStringLiteral("plugins"));
    group.writeEntry("disabled", QStringList{QStringLiteral("saveasplugin"), QStringLiteral("emailplugin")});

    plugins = listPlugins();
    QVERIFY(!plugins.contains(QStringLiteral("saveasplugin")));
    QVERIFY(!plugins.contains(QStringLiteral("emailplugin")));

    // "cleanup"
    group.writeEntry("disabled", QStringList());
}

void AlternativesModelTest::blacklistTest()
{
    const auto listPlugins = [](const QStringList &blacklist) {
        QStringList plugins;
        Purpose::AlternativesModel model;
        QJsonObject input =
            QJsonObject{{QStringLiteral("urls"), QJsonArray{QStringLiteral("http://kde.org")}}, {QStringLiteral("mimeType"), QStringLiteral("dummy/thing")}};
        model.setInputData(input);
        model.setPluginType(QStringLiteral("Export"));
        if (!blacklist.isEmpty()) {
            model.setDisabledPlugins(blacklist);
        }

        for (int i = 0; i < model.rowCount(); ++i) {
            plugins << model.index(i).data(Purpose::AlternativesModel::PluginIdRole).toString();
        }
        return plugins;
    };

    auto plugins = listPlugins(QStringList());
    QVERIFY(plugins.contains(QStringLiteral("emailplugin")));

    plugins = listPlugins({QStringLiteral("emailplugin")});
    QVERIFY(!plugins.contains(QStringLiteral("emailplugin")));

    plugins = listPlugins({QStringLiteral("saveasplugin")});
    QVERIFY(plugins.contains(QStringLiteral("emailplugin")));

    // Admin settings have precedence
    QStandardPaths::setTestModeEnabled(true);
    auto config = KSharedConfig::openConfig(QStringLiteral("purposerc"));
    auto group = config->group(QStringLiteral("plugins"));
    group.writeEntry("disabled", QStringList{QStringLiteral("emailplugin")});

    plugins = listPlugins({QStringLiteral("saveasplugin")});
    QVERIFY(!plugins.contains(QStringLiteral("emailplugin")));
}

#include "moc_alternativesmodeltest.cpp"
