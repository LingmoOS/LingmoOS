/*
    SPDX-FileCopyrightText: 2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "partloader.h"
#include <KParts/PartLoader>
#include <KParts/ReadOnlyPart>
#include <QTest>

#include <KPluginMetaData>
#include <QDebug>
#include <QJsonArray>
#include <QStandardPaths>

class PartLoaderTest : public QObject
{
    Q_OBJECT
private:
    const QString m_plainTextMimetype = QStringLiteral("text/plain");
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);

        // Ensure notepadpart is preferred over other installed parts.
        // This also tests the mimeapps.list parsing in PartLoader
        const QByteArray contents =
            "[Added KDE Service Associations]\n"
            "text/plain=notepad.desktop;\n";
        const QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
        QDir().mkpath(configDir);
        const QString mimeAppsPath = configDir + QLatin1String("/mimeapps.list");
        QFile mimeAppsFile(mimeAppsPath);
        QVERIFY(mimeAppsFile.open(QIODevice::WriteOnly));
        mimeAppsFile.write(contents);
    }

    void shouldListParts()
    {
        const QList<KPluginMetaData> plugins = KParts::PartLoader::partsForMimeType(m_plainTextMimetype);

        QVERIFY(!plugins.isEmpty());
        QCOMPARE(plugins.at(0).pluginId(), QStringLiteral("notepadpart")); // basename of plugin
        const QString fileName = plugins.at(0).fileName();
        QVERIFY2(fileName.contains(QLatin1String("notepadpart")), qPrintable(fileName));
    }

    void shouldLoadPlainTextPart()
    {
        const QString testFile = QFINDTESTDATA("partloadertest.cpp");
        QVERIFY(!testFile.isEmpty());
        QWidget parentWidget;

        auto res = KParts::PartLoader::instantiatePartForMimeType<KParts::ReadOnlyPart>(m_plainTextMimetype, &parentWidget, this);

        QVERIFY(res);
        QCOMPARE(res.errorString, QString());
        QCOMPARE(res.plugin->metaObject()->className(), "NotepadPart");
        QVERIFY(res.plugin->openUrl(QUrl::fromLocalFile(testFile)));
    }

    void shouldHandleNoPartError()
    {
        // can't use an unlikely mimetype here, okteta is associated with application/octet-stream :-)
        const QString mimeType = QStringLiteral("does/not/exist");
        QWidget parentWidget;

        const KPluginFactory::Result result = KParts::PartLoader::instantiatePartForMimeType<KParts::ReadOnlyPart>(mimeType, &parentWidget, this);

        QVERIFY2(!result, result ? result.plugin->metaObject()->className() : nullptr);
        QCOMPARE(result.errorString, QStringLiteral("No part was found for mimeType does/not/exist"));
    }

    void shouldInstantiatePart()
    {
        const KPluginMetaData md(QStringLiteral("kf6/parts/notepadpart"));
        QVERIFY(md.isValid());

        QWidget parentWidget;
        const KPluginFactory::Result result = KParts::PartLoader::instantiatePart<KParts::ReadOnlyPart>(md, &parentWidget, this);

        QVERIFY(result);
        QCOMPARE(result.plugin->metaObject()->className(), "NotepadPart");
    }

    void testPartCapabilities()
    {
        const KPluginMetaData md(QStringLiteral("kf6/parts/notepadpart"));
        QVERIFY(md.isValid());

        QCOMPARE(KParts::PartLoader::partCapabilities(md), KParts::PartCapability::BrowserView | KParts::PartCapability::ReadWrite);
    }

    void testPartCapabilitiesCompat()
    {
        const KPluginMetaData md(QStringLiteral("kf6/parts/notepadpart"));
        QJsonObject obj = md.rawData();
        QJsonObject kplugin = obj[QLatin1String("KPlugin")].toObject();
        kplugin[QLatin1String("ServiceTypes")] = QJsonValue::fromVariant(QStringList{QStringLiteral("Browser/View")});
        obj[QLatin1String("KPlugin")] = kplugin;
        obj.remove(QLatin1String("KParts"));

        const auto caps = KParts::PartLoader::partCapabilities(KPluginMetaData(obj, md.fileName()));
        QCOMPARE(caps, QFlags(KParts::PartCapability::BrowserView));
    }
};

QTEST_MAIN(PartLoaderTest)

#include "partloadertest.moc"
