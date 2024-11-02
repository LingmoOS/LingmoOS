/*
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 *
 */
#include "pending-file.h"
using namespace LingmoUISearch;
PendingFile::PendingFile(const QString &path)
    : m_path(path)
    , m_deleted(false)
    , m_modified(false)
    , m_isDir(false)
{

}

QString PendingFile::path() const
{
    return m_path;
}

void PendingFile::setPath(const QString& path)
{
    if (path.endsWith(QLatin1Char('/'))) {
        m_path = path.mid(0, m_path.length() - 1);
        return;
    }
    m_path = path;
}

void PendingFile::setIsDir(bool isDir)
{
    m_isDir = isDir;
}

//bool PendingFile::isNewFile() const
//{
//    return m_created;
//}

//bool PendingFile::shouldIndexContents() const
//{
//    if (m_created || m_modified) {
//        return true;
//    }
//    return false;
//}

bool PendingFile::isDir() const
{
    return m_isDir;
}

bool PendingFile::shouldRemoveIndex() const
{
    return m_deleted;
}

void PendingFile::merge(const PendingFile& file)
{
    m_created |= file.m_created;
    m_modified |= file.m_modified;
    m_deleted = file.m_deleted;
    m_moveTo |= file.m_moveTo;
}

void PendingFile::printFlags() const
{
    qDebug() << "Created:" << m_created;
    qDebug() << "Deleted:" << m_deleted;
    qDebug() << "Modified:" << m_modified;
    qDebug() << "Is dir:" << m_isDir;
}
