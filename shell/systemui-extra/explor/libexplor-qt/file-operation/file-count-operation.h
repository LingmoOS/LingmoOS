/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILECOUNTOPERATION_H
#define FILECOUNTOPERATION_H

#include "file-operation.h"

namespace Peony {

class FileNodeReporter;

class PEONYCORESHARED_EXPORT FileCountOperation : public FileOperation
{
    Q_OBJECT
public:
    explicit FileCountOperation(const QStringList &uris, bool countRoot = true, QObject *parent = nullptr);
    ~FileCountOperation() override;
    void run() override;
    std::shared_ptr<FileOperationInfo> getOperationInfo() override {
        return nullptr;
    }
    void getInfo(quint64 &file_count, quint64 &hidden_file_count, quint64 &total_size) {
        file_count = m_file_count;
        hidden_file_count = m_hidden_file_count;
        total_size = m_total_size;
    }

Q_SIGNALS:
    void countDone(quint64 file_count, quint64 hidden_file_count, quint64 total_size);

public Q_SLOTS:
    void cancel() override;

private:
    FileNodeReporter *m_reporter = nullptr;
    QStringList m_uris;

    quint64 m_file_count = 0;
    quint64 m_hidden_file_count = 0;
    quint64 m_total_size = 0;

    bool m_count_root = true;
};

}

#endif // FILECOUNTOPERATION_H
