#include "hintsettings.h"

#include <QDir>
#include <QFileInfo>
#include <QSize>
#include <QStandardPaths>
#include <QString>
#include <qpa/qplatformdialoghelper.h>

#include <QDBusConnection>
#include <QFontDatabase>

#include <utility>

static const QByteArray s_systemFontName = QByteArrayLiteral("Font");
static const QByteArray s_systemFixedFontName = QByteArrayLiteral("FixedFont");
static const QByteArray s_systemPointFontSize = QByteArrayLiteral("FontSize");
static const QByteArray s_darkModeName = QByteArrayLiteral("DarkMode");
static const QByteArray s_accentColorName = QByteArrayLiteral("AccentColor");
static const QString s_defaultIconName = QStringLiteral("Lingmo");

static QString availableFontFamily(const QString &requested, const QString &fallback)
{
    const QStringList families = QFontDatabase::families();

    if (families.contains(requested))
        return requested;
    if (families.contains(fallback))
        return fallback;

    return QStringLiteral("Sans Serif");
}

static QVariantMap settingsValues(QSettings *settings)
{
    QVariantMap values;
    for (const QString &key : settings->allKeys())
        values.insert(key, settings->value(key));
    return values;
}

HintsSettings::HintsSettings(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
                                       + "/lingmo/services.conf",
                               QSettings::IniFormat, this))
{
    m_settings->beginGroup("Appearance");
    m_values = settingsValues(m_settings);
    m_iconTheme = m_settings->value("IconTheme", s_defaultIconName).toString();
    updateIconThemeHint();

    m_hints[QPlatformTheme::StyleNames] = QStringList {QStringLiteral("Lingmo"),
                                                        QStringLiteral("Fusion")};
    m_hints[QPlatformTheme::SystemIconFallbackThemeName] = QStringLiteral("hicolor");
    m_hints[QPlatformTheme::IconThemeSearchPaths] = xdgIconThemePaths();
    m_hints[QPlatformTheme::UseFullScreenForPopupMenu] = false;
    m_hints[QPlatformTheme::DialogButtonBoxLayout] = QPlatformDialogHelper::MacLayout;

    m_settingsFile = m_settings->fileName();
    updateCursorHints();
    for (const auto *signal : {"cursorThemeChanged", "cursorSizeChanged"})
        QDBusConnection::sessionBus().connect(
                "org.lingmo.Services", "/org/lingmo/Services/Appearance",
                "org.lingmo.Services.Appearance", QLatin1String(signal), this,
                SLOT(updateCursorHints()));
    // Delivered in order before any application-side icon reload that listens to the same
    // signal, so QIconLoader is on the new theme first whatever the file watcher does.
    QDBusConnection::sessionBus().connect("org.lingmo.Services",
                                          "/org/lingmo/Services/Appearance",
                                          "org.lingmo.Services.Appearance", "iconThemeChanged",
                                          this, SLOT(updateIconTheme()));

    QMetaObject::invokeMethod(this, "lazyInit", Qt::QueuedConnection);
}

HintsSettings::~HintsSettings()
{
}

void HintsSettings::lazyInit()
{
    m_fileWatcher = new QFileSystemWatcher(this);
    m_fileWatcher->addPath(QFileInfo(m_settingsFile).absolutePath());
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, this,
            &HintsSettings::onFileChanged);
    m_fileWatcher->addPath(m_settingsFile);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &HintsSettings::onFileChanged);
}

QStringList HintsSettings::xdgIconThemePaths() const
{
    QStringList paths;

    // make sure we have ~/.local/share/icons in paths if it exists
    paths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"),
                                       QStandardPaths::LocateDirectory);

    const QFileInfo homeIconDir(QDir::homePath() + QStringLiteral("/.icons"));
    if (homeIconDir.isDir()) {
        paths << homeIconDir.absoluteFilePath();
    }

    return paths;
}

QString HintsSettings::systemFont() const
{
    return availableFontFamily(m_settings->value(s_systemFontName, "Noto Sans").toString(),
                               QStringLiteral("DejaVu Sans"));
}

QString HintsSettings::systemFixedFont() const
{
    return availableFontFamily(m_settings->value(s_systemFixedFontName, "Noto Mono").toString(),
                               QStringLiteral("DejaVu Sans Mono"));
}

qreal HintsSettings::systemFontPointSize() const
{
    return m_settings->value(s_systemPointFontSize, 9).toDouble();
}

void HintsSettings::updateIconThemeHint()
{
    m_hints[QPlatformTheme::SystemIconThemeName] =
            m_iconTheme.isEmpty() ? s_defaultIconName : m_iconTheme;
}

bool HintsSettings::darkMode()
{
    return m_settings->value(s_darkModeName, false).toBool();
}

int HintsSettings::accentColor() const
{
    return m_settings->value(s_accentColorName, 0).toInt();
}

void HintsSettings::onFileChanged(const QString &path)
{
    Q_UNUSED(path);

    // The appearance signals sync the settings too, so the values before this
    // change are the ones seen here last, not what the settings hold now.
    m_settings->sync();
    const QVariantMap map = std::exchange(m_values, settingsValues(m_settings));

    for (const QString &value : m_values.keys()) {
        const QVariant &oldValue = map.value(value);
        const QVariant &newValue = m_values.value(value);

        if (oldValue != newValue) {
            if (value == s_systemFontName)
                emit systemFontChanged(newValue.toString());
            else if (value == s_systemFixedFontName)
                emit systemFixedFontChanged(newValue.toString());
            else if (value == s_systemPointFontSize)
                emit systemFontPointSizeChanged(newValue.toDouble());
            else if (value == s_darkModeName)
                emit darkModeChanged(newValue.toBool());
            else if (value == s_accentColorName)
                emit accentColorChanged(newValue.toInt());
            else if (value == "IconTheme")
                updateIconTheme();
        }
    }

    bool fileDeleted = !m_fileWatcher->files().contains(m_settingsFile);
    if (fileDeleted)
        m_fileWatcher->addPath(m_settingsFile);
    updateCursorHints();
}

void HintsSettings::updateIconTheme()
{
    m_settings->sync();
    const QString iconTheme = m_settings->value("IconTheme", s_defaultIconName).toString();
    if (iconTheme == m_iconTheme)
        return;

    m_iconTheme = iconTheme;
    updateIconThemeHint();
    emit iconThemeChanged();
}

void HintsSettings::updateCursorHints()
{
    m_settings->sync();
    const auto theme = m_settings->value("CursorTheme", "default").toString();
    const int size = qBound(16, m_settings->value("CursorSize", 24).toInt(), 64);
    if (m_hints.value(QPlatformTheme::MouseCursorTheme) == theme
        && m_hints.value(QPlatformTheme::MouseCursorSize) == QSize(size, size))
        return;
    m_hints[QPlatformTheme::MouseCursorTheme] = theme;
    m_hints[QPlatformTheme::MouseCursorSize] = QSize(size, size);
    emit cursorThemeChanged();
}
