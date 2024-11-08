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
 */

#ifndef VFSINFOPLUGINIFACE_H
#define VFSINFOPLUGINIFACE_H

#include "plugin-iface.h"
#include "custom-error-handler.h"
#include <QObject>
#include <file-vfs-info.h>
#include <memory>

#define VFSInfoPluginInterface_iid "org.lingmo.explorer-qt.plugin-iface.VFSInfoPluginInterface"

namespace Peony {

typedef enum{
    ERROR_FAILED,
    ERROR_NOT_FOUND,
    ERROR_EXISTS,
    ERROR_IS_DIRECTORY,
    ERROR_NOT_DIRECTORY,
    ERROR_NOT_EMPTY,
    ERROR_NOT_REGULAR_FILE,
    ERROR_NOT_SYMBOLIC_LINK,
    ERROR_NOT_MOUNTABLE_FILE,
    ERROR_FILENAME_TOO_LONG,
    ERROR_INVALID_FILENAME,
    ERROR_TOO_MANY_LINKS,
    ERROR_NO_SPACE,
    ERROR_INVALID_ARGUMENT,
    ERROR_PERMISSION_DENIED,
    ERROR_NOT_SUPPORTED,
    ERROR_NOT_MOUNTED,
    ERROR_ALREADY_MOUNTED,
    ERROR_CLOSED,
    ERROR_CANCELLED,
    ERROR_PENDING,
    ERROR_READ_ONLY,
    ERROR_CANT_CREATE_BACKUP,
    ERROR_WRONG_ETAG,
    ERROR_TIMED_OUT,
    ERROR_WOULD_RECURSE,
    ERROR_BUSY,
    ERROR_WOULD_BLOCK,
    ERROR_HOST_NOT_FOUND,
    ERROR_WOULD_MERGE,
    ERROR_FAILED_HANDLED,
    ERROR_TOO_MANY_OPEN_FILES,
    ERROR_NOT_INITIALIZED,
    ERROR_ADDRESS_IN_USE,
    ERROR_PARTIAL_INPUT,
    ERROR_INVALID_DATA,
    ERROR_DBUS_ERROR,
    ERROR_HOST_UNREACHABLE,
    ERROR_NETWORK_UNREACHABLE,
    ERROR_CONNECTION_REFUSED,
    ERROR_PROXY_FAILED,
    ERROR_PROXY_AUTH_FAILED,
    ERROR_PROXY_NEED_AUTH,
    ERROR_PROXY_NOT_ALLOWED,
    ERROR_BROKEN_PIPE,
    ERROR_CONNECTION_CLOSED = ERROR_BROKEN_PIPE,
    ERROR_NOT_CONNECTED,
    ERROR_MESSAGE_TOO_LARGE,
    ERROR_NO
}ErrorType;

struct VFSError
{
    ErrorType code = ERROR_NO;
    QString message;
};

class PEONYCORESHARED_EXPORT HanderTransfer : public QObject {
    Q_OBJECT
public:
    explicit HanderTransfer(QObject* parent = nullptr);

Q_SIGNALS:
    void fileCreate(const QString &path);
    void fileDelete(const QString &path);
    void fileChanged(const QString &path);
};

class FileVFSInfo;
class PEONYCORESHARED_EXPORT VFSInfoPluginIface : public PluginInterface
{
public:
    virtual ~VFSInfoPluginIface() {}

    virtual QString pathScheme() = 0;

    virtual bool holdInSideBar() = 0;

    virtual CustomErrorHandler *customErrorHandler() = 0;

    virtual QStringList fileEnumerator(const QString &path) = 0;

    virtual std::shared_ptr<FileVFSInfo> queryFile(const QString &path) = 0;

    //file operation
    virtual VFSError* handerMakeDirectory(const QString &uri, bool &ret) = 0;

    virtual VFSError* handerFileCreate(const QString &uri, bool &ret) = 0;

    virtual VFSError* handerFileDelete(const QString &uri, bool &ret) = 0;

    virtual VFSError* handerFileCopy(const QString &sourceUri, const QString &destUri, bool &ret) = 0;

    virtual VFSError* handerFileMove(const QString &sourceUri, const QString &destUri, bool &ret) = 0;

    virtual VFSError* handerFileRename(const QString &sourceUri, const QString &newName) = 0;

    virtual void handerMonitorDirectory(HanderTransfer *transfer, const QString &uri) = 0;
};

}

Q_DECLARE_INTERFACE(Peony::VFSInfoPluginIface, VFSInfoPluginInterface_iid)
#endif // VFSINFOPLUGINIFACE_H
