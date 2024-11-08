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

#include "samba-config-thread.h"
#include <PeonyFileInfo>

SambaConfigThread::SambaConfigThread(const Peony::ShareInfo shareInfo, const QString uri, const QString acl)
{
    m_shareInfoTmp = new Peony::ShareInfo();
    m_shareInfoTmp->allowGuest = shareInfo.allowGuest;
    m_shareInfoTmp->comment = shareInfo.comment;
    m_shareInfoTmp->isShared = shareInfo.isShared;
    m_shareInfoTmp->name = shareInfo.name;
    m_shareInfoTmp->originalPath = shareInfo.originalPath;
    m_shareInfoTmp->readOnly = shareInfo.readOnly;
    m_uri = uri;
    m_acl = acl;
}

Peony::ShareInfo *SambaConfigThread::getShareInfo()
{
    return m_shareInfoTmp;
}

QString SambaConfigThread::getUri()
{
    return m_uri;
}

QString SambaConfigThread::getAcl()
{
    return m_acl;
}

void SambaConfigThread::run()
{
    SambaConfigInterface si(DBUS_NAME, DBUS_PATH, QDBusConnection::systemBus());

//    struct timeval tpstart,tpend;
//    float timeuse;
//    gettimeofday(&tpstart, NULL);
    bool initRet = si.init(g_get_user_name(), getpid(), getuid());
//    gettimeofday(&tpend, NULL);
//    timeuse = (1000000*(tpend.tv_sec-tpstart.tv_sec) + tpend.tv_usec-tpstart.tv_usec)/1000000.0;

    Q_EMIT isInitSignal(initRet);
    if (initRet) {
        bool hasPasswdRet = si.hasPasswd();
        QString passwd = "";
        qDebug()<< __func__ << __LINE__ << hasPasswdRet;
        Q_EMIT isHasPasswdSignal(hasPasswdRet, passwd);
        if (!passwd.isEmpty()) {
            qDebug()<< __func__ << __LINE__ << passwd;
            bool setPasswdRet = si.setPasswd(passwd);
            Q_EMIT isSetPasswdSignal(setPasswdRet);
        }
    }
    si.finished();
    qDebug()<< __func__ << __LINE__ << "samba finished";
}
