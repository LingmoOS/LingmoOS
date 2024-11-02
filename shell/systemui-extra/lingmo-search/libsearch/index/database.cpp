/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#include "database.h"
#include <QDir>

using namespace LingmoUISearch;
Database::Database(const DataBaseType &type): m_type(type)
{
    refresh();
}

Database::~Database()
{
    if(m_database) {
        delete m_database;
        m_database = nullptr;
    }
}

uint Database::getIndexDocCount()
{
    if(!valid()) {
        refresh();
    }
    if(m_database) {
        return m_database->get_doccount();
    } else {
        return 0;
    }
}

bool Database::valid() const
{
    return m_database != nullptr;
}

bool Database::refresh()
{
    switch (m_type) {
        case DataBaseType::Basic:
            if(QDir(INDEX_PATH).exists()) {
                m_database = new Xapian::Database(INDEX_PATH.toStdString());
                break;
            }
        case DataBaseType::Content:
            if(QDir(CONTENT_INDEX_PATH).exists()) {
                m_database = new Xapian::Database(CONTENT_INDEX_PATH.toStdString());
                break;
            }
        case DataBaseType::OcrContent:
            if(QDir(OCR_CONTENT_INDEX_PATH).exists()) {
                m_database = new Xapian::Database(OCR_CONTENT_INDEX_PATH.toStdString());
                break;
            }
        default:
            break;
    }
    return m_database != nullptr;
}
