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
#include "writable-database.h"
#include <QDir>
#include <QDebug>
#include <memory>
#include "file-utils.h"
#include "index-status-recorder.h"
#include "common.h"

using namespace LingmoUISearch;

static QMutex g_basicDatabaseMutex;
static QMutex g_contentDatabaseMutex;
static QMutex g_ocrContentDatabaseMutex;

#define DATABASE_TRY(code) try { \
code; \
} \
catch (const Xapian::Error &e)\
{\
    qWarning() << "Transaction error," << QString::fromStdString(e.get_description()); \
    errorRecord(); \
}

WritableDatabase::WritableDatabase(const DataBaseType &type)
    : m_type(type)
{
    switch (type) {
        case DataBaseType::Basic:
            m_path = INDEX_PATH;
            m_mutex = &g_basicDatabaseMutex;
            break;
        case DataBaseType::Content:
            m_path = CONTENT_INDEX_PATH;
            m_mutex = &g_contentDatabaseMutex;
            break;
        case DataBaseType::OcrContent:
            m_path = OCR_CONTENT_INDEX_PATH;
            m_mutex = &g_ocrContentDatabaseMutex;
        default:
            break;
    }
    m_mutex->lock();
}

WritableDatabase::~WritableDatabase()
{
    if(m_xpDatabase) {
        delete m_xpDatabase;
        m_xpDatabase = nullptr;
    }
    m_mutex->unlock();
}

bool WritableDatabase::open()
{
    if(m_xpDatabase) {
        return true;
    }
    QDir database(m_path);
    if(!database.exists()) {
        qDebug() << "Create writable database" << m_path<< database.mkpath(m_path);
    }
    try {
        m_xpDatabase = new Xapian::WritableDatabase(m_path.toStdString(), Xapian::DB_CREATE_OR_OPEN);
    } catch(const Xapian::Error &e) {
        qWarning() << "Open WritableDatabase fail!" << m_path << QString::fromStdString(e.get_description());
        return false;
    }
    return true;
}

void WritableDatabase::rebuild()
{
    if(m_xpDatabase) {
        delete m_xpDatabase;
        m_xpDatabase = nullptr;
    }

    QDir database(m_path);
    if(database.exists()) {
        qDebug() << "Dababase rebuild, remove" << m_path << database.removeRecursively();
    } else {
        qDebug() << "Dababase rebuild, create" << m_path << database.mkpath(m_path);
    }

    //更新版本号
    switch (m_type) {
        case DataBaseType::Basic:
            IndexStatusRecorder::getInstance()->setVersion(INDEX_DATABASE_VERSION_KEY, INDEX_DATABASE_VERSION);
            break;
        case DataBaseType::Content:
            IndexStatusRecorder::getInstance()->setVersion(CONTENT_DATABASE_VERSION_KEY, CONTENT_DATABASE_VERSION);
            break;
        case DataBaseType::OcrContent:
            IndexStatusRecorder::getInstance()->setVersion(OCR_CONTENT_DATABASE_VERSION_KEY, OCR_CONTENT_DATABASE_VERSION);
        default:
            break;
    }
}

void WritableDatabase::beginTransation()
{
    DATABASE_TRY(m_xpDatabase->begin_transaction();)
}

void WritableDatabase::endTransation()
{
    DATABASE_TRY(m_xpDatabase->commit_transaction();)
}

void WritableDatabase::cancelTransation()
{
    DATABASE_TRY(m_xpDatabase->cancel_transaction();)
}

void WritableDatabase::commit()
{
    DATABASE_TRY(m_xpDatabase->commit();)
}

void WritableDatabase::addDocument(const Document &doc)
{
    DATABASE_TRY(m_xpDatabase->replace_document(doc.getUniqueTerm(), doc.getXapianDocument());\
            m_xpDatabase->set_metadata(doc.getUniqueTerm(), doc.indexTime());)
}

void WritableDatabase::removeDocument(const QString &path)
{
    DATABASE_TRY(m_xpDatabase->delete_document(FileUtils::makeDocUterm(path));)
}

void WritableDatabase::removeDocument(const std::string uniqueTerm)
{
    DATABASE_TRY(m_xpDatabase->delete_document(uniqueTerm);
            m_xpDatabase->set_metadata(uniqueTerm, "");)
}

void WritableDatabase::removeChildrenDocument(const QString &path)
{
    DATABASE_TRY(m_xpDatabase->delete_document("PARENTTERM" + FileUtils::makeDocUterm(path));)
}

void WritableDatabase::setMetaData(const QString &key, const QString &value)
{
    DATABASE_TRY(m_xpDatabase->set_metadata(key.toStdString(), value.toStdString());)
}

void WritableDatabase::setMetaData(const std::string &key, const std::string &value)
{
    DATABASE_TRY(m_xpDatabase->set_metadata(key, value);)
}

const std::string WritableDatabase::getMetaData(const std::string &key)
{
    std::string value;
    DATABASE_TRY(value = m_xpDatabase->get_metadata(key);)
    return value;
}

QMap<std::string, std::string> WritableDatabase::getIndexTimes()
{
    QMap<std::string, std::string> indexTimes;
    DATABASE_TRY(
    for(Xapian::TermIterator iter = m_xpDatabase->metadata_keys_begin();
        iter != m_xpDatabase->metadata_keys_end(); ++iter) {
        indexTimes.insert(*iter, m_xpDatabase->get_metadata(*iter));
    })
    return indexTimes;
}

void WritableDatabase::errorRecord()
{
    switch (m_type) {
        case DataBaseType::Basic:
            IndexStatusRecorder::getInstance()->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Error);
            break;
        case DataBaseType::Content:
            IndexStatusRecorder::getInstance()->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Error);
            break;
        case DataBaseType::OcrContent:
            IndexStatusRecorder::getInstance()->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Error);
            break;
        default:
            break;
    }
}

