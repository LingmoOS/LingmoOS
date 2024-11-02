/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhuoan Li <lizhuoan@kylinos.cn>
 *
 */

#ifndef FILEOPERATIONHELPER_H
#define FILEOPERATIONHELPER_H

#ifdef LINGMO_UDF_BURN

#include "file-operation.h"
#include <QObject>

class FileOperationHelper : public QObject
{
    Q_OBJECT
public:
    explicit FileOperationHelper(const QString destDir,QObject *parent = nullptr);
    ~FileOperationHelper();

    void judgeSpecialDiscOperation();

    QString getDiscType();

    QString dealDVDReduce();

    bool isUnixCDDevice();

    bool discWriteOperation(const QStringList &sourUrisList, const QString &destUri);

    void discRenameOperation(const QString &oldName,const QString &newName);

    void discDeleteOperation(const QStringList &srcUris);

    QString getDestName(const QString &destUri);

    QString getDiscError();
protected:
    /**
     * @brief matchingUnixDevice
     * @param uri
     * @return
     * Matches the device mount point
     */
    QString matchingUnixDevice(QString uri);

private:
    bool m_is_disk_work = false;
    QString m_unix_device = nullptr;
    QString m_disc_media_type = nullptr;
    QString m_disc_system_type = nullptr;
    QString m_disc_error_msg = nullptr;

};

#endif

#endif // FILEOPERATIONHELPER_H
