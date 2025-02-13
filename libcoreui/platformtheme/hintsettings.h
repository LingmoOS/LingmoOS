#ifndef HINTSSETTINGS_H
#define HINTSSETTINGS_H

#include <QDBusVariant>
#include <QFileSystemWatcher>
#include <QObject>
#include <QVariant>
#include <QSettings>

#include <qpa/qplatformtheme.h>

class QPalette;
class HintsSettings : public QObject
{
    Q_OBJECT

public:
    explicit HintsSettings(QObject *parent = nullptr);
    ~HintsSettings() override;

    QStringList xdgIconThemePaths() const;

    inline QVariant hint(QPlatformTheme::ThemeHint hint) const {
        return m_hints[hint];
    }

    QString systemFont() const;
    QString systemFixedFont() const;
    qreal systemFontPointSize() const;

    bool darkMode();

public Q_SLOTS:
    void lazyInit();

Q_SIGNALS:
    void systemFontChanged(QString font);
    void systemFixedFontChanged(QString fixedFont);
    void systemFontPointSizeChanged(qreal systemFontPointSize);
    void iconThemeChanged();
    void darkModeChanged(bool darkMode);

private:
    void onFileChanged(const QString &path);

private:
    QHash<QPlatformTheme::ThemeHint, QVariant> m_hints;
    QSettings *m_settings;
    QString m_settingsFile;
    QFileSystemWatcher *m_fileWatcher;

    QString m_systemFont;
    QString m_systemFixedFont;
    qreal m_systemFontPointSize;
    QString m_iconTheme;
};

#endif //HINTSSETTINGS_H
