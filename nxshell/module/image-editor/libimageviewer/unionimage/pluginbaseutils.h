// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINBASEUTILS_H
#define PLUGINBASEUTILS_H

#include <QObject>
#include <QTimer>
#include <QColor>
#include <QMimeData>
#include <QFileInfoList>

namespace pluginUtils {

namespace base {
//void        copyOneImageToClipboard(const QString &path);
//void        copyImageToClipboard(const QStringList &paths);
//void        showInFileManager(const QString &path);
//int         stringHeight(const QFont &f, const QString &str);
//QString     hash(const QString &str);
//QString     SpliteText(const QString &text, const QFont &font, int nLabelSize);
//QDateTime   stringToDateTime(const QString &time);
//QString     getFileContent(const QString &file);
//QPixmap     renderSVG(const QString &filePath, const QSize &size);
bool checkMimeData(const QMimeData *mimeData);
QString mkMutiDir(const QString &path);

const QFileInfoList getImagesInfo(const QString &dir, bool recursive = true);
bool imageSupportRead(const QString &path);
//bool                                imageSupportSave(const QString &path);
QStringList supportedImageFormats();
}  // namespace base

}  // namespace utils

#endif // BASEUTILS_H
