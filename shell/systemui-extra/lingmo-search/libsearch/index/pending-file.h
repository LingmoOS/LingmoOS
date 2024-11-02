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
#ifndef PENDINGFILE_H
#define PENDINGFILE_H

#include <QString>
#include <QDebug>
namespace LingmoUISearch {
/**
 * Represents a file/folder which needs to be indexed.
 */
class PendingFile
{
public:
    explicit PendingFile(const QString& path = QString());

    QString path() const;
    void setPath(const QString& path);
    QString suffix();

    void setIsDir(){ m_isDir = true; }
    void setIsDir(bool isDir);

    void setModified() { m_modified = true; }
    bool isModified() { return m_modified; }

    void setCreated() { m_created = true; }
    void setMoveTo() { m_moveTo = true;}
    bool isMoveTo() { return m_moveTo;}

    void setDeleted() { m_deleted = true; }

    bool isCreated() const {return m_created;}

    bool shouldRemoveIndex() const;
//    bool shouldIndexContents() const;

    bool isDir() const;

    bool operator == (const PendingFile& rhs) const {
        return (m_path == rhs.m_path);
    }

    /**
     * Takes a PendingFile \p file and merges its flags into
     * the current PendingFile
     */
    void merge(const PendingFile& file);

private:
    QString m_path;

    bool m_created : 1;
    bool m_moveTo : 1;
    bool m_deleted : 1;
    bool m_modified : 1;
    bool m_isDir : 1;

    void printFlags() const;
};
}
#endif // PENDINGFILE_H
