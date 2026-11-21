// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedialoghelper.h"
#include "hintsettings.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QMimeDatabase>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

struct PortalRule {
    uint type = 0;
    QString value;
};

struct PortalFilter {
    QString name;
    QList<PortalRule> rules;
};

Q_DECLARE_METATYPE(PortalRule)
Q_DECLARE_METATYPE(PortalFilter)

static QDBusArgument &operator<<(QDBusArgument &argument, const PortalRule &rule)
{
    argument.beginStructure();
    argument << rule.type << rule.value;
    argument.endStructure();
    return argument;
}

static const QDBusArgument &operator>>(const QDBusArgument &argument, PortalRule &rule)
{
    argument.beginStructure();
    argument >> rule.type >> rule.value;
    argument.endStructure();
    return argument;
}

static QDBusArgument &operator<<(QDBusArgument &argument, const PortalFilter &filter)
{
    argument.beginStructure();
    argument << filter.name << filter.rules;
    argument.endStructure();
    return argument;
}

static const QDBusArgument &operator>>(const QDBusArgument &argument, PortalFilter &filter)
{
    argument.beginStructure();
    argument >> filter.name >> filter.rules;
    argument.endStructure();
    return argument;
}

// Answers every request at once with the response and results set beforehand.
class FakeFileChooser : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.portal.FileChooser")

public:
    explicit FakeFileChooser(const QDBusConnection &connection)
        : m_connection(connection)
    {
    }

    uint response = 0;
    QVariantMap results;
    QVariantMap options;

public Q_SLOTS:
    QDBusObjectPath OpenFile(const QString &parent, const QString &title,
                             const QVariantMap &options, const QDBusMessage &message)
    {
        Q_UNUSED(parent)
        Q_UNUSED(title)
        return request(options, message);
    }

    QDBusObjectPath SaveFile(const QString &parent, const QString &title,
                             const QVariantMap &options, const QDBusMessage &message)
    {
        Q_UNUSED(parent)
        Q_UNUSED(title)
        return request(options, message);
    }

private:
    QDBusObjectPath request(const QVariantMap &options, const QDBusMessage &message)
    {
        this->options = options;
        const QString sender = message.service().mid(1).replace(QLatin1Char('.'), QLatin1Char('_'));
        const QString path = QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
                .arg(sender, options.value(QStringLiteral("handle_token")).toString());
        QMetaObject::invokeMethod(this, [this, path] {
            QDBusMessage signal = QDBusMessage::createSignal(
                    path, QStringLiteral("org.freedesktop.portal.Request"), QStringLiteral("Response"));
            signal << response << results;
            m_connection.send(signal);
        }, Qt::QueuedConnection);
        return QDBusObjectPath(path);
    }

    QDBusConnection m_connection;
};

class PlatformThemeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void multipleSelectionSelectsNoSingleFile();
    void defaultMimeTypeIsAllFiles();
    void settingsChangeSurvivesAnEarlierSync();
    void defaultStylePrefersLingmo();
    void accentColorIsReadFromAppearanceSettings();

private:
    QDBusConnection m_portalBus = QDBusConnection(QString());
    FakeFileChooser *m_portal = nullptr;
};

static bool writeSettings(const QByteArray &contents)
{
    const QString directory =
            QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/lingmo";
    QFile file(directory + "/services.conf");
    return QDir().mkpath(directory) && file.open(QIODevice::WriteOnly | QIODevice::Truncate)
        && file.write(contents) == contents.size();
}

void PlatformThemeTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    qDBusRegisterMetaType<PortalRule>();
    qDBusRegisterMetaType<QList<PortalRule>>();
    qDBusRegisterMetaType<PortalFilter>();
    qDBusRegisterMetaType<QList<PortalFilter>>();

    QVERIFY(QDBusConnection::sessionBus().isConnected());
    // The helper must see the portal as another peer, as it is in a session.
    m_portalBus = QDBusConnection::connectToBus(QDBusConnection::SessionBus,
                                                QStringLiteral("fake-portal"));
    QVERIFY(m_portalBus.isConnected());
    m_portal = new FakeFileChooser(m_portalBus);
    QVERIFY(m_portalBus.registerService(QStringLiteral("org.freedesktop.portal.Desktop")));
    QVERIFY(m_portalBus.registerObject(QStringLiteral("/org/freedesktop/portal/desktop"), m_portal,
                                       QDBusConnection::ExportAllSlots));
}

void PlatformThemeTest::multipleSelectionSelectsNoSingleFile()
{
    FileDialogHelper helper;
    auto options = QFileDialogOptions::create();
    options->setFileMode(QFileDialogOptions::ExistingFiles);
    helper.setOptions(options);

    m_portal->response = 0;
    m_portal->results = {{QStringLiteral("uris"),
                          QStringList {QStringLiteral("file:///tmp/a.txt"),
                                       QStringLiteral("file:///tmp/b.txt")}}};
    QSignalSpy accepted(&helper, &QPlatformDialogHelper::accept);
    QSignalSpy fileSelected(&helper, &QPlatformFileDialogHelper::fileSelected);
    QVERIFY(helper.show(Qt::Dialog, Qt::ApplicationModal, nullptr));
    QVERIFY(accepted.wait());

    QCOMPARE(m_portal->options.value(QStringLiteral("multiple")).toBool(), true);
    QCOMPARE(helper.selectedFiles().size(), 2);
    // QFileDialog emits fileSelected itself, and only for a single file.
    QCOMPARE(fileSelected.count(), 0);
}

void PlatformThemeTest::defaultMimeTypeIsAllFiles()
{
    const QString png = QStringLiteral("image/png");
    const QString other = QStringLiteral("application/octet-stream");
    const QMimeType pngType = QMimeDatabase().mimeTypeForName(png);
    const QString pngFilter =
            pngType.comment() + QStringLiteral(" (") + pngType.globPatterns().join(QLatin1Char(' ')) + QLatin1Char(')');
    const QString allFilter = QStringLiteral("All files (*)");

    FileDialogHelper helper;
    auto options = QFileDialogOptions::create();
    options->setFileMode(QFileDialogOptions::ExistingFile);
    // As QFileDialog::setMimeTypeFilters() and selectMimeTypeFilter() leave them.
    options->setMimeTypeFilters({png, other});
    options->setNameFilters({pngFilter, allFilter});
    options->setInitiallySelectedMimeTypeFilter(other);
    options->setInitiallySelectedNameFilter(allFilter);
    helper.setOptions(options);

    m_portal->response = 0;
    m_portal->results = {
        {QStringLiteral("uris"), QStringList {QStringLiteral("file:///tmp/a.bin")}},
        {QStringLiteral("current_filter"),
         QVariant::fromValue(PortalFilter {QStringLiteral("All files"), {{0, QStringLiteral("*")}}})},
    };
    QSignalSpy accepted(&helper, &QPlatformDialogHelper::accept);
    QVERIFY(helper.show(Qt::Dialog, Qt::ApplicationModal, nullptr));
    QVERIFY(accepted.wait());

    const auto filters =
            qdbus_cast<QList<PortalFilter>>(m_portal->options.value(QStringLiteral("filters")));
    QCOMPARE(filters.size(), 2);
    QCOMPARE(filters.at(0).name, pngType.comment());
    QCOMPARE(filters.at(1).name, QStringLiteral("All files"));
    QCOMPARE(filters.at(1).rules.size(), 1);
    QCOMPARE(filters.at(1).rules.at(0).value, QStringLiteral("*"));
    const auto current =
            qdbus_cast<PortalFilter>(m_portal->options.value(QStringLiteral("current_filter")));
    QCOMPARE(current.name, QStringLiteral("All files"));
    // QFileDialog::selectedMimeTypeFilter() finds the type by this name filter.
    QCOMPARE(helper.selectedNameFilter(), allFilter);
}

void PlatformThemeTest::settingsChangeSurvivesAnEarlierSync()
{
    QVERIFY(writeSettings("[Appearance]\nDarkMode=false\nIconTheme=A\n"));
    HintsSettings hints;
    QCoreApplication::processEvents();
    QSignalSpy darkMode(&hints, &HintsSettings::darkModeChanged);

    QVERIFY(writeSettings("[Appearance]\nDarkMode=true\nIconTheme=Second\n"));
    // The icon theme signal can come before the file watcher sees the write.
    hints.updateIconTheme();
    QVERIFY(darkMode.wait(2000));
    QCOMPARE(darkMode.constFirst().constFirst().toBool(), true);
    QCOMPARE(hints.darkMode(), true);
}

void PlatformThemeTest::defaultStylePrefersLingmo()
{
    HintsSettings hints;
    const QStringList styles = hints.hint(QPlatformTheme::StyleNames).toStringList();
    QVERIFY(!styles.isEmpty());
    QCOMPARE(styles.constFirst(), QStringLiteral("Lingmo"));
}

void PlatformThemeTest::accentColorIsReadFromAppearanceSettings()
{
    QVERIFY(writeSettings("[Appearance]\nAccentColor=5\n"));
    HintsSettings hints;
    QCOMPARE(hints.accentColor(), 5);
}

QTEST_MAIN(PlatformThemeTest)

#include "platformtheme_test.moc"
