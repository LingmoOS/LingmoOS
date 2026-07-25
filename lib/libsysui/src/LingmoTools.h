#ifndef LINGMOTOOLS_H
#define LINGMOTOOLS_H

#include <cstdint>

#include <QIcon>
#include <QImage>
#include <QObject>
#include <QQmlEngine>
#include <QQuickWindow>

#include "singleton.h"

typedef std::uint64_t hash_t;

constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;

// Compute the hash of a string.
hash_t hash_(char const* str);

constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)
{
    return *str ? hash_compile_time(str + 1, (*str ^ last_value) * prime)
                : last_value;
}

/**
 * @brief The LingmoTools class. Contains some small utils.
 * @since LingmoUI 3.0
 */
class LingmoTools : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(LingmoTools)
    QML_SINGLETON

private:
    explicit LingmoTools(QObject* parent = nullptr);

public:
    SINGLETON(LingmoTools)

    static auto create(QQmlEngine*, QJSEngine*) { return getInstance(); }

    Q_INVOKABLE int qtMajor();

    Q_INVOKABLE int qtMinor();

    Q_INVOKABLE bool isMacos();

    Q_INVOKABLE bool isLinux();

    Q_INVOKABLE bool isWin();

    Q_INVOKABLE void clipText(const QString& text);

    Q_INVOKABLE QString uuid();

    Q_INVOKABLE QString readFile(const QString& fileName);

    Q_INVOKABLE void setQuitOnLastWindowClosed(bool val);

    Q_INVOKABLE void setOverrideCursor(Qt::CursorShape shape);

    Q_INVOKABLE void restoreOverrideCursor();

    Q_INVOKABLE QString html2PlantText(const QString& html);

    Q_INVOKABLE QString toLocalPath(const QUrl& url);

    Q_INVOKABLE void deleteLater(QObject* p);

    Q_INVOKABLE QString getFileNameByUrl(const QUrl& url);

    Q_INVOKABLE QRect getVirtualGeometry();

    Q_INVOKABLE QString getApplicationDirPath();

    Q_INVOKABLE QUrl getUrlByFilePath(const QString& path);

    Q_INVOKABLE QColor withOpacity(const QColor&, qreal alpha);

    Q_INVOKABLE QString md5(const QString& text);

    Q_INVOKABLE QString sha256(const QString& text);

    Q_INVOKABLE QString toBase64(const QString& text);

    Q_INVOKABLE QString fromBase64(const QString& text);

    Q_INVOKABLE bool removeDir(const QString& dirPath);

    Q_INVOKABLE bool removeFile(const QString& filePath);

    Q_INVOKABLE void showFileInFolder(const QString& path);

    Q_INVOKABLE bool isSoftware(); // Checkfor software rendering

    Q_INVOKABLE qint64 currentTimestamp();

    Q_INVOKABLE QPoint cursorPos();

    Q_INVOKABLE QIcon windowIcon();

    Q_INVOKABLE int cursorScreenIndex();

    Q_INVOKABLE int windowBuildNumber();

    Q_INVOKABLE bool isWindows11OrGreater();

    Q_INVOKABLE bool isWindows10OrGreater();

    Q_INVOKABLE QRect desktopAvailableGeometry(QQuickWindow* window);

    Q_INVOKABLE QString getWallpaperFilePath();

    Q_INVOKABLE QColor imageMainColor(const QImage& image, double bright = 1);
};

#endif // LINGMOTOOLS_H
