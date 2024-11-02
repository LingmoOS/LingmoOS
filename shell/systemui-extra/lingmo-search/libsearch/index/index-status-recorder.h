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
#ifndef INDEXSTATUSRECORDER_H
#define INDEXSTATUSRECORDER_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QMutex>
#define CONTENT_INDEX_DATABASE_STATE_KEY "content_index_database_state"
#define OCR_CONTENT_INDEX_DATABASE_STATE_KEY "ocr_content_index_database_state"
#define INDEX_DATABASE_STATE_KEY "index_database_state"
#define AI_INDEX_DATABASE_STATE_KEY "ai_index_database_state"
#define INDEX_STATUS QDir::homePath() + "/.config/org.lingmo/lingmo-search/lingmo-search-index-status.conf"
#define INDEX_DATABASE_VERSION_KEY "index_database_version"
#define CONTENT_DATABASE_VERSION_KEY "content_database_version"
#define OCR_CONTENT_DATABASE_VERSION_KEY "ocr_content_database_version"
namespace LingmoUISearch {
//fixme: we need a better way to record index status.
class IndexStatusRecorder : public QObject
{
    Q_OBJECT
public:
    enum State{
        Initializing = 0,
        Error = 1,
        Ready = 2,
        Updating = 3,
        Off = 4,
        RealTimeUpdating = 5
    };
    Q_ENUM(State)

    static IndexStatusRecorder *getInstance();
    void setStatus(const QString& key, LingmoUISearch::IndexStatusRecorder::State state);
    const QVariant getStatus(const QString& key);
    void setVersion(const QString& key, const QString& version);

    bool versionCheck(const QString& key, const QString& version);
    bool indexDatabaseEnable();
    bool contentIndexDatabaseEnable();

private:
    explicit IndexStatusRecorder(QObject *parent = nullptr);
    static IndexStatusRecorder *m_instance;
    QSettings *m_status = nullptr;
    QMutex m_mutex;
};
}

#endif // INDEXSTATUSRECORDER_H
