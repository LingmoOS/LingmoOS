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

#ifndef CREATETEMPLATEOPERATION_H
#define CREATETEMPLATEOPERATION_H

#include <QObject>
#include "explor-core_global.h"

#include "file-operation.h"

namespace Peony {

class PEONYCORESHARED_EXPORT CreateTemplateOperation : public FileOperation
{
    Q_OBJECT
public:
    enum Type {
        EmptyFile,
        EmptyFolder,
        Template
    };

    explicit CreateTemplateOperation(const QString &destDirUri, Type type = EmptyFile, const QString &templateName = nullptr, QObject *parent = nullptr);
    void run() override;
    std::shared_ptr<FileOperationInfo> getOperationInfo() override {
        return m_info;
    }

    const QString target() {
        return m_target_uri;
    }

protected:
    void handleDuplicate(const QString &uri);

private:
    std::shared_ptr<FileOperationInfo> m_info;

    QString m_src_uri;
    QString m_dest_dir_uri;
    QString m_target_uri;
    Type m_type;
};

}

#endif // CREATETEMPLATEOPERATION_H
