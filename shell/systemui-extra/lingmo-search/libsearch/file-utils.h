/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangpengfei <zhangpengfei@kylinos.cn>
 * Modified by: zhangzihao <zhangzihao@kylinos.cn>
 * Modified by: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <QString>
#include <QIcon>
#include <QMap>
#include <QMimeType>
#include <QLabel>
#include "libsearch_global.h"

namespace LingmoUISearch {
class  LIBSEARCH_EXPORT FileUtils {
public:
    static QString getHtmlText(const QString &text, const QString &keyword);
    static QString setAllTextBold(const QString &name);
    static QString wrapData(QLabel *p_label, const QString &text);
    static qreal horizontalAdvanceContainsKeyword(const QString &content, const QString &keyword);
    static std::string makeDocUterm(const QString& path);
    static QIcon getFileIcon(const QString &uri, bool checkValid = true);
    static QIcon getSettingIcon();

    //A is or under B
    static bool isOrUnder(const QString& pathA, const QString& pathB);
    static QStringList findMultiToneWords(const QString &hanzi);

    //parse text,docx.....
    static QMimeType getMimetype(const QString &path);

    static int openFile(QString &path, bool openInDir = false);
    static bool copyPath(QString &path);
    static QString escapeHtml(const QString &str);
    static QString getSnippet(const std::string &myStr, uint start, const QString &keyword);
    static QString getSnippetWithoutKeyword(const QString &content, int lineCount);
    static bool isOpenXMLFileEncrypted(const QString &path);
    /**
     * @brief isEncrypedOrUnsupport
     * @param path
     * @param suffix
     * @return true if file(path) is not a support format for content index.
     */
    static bool isEncrypedOrUnsupport(const QString &path, const QString &suffix);
    static bool isOcrSupportSize(QString path);

private:
    FileUtils();
};

}

#endif // FILEUTILS_H
