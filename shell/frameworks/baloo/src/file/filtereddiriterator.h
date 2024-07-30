/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FILTEREDDIRITERATOR_H
#define FILTEREDDIRITERATOR_H

#include <QDirIterator>
#include <QFileInfo>
#include <QStack>

namespace Baloo {

class FileIndexerConfig;

class FilteredDirIterator
{
public:
    enum Filter {
        FilesAndDirs,
        DirsOnly,
    };
    FilteredDirIterator(const FileIndexerConfig* config, const QString& folder, Filter filter = FilesAndDirs);
    ~FilteredDirIterator();

    FilteredDirIterator(const FilteredDirIterator &) = delete;
    FilteredDirIterator &operator=(const FilteredDirIterator &) = delete;

    QString next();
    QString filePath() const;
    QFileInfo fileInfo() const;

private:
    const FileIndexerConfig* m_config;

    QDirIterator* m_currentIter;
    QStack<QString> m_paths;
    QDir::Filters m_filters;

    QString m_filePath;
    QFileInfo m_fileInfo;
    bool m_firstItem;
};

}

#endif // FILTEREDDIRITERATOR_H
