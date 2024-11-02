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
#include "file-indexer-config.h"

#include <mutex>
#include <QDebug>
#include <QDir>
#include <QMutexLocker>

static const QString INDEX_SETTINGS = QDir::homePath() + "/.config/org.lingmo/lingmo-search/lingmo-search-service.conf";
static const QString CONTENT_INDEX_TARGET_TYPE = "contentIndexTarget";
static const QString OCR_CONTENT_INDEX_TARGET_TYPE = "ocrContentIndexTarget";
static const QString S_AI_FILE_TYPE = "sAiFileType";
static const QString S_AI_IMAGE_TYPE = "sAiImageType";
/**
 * changelog: 1.2 增加AI索引开关
 * changelog: 1.1 增加ocr开关
 */
static const QString GSETTINGS_VERSION = QStringLiteral("1.2");
/**
 * changelog: 1.0 init
 */
static const QString SETTINGS_VERSION = QStringLiteral("1.2");
static const QByteArray LINGMO_SEARCH_SCHEMAS = QByteArrayLiteral("org.lingmo.search.settings");
static const QString FILE_INDEX_ENABLE_KEY = QStringLiteral("fileIndexEnable");
static const QString CONTENT_INDEX_ENABLE_KEY = QStringLiteral("contentIndexEnable");
static const QString CONTENT_FUZZY_SEARCH_KEY = QStringLiteral("contentFuzzySearch");
static const QString CONTENT_INDEX_ENABLE_OCR_KEY = QStringLiteral("contentIndexEnableOcr");
static const QString META_DATA_INDEX_ENABLE_KEY = QStringLiteral("metaDataIndexEnable");
static const QString AI_INDEX_ENABLE_KEY = QStringLiteral("aiIndexEnable");
static const QString GSETTINGS_VERSION_KEY = QStringLiteral("version");
static const QString SETTINGS_VERSION_KEY = QStringLiteral("version");


static std::once_flag flag;
static LingmoUISearch::FileIndexerConfig *globalInstance = nullptr;
using namespace LingmoUISearch;
FileIndexerConfig *FileIndexerConfig::getInstance()
{
    std::call_once(flag, [ & ] {
        globalInstance = new FileIndexerConfig();
    });
    return globalInstance;
}

FileIndexerConfig::FileIndexerConfig(QObject *parent)
    : QObject(parent),
      m_dirWatcher(DirWatcher::getDirWatcher())
{
    connect(m_dirWatcher, &DirWatcher::appendIndexItem, this, &FileIndexerConfig::appendIndexDir);
    connect(m_dirWatcher, &DirWatcher::removeIndexItem, this, &FileIndexerConfig::removeIndexDir);

    const QByteArray id(LINGMO_SEARCH_SCHEMAS);
    if(QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id, QByteArray(), this);
        //保留旧版本配置
        if(m_gsettings->keys().contains(GSETTINGS_VERSION_KEY)) {
            QString oldVersion = m_gsettings->get(GSETTINGS_VERSION_KEY).toString();
            if(oldVersion == "0.0") {
                bool fileIndexEnable = false;
                if(m_gsettings->keys().contains(FILE_INDEX_ENABLE_KEY)) {
                    fileIndexEnable = m_gsettings->get(FILE_INDEX_ENABLE_KEY).toBool();
                }
                if(fileIndexEnable) {
                    if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_KEY)) {
                        m_gsettings->set(CONTENT_INDEX_ENABLE_KEY, true);
                    }
                    if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_OCR_KEY)) {
                        m_gsettings->set(CONTENT_INDEX_ENABLE_OCR_KEY, true);
                    }
                }
                m_gsettings->set(GSETTINGS_VERSION_KEY, GSETTINGS_VERSION);
            } else if (oldVersion == "1.0") {
                bool contentIndex = false;
                if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_KEY)) {
                    contentIndex = m_gsettings->get(CONTENT_INDEX_ENABLE_KEY).toBool();
                }
                if(contentIndex) {
                    if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_OCR_KEY)) {
                        m_gsettings->set(CONTENT_INDEX_ENABLE_OCR_KEY, true);
                    }
                }
                m_gsettings->set(GSETTINGS_VERSION_KEY, GSETTINGS_VERSION);
            }
        }
        connect(m_gsettings, &QGSettings::changed, this, [ = ](const QString &key) {
            if(key == FILE_INDEX_ENABLE_KEY) {
                Q_EMIT fileIndexEnableStatusChanged(m_gsettings->get(FILE_INDEX_ENABLE_KEY).toBool());
            } else if (key == CONTENT_INDEX_ENABLE_KEY) {
                Q_EMIT contentIndexEnableStatusChanged(m_gsettings->get(CONTENT_INDEX_ENABLE_KEY).toBool());
            } else if (key == CONTENT_INDEX_ENABLE_OCR_KEY) {
                Q_EMIT contentIndexEnableOcrStatusChanged(m_gsettings->get(CONTENT_INDEX_ENABLE_OCR_KEY).toBool());
            } else if (key == AI_INDEX_ENABLE_KEY) {
                Q_EMIT aiIndexEnableStatusChanged(m_gsettings->get(AI_INDEX_ENABLE_KEY).toBool());
            }
        });
    } else {
        qWarning() << LINGMO_SEARCH_SCHEMAS << " is not found!";
    }
    m_settings = new QSettings(INDEX_SETTINGS, QSettings::IniFormat, this);
    bool useDefaultSettings = false;
    if(m_settings->value(SETTINGS_VERSION_KEY).toString() != SETTINGS_VERSION) {
        qDebug() << "File index Settings version changed, using default file types, current version:"
                 << SETTINGS_VERSION << "previous version:"
                 << m_settings->value(SETTINGS_VERSION_KEY).toString();
        useDefaultSettings = true;
        m_settings->setValue(SETTINGS_VERSION_KEY, SETTINGS_VERSION);
    }
    syncIndexFileTypes(useDefaultSettings);
}

FileIndexerConfig::~FileIndexerConfig()
= default;

QStringList FileIndexerConfig::currentIndexableDir()
{
    return DirWatcher::getDirWatcher()->currentIndexableDir();
}

QStringList FileIndexerConfig::currentBlackListOfIndex()
{
    return DirWatcher::getDirWatcher()->currentBlackListOfIndex();
}

bool FileIndexerConfig::isFileIndexEnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(FILE_INDEX_ENABLE_KEY)) {
            return m_gsettings->get(FILE_INDEX_ENABLE_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << FILE_INDEX_ENABLE_KEY << "in" << LINGMO_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << LINGMO_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

bool FileIndexerConfig::isContentIndexEnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_KEY)) {
            return m_gsettings->get(CONTENT_INDEX_ENABLE_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << CONTENT_INDEX_ENABLE_KEY << "in" << LINGMO_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << LINGMO_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

bool FileIndexerConfig::isFuzzySearchEnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(CONTENT_FUZZY_SEARCH_KEY)) {
            return m_gsettings->get(CONTENT_FUZZY_SEARCH_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << CONTENT_FUZZY_SEARCH_KEY << "in" << LINGMO_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << LINGMO_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

bool FileIndexerConfig::isOCREnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_OCR_KEY)) {
            return m_gsettings->get(CONTENT_INDEX_ENABLE_OCR_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << CONTENT_INDEX_ENABLE_OCR_KEY << "in" << LINGMO_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << LINGMO_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

bool FileIndexerConfig::isMetaDataIndexEnable()
{
    return m_settings->value(META_DATA_INDEX_ENABLE_KEY, true).toBool();
}

bool FileIndexerConfig::isAiIndexEnable()
{
    QMutexLocker locker(&m_mutex);
    if(m_gsettings) {
        if(m_gsettings->keys().contains(AI_INDEX_ENABLE_KEY)) {
            return m_gsettings->get(AI_INDEX_ENABLE_KEY).toBool();
        } else {
            qWarning() << "FileIndexerConfig: Can not find key:" << AI_INDEX_ENABLE_KEY << "in" << LINGMO_SEARCH_SCHEMAS;
            return false;
        }
    } else {
        qWarning() << "FileIndexerConfig:" << LINGMO_SEARCH_SCHEMAS << " is not found!";
        return false;
    }
}

QMap<QString, bool> FileIndexerConfig::contentIndexTarget()
{
    QMutexLocker locker(&m_mutex);
    return m_targetFileTypeMap;
}

QMap<QString, bool> FileIndexerConfig::ocrContentIndexTarget()
{
    QMutexLocker locker(&m_mutex);
    return m_targetPhotographTypeMap;
}

QMap<QString, bool> FileIndexerConfig::aiIndexFileTarget()
{
    QMutexLocker locker(&m_mutex);
    return m_sAiFileTypeMap;
}

QMap<QString, bool> FileIndexerConfig::aiIndexImageTarget()
{
    QMutexLocker locker(&m_mutex);
    return m_sAiImageTypeMap;
}

void FileIndexerConfig::syncIndexFileTypes(bool useDefaultSettings)
{
    QMutexLocker locker(&m_mutex);
    m_settings->sync();
    //文本内容
    m_settings->beginGroup(CONTENT_INDEX_TARGET_TYPE);
    if(m_settings->allKeys().isEmpty() || useDefaultSettings) {
        m_settings->remove("");
        for(const QString &type : m_targetFileTypeMap.keys()) {
            m_settings->setValue(type, m_targetFileTypeMap[type]);
        }
    } else {
        m_targetFileTypeMap.clear();
        for(const QString &type : m_settings->allKeys()) {
            m_targetFileTypeMap.insert(type, m_settings->value(type).toBool());
        }
    }
    m_settings->endGroup();
    //图片ocr
    m_settings->beginGroup(OCR_CONTENT_INDEX_TARGET_TYPE);
    if(m_settings->allKeys().isEmpty() || useDefaultSettings) {
        m_settings->remove("");
        for(const QString &type : m_targetPhotographTypeMap.keys()) {
            m_settings->setValue(type, m_targetPhotographTypeMap[type]);
        }
    } else {
        m_targetPhotographTypeMap.clear();
        for(const QString &type : m_settings->allKeys()) {
            m_targetPhotographTypeMap.insert(type, m_settings->value(type).toBool());
        }
    }
    m_settings->endGroup();
    //“AI文本内容”
    m_settings->beginGroup(S_AI_FILE_TYPE);
    if(m_settings->allKeys().isEmpty() || useDefaultSettings) {
        m_settings->remove("");
        for(const QString &type : m_sAiFileTypeMap.keys()) {
            m_settings->setValue(type, m_sAiFileTypeMap[type]);
        }
    } else {
        m_sAiFileTypeMap.clear();
        for(const QString &type : m_settings->allKeys()) {
            m_sAiFileTypeMap.insert(type, m_settings->value(type).toBool());
        }
    }
    m_settings->endGroup();
    //“AI图片”
    m_settings->beginGroup(S_AI_IMAGE_TYPE);
    if(m_settings->allKeys().isEmpty() || useDefaultSettings) {
        m_settings->remove("");
        for(const QString &type : m_sAiImageTypeMap.keys()) {
            m_settings->setValue(type, m_sAiImageTypeMap[type]);
        }
    } else {
        m_sAiImageTypeMap.clear();
        for(const QString &type : m_settings->allKeys()) {
            m_sAiImageTypeMap.insert(type, m_settings->value(type).toBool());
        }
    }
    m_settings->endGroup();
}