// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <fstab.h>
#include <sys/stat.h>

#include <QMutex>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

#include "utils.h"

DSEARCH_BEGIN_NAMESPACE

namespace Utils {

QMap<QString, QString> &fstabBindInfo()
{
    static QMutex mutex;
    static QMap<QString, QString> table;
    struct stat statInfo;
    int result = stat("/etc/fstab", &statInfo);

    QMutexLocker locker(&mutex);
    if (0 == result) {
        static quint32 lastModify = 0;
        if (lastModify != statInfo.st_mtime) {
            lastModify = static_cast<quint32>(statInfo.st_mtime);
            table.clear();
            struct fstab *fs;

            setfsent();
            while ((fs = getfsent()) != nullptr) {
                QString mntops(fs->fs_mntops);
                if (mntops.contains("bind"))
                    table.insert(fs->fs_spec, fs->fs_file);
            }
            endfsent();
        }
    }

    return table;
}

QString convertTo(const QString &path, Utils::ConvertType type)
{
    if (!path.startsWith("/") || path == "/")
        return path;

    const auto &table = fstabBindInfo();
    if (table.isEmpty())
        return path;

    QString bindPath(path);
    switch (type) {
    case Utils::DeviceType:
        for (const auto &mntPoint : table.values()) {
            if (path.startsWith(mntPoint)) {
                bindPath.replace(mntPoint, table.key(mntPoint));
                break;
            }
        }
        break;
    case Utils::DirType:
        for (const auto &device : table.keys()) {
            if (path.startsWith(device)) {
                bindPath.replace(device, table[device]);
                break;
            }
        }
        break;
    }

    return bindPath;
}

QString checkWildcardAndToRegularExpression(const QString &pattern)
{
    if (!pattern.contains('*') && !pattern.contains('?'))
        return wildcardToRegularExpression('*' + pattern + '*');

    return wildcardToRegularExpression(pattern);
}

QString wildcardToRegularExpression(const QString &pattern)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    return QRegularExpression::wildcardToRegularExpression(pattern);
#endif   // (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))

    const int wclen = pattern.length();
    QString rx;
    rx.reserve(wclen + wclen / 16);
    int i = 0;
    const QChar *wc = pattern.unicode();
#ifdef Q_OS_WIN
    const QLatin1Char nativePathSeparator('\\');
    const QLatin1String starEscape("[^/\\\\]*");
    const QLatin1String questionMarkEscape("[^/\\\\]");
#else
    const QLatin1Char nativePathSeparator('/');
    const QLatin1String starEscape("[^/]*");
    const QLatin1String questionMarkEscape("[^/]");
#endif
    while (i < wclen) {
        const QChar c = wc[i++];
        switch (c.unicode()) {
        case '*':
            rx += starEscape;
            break;
        case '?':
            rx += questionMarkEscape;
            break;
        case '\\':
#ifdef Q_OS_WIN
        case '/':
            rx += QLatin1String("[/\\\\]");
            break;
#endif
        case '$':
        case '(':
        case ')':
        case '+':
        case '.':
        case '^':
        case '{':
        case '|':
        case '}':
            rx += QLatin1Char('\\');
            rx += c;
            break;
        case '[':
            rx += c;
            // Support for the [!abc] or [!a-c] syntax
            if (i < wclen) {
                if (wc[i] == QLatin1Char('!')) {
                    rx += QLatin1Char('^');
                    ++i;
                }
                if (i < wclen && wc[i] == QLatin1Char(']'))
                    rx += wc[i++];
                while (i < wclen && wc[i] != QLatin1Char(']')) {
                    // The '/' appearing in a character class invalidates the
                    // regular expression parsing. It also concerns '\\' on
                    // Windows OS types.
                    if (wc[i] == QLatin1Char('/') || wc[i] == nativePathSeparator)
                        return rx;
                    if (wc[i] == QLatin1Char('\\'))
                        rx += QLatin1Char('\\');
                    rx += wc[i++];
                }
            }
            break;
        default:
            rx += c;
            break;
        }
    }
    return anchoredPattern(rx);
}

QString Chinese2Pinyin(const QString &words)
{
    const auto &dict = dictData();
    QString result;

    for (int i = 0; i < words.length(); ++i) {
        const uint key = words.at(i).unicode();
        auto find_result = dict.find(key);

        if (find_result != dict.end()) {
            result.append(find_result.value());
        } else {
            result.append(words.at(i));
        }
    }

    return result;
}

QHash<uint, QString> &dictData()
{
    static QHash<uint, QString> dict;
    if (dict.isEmpty()) {
        dict.reserve(25333);
        QFile file(":/resources/pinyin.dict");

        if (!file.open(QIODevice::ReadOnly))
            return dict;

        QByteArray content = file.readAll();
        file.close();

        QTextStream stream(&content, QIODevice::ReadOnly);
        while (!stream.atEnd()) {
            const QString line = stream.readLine();
            const QStringList items = line.split(QChar(':'));

            if (items.size() == 2)
                dict.insert(static_cast<uint>(items[0].toInt(nullptr, 16)), items[1]);
        }
    }

    return dict;
}

}

DSEARCH_END_NAMESPACE
