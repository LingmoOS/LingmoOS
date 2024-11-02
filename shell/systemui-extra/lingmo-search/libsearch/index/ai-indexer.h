/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#ifndef LINGMO_SEARCH_AI_INDEXER_H
#define LINGMO_SEARCH_AI_INDEXER_H
#include <QObject>
#include <QJsonObject>
namespace LingmoUISearch {
class AiIndexerPrivate;
class AiIndexer : public QObject
{
    Q_OBJECT
public:
    explicit AiIndexer(QObject *parent = nullptr);
    ~AiIndexer();
    bool createSession();
    bool reCreateSession();
    bool checkFileSupported(const QString &filePath, QString &result);
    bool addTextFileIndex(const QJsonArray &object);
    bool addImageFileIndex(const QJsonArray &object);
    bool deleteFileIndex(const QStringList &files);
    bool getAllIndexedFiles(QJsonObject &object);
    bool update(const QJsonArray &object);

private:
    bool destroySession();
    AiIndexerPrivate *d = nullptr;

};

} // LingmoUISearch

#endif //LINGMO_SEARCH_AI_INDEXER_H
