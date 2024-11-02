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
#include "index-status-recorder.h"
#include <mutex>

using namespace LingmoUISearch;
IndexStatusRecorder *IndexStatusRecorder::m_instance = nullptr;
std::once_flag g_IndexStatusRecorderInstanceFlag;
IndexStatusRecorder *IndexStatusRecorder::getInstance()
{
    std::call_once(g_IndexStatusRecorderInstanceFlag, [] () {
        m_instance = new IndexStatusRecorder;
    });
    return m_instance;
}

IndexStatusRecorder::IndexStatusRecorder(QObject *parent) : QObject(parent)
{
    m_status = new QSettings(INDEX_STATUS, QSettings::IniFormat, this);
}

void IndexStatusRecorder::setStatus(const QString &key, State state)
{
    m_mutex.lock();
    m_status->setValue(key, state);
    m_status->sync();
    m_mutex.unlock();
}

const QVariant IndexStatusRecorder::getStatus(const QString &key)
{
    m_mutex.lock();
    m_status->sync();
    QVariant value = m_status->value(key);
    m_mutex.unlock();
    return value;
}

void IndexStatusRecorder::setVersion(const QString &key, const QString &version)
{
    m_mutex.lock();
    m_status->setValue(key, version);
    m_status->sync();
    m_mutex.unlock();
}

bool IndexStatusRecorder::versionCheck(const QString &key, const QString &version)
{
    if(getStatus(key).toString() == version) {
        return true;
    } else {
        return false;
    }
}

bool IndexStatusRecorder::indexDatabaseEnable()
{
    m_mutex.lock();
    m_status->sync();
    int state = m_status->value(INDEX_DATABASE_STATE_KEY, 0).toInt();
    m_mutex.unlock();
    if(state == State::Ready) {
        return true;
    } else {
        return false;
    }

}

bool IndexStatusRecorder::contentIndexDatabaseEnable()
{
    m_mutex.lock();
    m_status->sync();
    int state = m_status->value(CONTENT_INDEX_DATABASE_STATE_KEY, 0).toInt();
    m_mutex.unlock();
    if(state == State::Ready) {
        return true;
    } else {
        return false;
    }
}
