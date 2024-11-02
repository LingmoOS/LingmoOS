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
#ifndef DATABASE_H
#define DATABASE_H
#include <QMutex>
#include <xapian.h>
#include "document.h"
#include "common.h"
namespace LingmoUISearch {
/**
 * @brief The WritableDatabase class\
 * Wrapper for Xapian::WritableDatabase
 */
class WritableDatabase
{
    friend class Transaction;
public:

    WritableDatabase(const DataBaseType &type);
    ~WritableDatabase();
    bool open();
    void rebuild();

    void beginTransation();
    void endTransation();
    void cancelTransation();
    void commit();

    void addDocument(const Document& doc);
    void removeDocument(const QString& path);
    void removeDocument(const std::string uniqueTerm);
    void removeChildrenDocument(const QString& path);
    void setMetaData(const QString& key, const QString& value);
    void setMetaData(const std::string& key, const std::string& value);
    const std::string getMetaData(const std::string &key);
    QMap<std::string, std::string> getIndexTimes();

private:
    WritableDatabase(const WritableDatabase& rhs) = delete;
    void errorRecord();

    DataBaseType m_type;
    QString m_path;
    Xapian::WritableDatabase* m_xpDatabase = nullptr;
    QMutex *m_mutex = nullptr;
};
}
#endif // DATABASE_H
