/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef FILEINDEXERCONFIG_H
#define FILEINDEXERCONFIG_H

#include <QObject>
#include <QSettings>
#include <QGSettings>
#include <QMutex>
#include "dir-watcher.h"
namespace LingmoUISearch{

class FileIndexerConfig : public QObject
{
    Q_OBJECT
public:
    static FileIndexerConfig* getInstance();
    /**
     * @brief currentIndexableDir
     * @return 当前可索引的所有目录
     */
    QStringList currentIndexableDir();
    /**
     * @brief currentBlackListOfIndex
     * @return 当前索引黑名单
     */
    QStringList currentBlackListOfIndex();
    /**
     * @brief isFileIndexEnable
     * @return 是否启动基本索引（只索引文件基本数据，包含路径等内容）
     */
    bool isFileIndexEnable();
    /**
     * @brief isContentIndexEnable
     * @return 是否启动文本内容索引
     */
    bool isContentIndexEnable();
    /**
     * @brief isFuzzySearchEnable
     * @return 是否启动模糊搜索
     */
    bool isFuzzySearchEnable();
    /**
     * @brief isOCREnable
     * @return 是否激活OCR功能（文件内容索引）
     */
    bool isOCREnable();
    /**
     * @brief isMetaDataIndexEnable
     * @return 是否激活元数据索引
     */
    bool isMetaDataIndexEnable();
    /**
     * 是否激活AI索引
     * @return
     */
    bool isAiIndexEnable();
    QMap<QString, bool> contentIndexTarget();
    QMap<QString, bool> ocrContentIndexTarget();
    QMap<QString, bool> aiIndexFileTarget();
    QMap<QString, bool> aiIndexImageTarget();
    void syncIndexFileTypes(bool useDefaultSettings = false);

Q_SIGNALS:
    /**
     * @brief appendIndexDir
     * 索引目录增加时发送，参数为增加的目录和其下的黑名单
     */
    void appendIndexDir(const QString&, const QStringList&);
    /**
     * @brief removeIndexDir
     * 移除索引目录时发送
     */
    void removeIndexDir(const QString&);
    /**
     * @brief fileIndexEnableStatusChanged
     * 文件索引开关（基本索引）
     */
    void fileIndexEnableStatusChanged(bool);
    /**
     * @brief fileIndexEnableStatusChanged
     * 内容索引
     */
    void contentIndexEnableStatusChanged(bool);
    /**
     * @brief contentIndexEnableOcrStatusChanged
     * ocr
     */
    void contentIndexEnableOcrStatusChanged(bool);
    /**
     * AI索引
     */
    void aiIndexEnableStatusChanged(bool);

private:
    explicit FileIndexerConfig(QObject *parent = nullptr);
    ~FileIndexerConfig();
    QMap<QString, bool> m_targetFileTypeMap = {
            {"doc", true},
            {"docx", true},
            {"ppt", true},
            {"pptx", true},
            {"xls", true},
            {"xlsx", true},
            {"txt", true},
            {"dot", true},
            {"wps", true},
            {"pps", true},
            {"dps", true},
            {"et", true},
            {"pdf", true},
            {"html", true},
            {"uof", true},
            {"uot", true},
            {"uos", true},
            {"uop", true},
            {"ofd", true}
    };

    QMap<QString, bool> m_targetPhotographTypeMap = {
            {"png", true},
            {"bmp", true},
            {"hdr", false},
            {"gif", true},
            {"tif", true},
            {"tiff", true},
            {"heif", false},
            {"webp", true},
            {"jpe", true},
            {"dib", false},
            {"psd", false},
            {"jng", false},
            {"xpm", false},//pix read error.
            {"j2k", false},
            {"jp2", false},
            {"jpg", true},
            {"jpeg", true}
    };

    QMap<QString, bool> m_sAiFileTypeMap = {
            {"docx", true},
            {"pptx", true},
            {"txt", true},
            {"pdf", true},
    };
    QMap<QString, bool> m_sAiImageTypeMap = {
            {"png", true},
            {"jpg", true},
            {"jpeg", true}
    };

    DirWatcher *m_dirWatcher = nullptr;
    QGSettings *m_gsettings = nullptr;
    QSettings *m_settings = nullptr;
    QMutex m_mutex;
};
}
#endif // FILEINDEXERCONFIG_H
