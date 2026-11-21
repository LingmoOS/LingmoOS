#ifndef HINTSSETTINGS_H
#define HINTSSETTINGS_H

#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include <QSettings>
#include <QVariant>

#include <qpa/qplatformtheme.h>

class HintsSettings : public QObject
{
    Q_OBJECT

public:
    explicit HintsSettings(QObject *parent = nullptr);
    ~HintsSettings() override;

    QStringList xdgIconThemePaths() const;

    inline QVariant hint(QPlatformTheme::ThemeHint hint) const { return m_hints[hint]; }

    QString systemFont() const;
    QString systemFixedFont() const;
    qreal systemFontPointSize() const;

    bool darkMode();
    int accentColor() const;

public Q_SLOTS:
    void lazyInit();
    void updateCursorHints();
    void updateIconTheme();

Q_SIGNALS:
    void systemFontChanged(QString font);
    void systemFixedFontChanged(QString fixedFont);
    void systemFontPointSizeChanged(qreal systemFontPointSize);
    void iconThemeChanged();
    void cursorThemeChanged();
    void darkModeChanged(bool darkMode);
    void accentColorChanged(int accentColor);

private:
    void onFileChanged(const QString &path);
    void updateIconThemeHint();

private:
    QHash<QPlatformTheme::ThemeHint, QVariant> m_hints;
    QSettings *m_settings;
    QString m_settingsFile;
    QFileSystemWatcher *m_fileWatcher;
    QVariantMap m_values;

    QString m_iconTheme;
};

#endif // HINTSSETTINGS_H
