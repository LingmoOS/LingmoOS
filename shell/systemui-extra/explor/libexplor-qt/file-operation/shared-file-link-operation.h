/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: Yan Wei Yang <yangyanwei@kylinos.cn>
 *
 */

#ifndef SHAREDFILELINKOPERATION_H
#define SHAREDFILELINKOPERATION_H

#include "explor-core_global.h"
#include "file-operation.h"

namespace Peony {

class PEONYCORESHARED_EXPORT SharedFileLinkOperation : public FileOperation
{
    Q_OBJECT
public:
    SharedFileLinkOperation(QString srcUri, QString destDirUri, QObject *parent = nullptr);
    ~SharedFileLinkOperation() override;

    std::shared_ptr<FileOperationInfo> getOperationInfo() override {
        return m_info;
    }
    void run() override;
    void createShareFilesSymbolicLink(QString &srcUri);
private:
    QString m_src_uri = nullptr;
    QString m_dest_uri = nullptr;
    QString m_display_name = nullptr;

    std::shared_ptr<FileOperationInfo> m_info = nullptr;
};

}

#endif // SHAREDFILELINKOPERATION_H
