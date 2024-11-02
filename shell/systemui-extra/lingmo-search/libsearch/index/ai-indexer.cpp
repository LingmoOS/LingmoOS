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

#include "ai-indexer.h"
#include <unistd.h>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QMutex>
#include <lingmo-ai/ai-base/datamanagement.h>

namespace LingmoUISearch {
class AiIndexerPrivate
{
public:
    static QMutex m_mutex;
    static bool m_sessionValid;
    static DataManagementSession m_session;
};
DataManagementSession AiIndexerPrivate::m_session = nullptr;
bool AiIndexerPrivate::m_sessionValid = false;
QMutex AiIndexerPrivate::m_mutex;
AiIndexer::AiIndexer(QObject *parent) : QObject(parent), d(new AiIndexerPrivate)
{
    AiIndexerPrivate::m_mutex.lock();
}

AiIndexer::~AiIndexer()
{
    AiIndexerPrivate::m_mutex.unlock();
    if(d) {
        delete d;
        d = nullptr;
    }
}

bool AiIndexer::createSession()
{
    if(!AiIndexerPrivate::m_sessionValid) {
        DataManagementResult result = data_management_create_session(&AiIndexerPrivate::m_session, DataManagementType::SYS_SEARCH, getuid());
        qDebug() << "===creatSession===" <<result;
        if(result == DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
            AiIndexerPrivate::m_sessionValid = true;
            return true;
        }
        return false;
    } else {
        return true;
    }
}

bool AiIndexer::reCreateSession()
{
    destroySession();
    return createSession();
}

bool AiIndexer::destroySession()
{
    AiIndexerPrivate::m_sessionValid = false;
    return data_management_destroy_session(AiIndexerPrivate::m_session) == DataManagementResult::DATA_MANAGEMENT_SUCCESS;
}

bool AiIndexer::checkFileSupported(const QString &filePath, QString &result)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    char *originalResult;
    DataManagementResult re = data_management_check_file_format(AiIndexerPrivate::m_session, filePath.toLocal8Bit().data(), &originalResult);
    if(re == DATA_MANAGEMENT_SESSION_ERROR) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            data_management_free_check_file_format_result(AiIndexerPrivate::m_session, originalResult);
            return false;
        }
        qDebug() << "Creating session success.";
        re = data_management_check_file_format(AiIndexerPrivate::m_session, filePath.toLocal8Bit().data(), &originalResult);
    }
    if(re == DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
        QJsonObject jo = QJsonDocument::fromJson(QByteArray(originalResult)).object();
        data_management_free_check_file_format_result(AiIndexerPrivate::m_session, originalResult);
        if(jo.value(QStringLiteral("supported")).toBool()) {
            result = jo.value(QStringLiteral("format")).toString();
            return true;
        }
    };
    return false;
}

bool AiIndexer::addTextFileIndex(const QJsonArray &object)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    DataManagementResult result = data_management_add_text_files(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    if(result == DATA_MANAGEMENT_SESSION_ERROR) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            return false;
        }
        qDebug() << "Creating session success.";
        result = data_management_add_text_files(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    }
    if(result != DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
        qWarning() << "Fail to call data_management_add_text_files, input: " << QJsonDocument(object).toJson().data()
                << "result:" << result;
        return false;
    }
    return true;
}

bool AiIndexer::addImageFileIndex(const QJsonArray &object)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    DataManagementResult result = data_management_add_image_files(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    if(result == DATA_MANAGEMENT_SESSION_ERROR) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            return false;
        }
        qDebug() << "Creating session success.";
        result = data_management_add_image_files(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    }
    if(result != DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
        qWarning() << "Fail to call data_management_add_image_files, input: " << QJsonDocument(object).toJson().data()
                   << "result:" << result;
        return false;
    }
    return true;
}

bool AiIndexer::deleteFileIndex(const QStringList &files)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        qWarning() << "deleteFileIndex: Session is not valid";
        return false;
    }
    QJsonArray tmpArray;

    for(const QString &file : files) {
        QJsonObject oneFile;
        oneFile.insert(QStringLiteral("filepath"), file);
        tmpArray.append(oneFile);
    }
    DataManagementResult result = data_management_delete_files(AiIndexerPrivate::m_session, QJsonDocument(tmpArray).toJson().data());
    if(result == DATA_MANAGEMENT_SESSION_ERROR) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            return false;
        }
        qDebug() << "Creating session success.";
        result = data_management_delete_files(AiIndexerPrivate::m_session, QJsonDocument(tmpArray).toJson().data());
    }
    if(result != DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
        qWarning() << "Fail to call data_management_delete_files, input: " << QJsonDocument(tmpArray).toJson().data()
                   << "result:" << result;
        return false;
    }
    return true;
}

bool AiIndexer::getAllIndexedFiles(QJsonObject &object)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    char *originalResult;
    DataManagementResult result = data_management_get_all_fileinfos(AiIndexerPrivate::m_session, &originalResult);
    if(result == DATA_MANAGEMENT_SESSION_ERROR) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            data_management_free_get_all_fileinfos_result(AiIndexerPrivate::m_session, originalResult);
            return false;
        }
        qDebug() << "Creating session success.";
        result = data_management_get_all_fileinfos(AiIndexerPrivate::m_session, &originalResult);
    }
    if(result == DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
        QJsonArray array = QJsonDocument::fromJson(QByteArray(originalResult)).array();
        data_management_free_get_all_fileinfos_result(AiIndexerPrivate::m_session, originalResult);
        for(const QJsonValueRef value : array) {
            object.insert(value.toObject().value(QStringLiteral("filepath")).toString(), value.toObject().value(QStringLiteral("timestamp")).toInt());
        }
        return true;
    }
    return false;
}

bool AiIndexer::update(const QJsonArray &object)
{
    if(!AiIndexerPrivate::m_sessionValid) {
        return false;
    }
    DataManagementResult result = data_management_update_files_content(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    if(result == DATA_MANAGEMENT_SESSION_ERROR) {
        qWarning() << "AiIndexer:DATA_MANAGEMENT_SESSION_ERROR occured, re creating session";
        if(!reCreateSession()) {
            qWarning() << "AiIndexer:Failed to create session";
            return false;
        }
        qDebug() << "Creating session success.";
        result = data_management_update_files_content(AiIndexerPrivate::m_session, QJsonDocument(object).toJson().data());
    }
    if(result != DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
        qWarning() << "Fail to call data_management_update_files_content, input: " << QJsonDocument(object).toJson().data()
                   << "result:" << result;
        return false;
    }
    return true;
}

} // LingmoUISearch