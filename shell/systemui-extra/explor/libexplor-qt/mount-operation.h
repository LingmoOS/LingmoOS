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

#ifndef MOUNTOPERATION_H
#define MOUNTOPERATION_H

#include "explor-core_global.h"

#include <QObject>

#include <gio/gio.h>
#include <memory>

namespace Peony {

class ConnectServerLogin;
class GErrorWrapper;

/*!
 * \brief The MountOperation class
 * <br>
 * This class is a wrapper of GFileMountOperation.
 * MountOperation provides a interactive dialog for connecting server.
 * In gvfs, all kinds of remote location will use g_file_mount_enclosing_volume()
 * for those volumes mounting, many of them need extra infomation, such as user, password, etc.
 * This class will help to handle them together.
 * </br>
 * \note FileEnumerator::prepare() and FileEnumerator::handleError() might use this class instance.
 */
class PEONYCORESHARED_EXPORT MountOperation : public QObject
{
    Q_OBJECT
public:
    explicit MountOperation(QString uri, QObject *parent = nullptr);
    ~MountOperation();
    void setAutoDelete(bool isAuto = true) {
        m_auto_delete = isAuto;
    }

Q_SIGNALS:
    void finished(const std::shared_ptr<GErrorWrapper> &err = nullptr);
    void cancelled();

public Q_SLOTS:
    void start();
    void cancel();

protected:
    static GAsyncReadyCallback mount_enclosing_volume_callback(GFile *volume,
            GAsyncResult *res,
            MountOperation *p_this);

    static void
    aborted_cb (GMountOperation *op,
                MountOperation *p_this);

    static void
    ask_question_cb (GMountOperation *op,
                     char *message,
                     char **choices,
                     MountOperation *p_this);

    static void
    ask_password_cb (GMountOperation *op,
                     const char      *message,
                     const char      *default_user,
                     const char      *default_domain,
                     GAskPasswordFlags flags,
                     MountOperation *p_this);

private:
    bool m_auto_delete = false;
    GFile *m_volume = nullptr;
    GMountOperation *m_op = nullptr;
    GCancellable *m_cancellable = nullptr;
    /*!
     * \brief m_errs
     * \deprecated use GErrorWrapper.
     */
    GList *m_errs = nullptr;

    ConnectServerLogin *m_dlg = nullptr;
};

}

#endif // MOUNTOPERATION_H
