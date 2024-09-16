// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONTOOLS_H
#define COMMONTOOLS_H

#include <QStringList>
#include <QPoint>
#include <QColor>
#include <QtDebug>
#include <QImage>
#include <QPainter>
#include <QMutex>

#include <fstab.h>
#include <sys/stat.h>

namespace GrandSearch {

namespace CommonTools {

// 规范化浮点型字串，去掉小数点后多余的0
inline QString normalizeDoubleString(const QString &str)
{
    int begin_pos = str.indexOf('.');

    if (begin_pos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > begin_pos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

// 根据文件size换算为KB\MB\GB\TB
inline QString formatFileSize(qint64 num, bool withUnitVisible = true, int precision = 1, int forceUnit = -1,
                              QStringList unitList = QStringList({ " B", " KB", " MB", " GB", " TB" }))
{
    if (num < 0) {
        qWarning() << "Negative number passed to formatSize():" << num;
        num = 0;

        return QString::number(num);
    }

    bool isForceUnit = (forceUnit >= 0);

    qreal fileSize(num);
    QStringListIterator i(unitList);
    QString unit = i.hasNext() ? i.next() : QStringLiteral(" B");

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024 && !isForceUnit) {
            break;
        }

        if (isForceUnit && index == forceUnit) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    QString unitString = withUnitVisible ? unit : QString();
    return QString("%1%2").arg(normalizeDoubleString(QString::number(fileSize, 'f', precision)), unitString);
}

// 日期格式
inline QString dateTimeFormat()
{
    return "yyyy/MM/dd HH:mm";
}

inline QString durationString(qint64 seconds)
{
    int hour = static_cast<int>(seconds / 3600);

    QString mmStr = QString("%1").arg(seconds % 3600 / 60, 2, 10, QLatin1Char('0'));
    QString ssStr = QString("%1").arg(seconds % 60, 2, 10, QLatin1Char('0'));

    if (hour > 0) {
        return QString("%1:%2:%3").arg(hour).arg(mmStr).arg(ssStr);
    } else {
        return QString("%1:%2").arg(mmStr).arg(ssStr);
    }
}

inline QImage creatErrorImage(const QSize &imgSize, const QImage &errorImg)
{
    QImage img(imgSize, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(0, 0, img);

    // errorImg居中
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage((img.width() - errorImg.width()) / 2,
                      (img.height() - errorImg.height()) / 2, errorImg);
    painter.end();

    return img;
}

// 计算换行内容
inline QString lineFeed(const QString &text, int nWidth, const QFont &font, int nElidedRow = 2)
{
    if (nElidedRow < 0)
        nElidedRow = 2;

    QString strText = text;
    QStringList strListLine;
    QFontMetrics fm(font);
    // 一行就直接中间截断显示
    if (1 == nElidedRow)
        return fm.elidedText(text, Qt::ElideMiddle, nWidth);

    if (!strText.isEmpty()) {
        for (int i = 1; i <= strText.size(); i++) {
            if (fm.width(strText.left(i)) >= nWidth) {
                if (strListLine.size() + 1 == nElidedRow)
                    break;

                strListLine.append(strText.left(i - 1));
                strText = strText.right(strText.size() - i + 1);
                i = 0;
            }
        }
    }

    // 多行时，对最后一行字符左侧省略
    if (!strListLine.isEmpty()) {
        strText = fm.elidedText(strText, Qt::ElideLeft, nWidth);
        strListLine.append(strText);
        strText = strListLine.join('\n');
    }

    return strText;
}

// 获取bind信息
inline QMap<QString, QString> fstabBindInfo()
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

// bind目录相互转换
// toDevice为true转换为设备名，否则转换为挂载点名
inline QString bindPathTransform(const QString &path, bool toDevice)
{
    if (!path.startsWith("/") || path == "/")
        return path;

    const QMap<QString, QString> &table = fstabBindInfo();
    if (table.isEmpty())
        return path;

    QString bindPath(path);
    if (toDevice) {
        for (const auto &mntPoint : table.values()) {
            if (path.startsWith(mntPoint)) {
                bindPath.replace(mntPoint, table.key(mntPoint));
                break;
            }
        }
    } else {
        for (const auto &device : table.keys()) {
            if (path.startsWith(device)) {
                bindPath.replace(device, table[device]);
                break;
            }
        }
    }

    return bindPath;
}

}   // end CommonTools

}

#endif   // COMMONTOOLS_H
