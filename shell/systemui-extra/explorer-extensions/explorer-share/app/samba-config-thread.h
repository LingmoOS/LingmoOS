/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef SAMBACONFIGTHREAD_H
#define SAMBACONFIGTHREAD_H

#include <QObject>
#include "samba-config-interface.h"
#include "usershare-manager.h"

class SambaConfigThread : public QThread
{
    Q_OBJECT
public:
    SambaConfigThread(const Peony::ShareInfo shareInfo, const QString uri, const QString acl);
    Peony::ShareInfo* getShareInfo();
    QString getUri();
    QString getAcl();

Q_SIGNALS:
    void isInitSignal(bool ret);
    void isFinishedSignal(bool ret);
    void isHasPasswdSignal(bool ret, QString &passwd);
    void isSetPasswdSignal(bool ret);
    void getPasswd(QString &passwd);

protected:
    void run();
private:
    Peony::ShareInfo *m_shareInfoTmp;
    QString m_uri;
    QString m_acl;
};

#endif // SAMBACONFIGTHREAD_H
