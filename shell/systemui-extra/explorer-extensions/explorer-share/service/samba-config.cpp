/*
 * Copyright (C) 2022, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */
#include "samba-config.h"
#include <QMutex>
#include <glib.h>
#include <QCoreApplication>
#include <QProcess>

#include <polkit/polkit.h>
#include <QDebug>
#include <syslog.h>
#include <QFile>

#include <pwd.h>

class SambaConfigPrivate
{
public:
    explicit SambaConfigPrivate (SambaConfig* sm);

    bool launchSmbd();
    bool smbdIsActive ();
    bool launchNmbd();
    bool nmbdIsActive();
    bool userInSamba ();
    bool userInSambaGroup();
    bool addUserToSambaGroup();
    bool setUserPasswd(const QString& pass);

    bool checkAuthorization ();

public:
    SambaConfig*        q_ptr;

    QString             mUserName;
    int                 mUserUid;
    int                 mUserPid;
    PolkitAuthority*    mAuth = nullptr;

    QMutex              mLock;
};

SambaConfigPrivate::SambaConfigPrivate(SambaConfig *sm)
    : q_ptr(sm), mUserUid(-1), mUserPid(-1)
{
    g_autoptr(GError) error = NULL;

    mAuth = polkit_authority_get_sync(NULL, &error);
    if (error) {
        qWarning() << error->message;
    }
}

bool SambaConfigPrivate::launchSmbd()
{
    QString startCmd = QString("systemctl start smbd.service");
    QString enableCmd = QString("systemctl enable smbd.service");
    QProcess process;
    process.start("/usr/bin/bash",QStringList() << "-c" << startCmd);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    syslog(LOG_DEBUG, "launchSmbd output:%s error:%s", output.toLatin1().data(), error.toLatin1().data());
    if (output.isEmpty() && error.isEmpty()) {
        QProcess p;
        p.start("/usr/bin/bash",QStringList() << "-c" << enableCmd);
        p.waitForFinished();
        QString out = p.readAllStandardOutput();
        QString err = p.readAllStandardError();
        syslog(LOG_DEBUG, "launchSmbd enabele out:%s err:%s", out.toLatin1().data(), err.toLatin1().data());
        if (out.isEmpty() && err.contains("enable smbd")) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool SambaConfigPrivate::smbdIsActive()
{
    //return QProcess::execute("systemctl", QStringList()<<"status"<<"smbd") ? false : true;
    //g_autofree gchar* cmd = g_strdup_printf ("ps aux | grep smbd | grep root | wc -l");
    //int ret = QProcess::execute(cmd);
    QProcess process;
    process.start("/usr/bin/bash",QStringList() << "-c" << "ps aux | grep smbd | grep root | wc -l");
    process.waitForFinished();
    int ret = process.readAllStandardOutput().toInt();
    QString error = process.readAllStandardError();
    syslog(LOG_DEBUG, "smbdIsActive ret:%d error:%s", ret, error.toLatin1().data());
    if (ret - 1 > 1 && error.isEmpty()) {
        return true;
    } else {
        return false;
    }
}

bool SambaConfigPrivate::launchNmbd()
{
    QString startCmd = QString("systemctl start nmbd.service");
    QString enableCmd = QString("systemctl enable nmbd.service");
    QProcess process;
    process.start("/usr/bin/bash",QStringList() << "-c" << startCmd);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    syslog(LOG_DEBUG, "launchNmbd output:%s error:%s", output.toLatin1().data(), error.toLatin1().data());
    if (output.isEmpty() && error.isEmpty()) {
        QProcess p;
        p.start("/usr/bin/bash",QStringList() << "-c" << enableCmd);
        p.waitForFinished();
        QString out = p.readAllStandardOutput();
        QString err = p.readAllStandardError();
        syslog(LOG_DEBUG, "launchNmbd enabele out:%s err:%s", out.toLatin1().data(), err.toLatin1().data());
        if (out.isEmpty() && err.contains("enable nmbd")) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool SambaConfigPrivate::nmbdIsActive()
{
    //return QProcess::execute("systemctl", QStringList()<<"status"<<"nmbd") ? false : true;
    QProcess process;
    process.start("/usr/bin/bash",QStringList() << "-c" << "ps aux | grep nmbd | grep root | wc -l");
    process.waitForFinished();
    int ret = process.readAllStandardOutput().toInt();
    QString error = process.readAllStandardError();
    syslog(LOG_DEBUG, "nmbdIsActive ret:%d error:%s", ret, error.toLatin1().data());
    if (ret - 1 > 1 && error.isEmpty()) {
        return true;
    } else {
        return false;
    }
}

bool SambaConfigPrivate::userInSamba()
{
    if (mUserName.isEmpty() || -1 == mUserUid) {
        return false;
    }

    QString cmd = QString("/usr/bin/smbpasswd -e %1").arg(mUserName.toUtf8().constData());
    QProcess process;
    process.start("/usr/bin/bash",QStringList() << "-c" << cmd);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    syslog(LOG_DEBUG, "userInSamba cmd:%s output:%s error:%s", cmd.toLatin1().data(), output.toLatin1().data(), error.toLatin1().data());
    if (output.contains("Enabled user") && error.isEmpty()) {
        return true;
    } else {
        return false;
    }
}

bool SambaConfigPrivate::userInSambaGroup()
{
    if (mUserName.isEmpty() || -1 == mUserUid) {
        return false;
    }

    QString cmd = QString("/usr/bin/groups %1 | grep sambashare").arg(mUserName.toUtf8().constData());
    QProcess process;
    process.start("/usr/bin/bash",QStringList() << "-c" << cmd);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    syslog(LOG_DEBUG, "userInSambaGroup cmd:%s output:%s error:%s", cmd.toLatin1().data(), output.toLatin1().data(), error.toLatin1().data());
    if (output.contains("sambashare") && error.isEmpty()) {
        return true;
    } else {
        return false;
    }
}

bool SambaConfigPrivate::addUserToSambaGroup()
{
    if (mUserName.isEmpty() || -1 == mUserUid) {
        return false;
    }
//    QProcess p;
//    p.setProgram("usermod");
//    p.setArguments(QStringList() << "-G" << "sambashare" << "-a" << mUserName);
//    p.start();
//    p.waitForFinished(-1);

    QString usermod = "/usr/sbin/usermod";
    QString gpasswd = "/usr/bin/gpasswd";
    QString command;
    QString groupName = "sambashare";

    QFile usermodFile(usermod);
    QFile gpasswdFile(gpasswd);

    QProcess p(0);
    QStringList args;

    if(!usermodFile.exists()){
        syslog(LOG_DEBUG, "/usr/sbin/usermod file not exist \n");
        if(!gpasswdFile.exists()){
            syslog(LOG_DEBUG, "/usr/sbin/gpasswd file not exist \n");
            return false;
        }
        command = gpasswd;
        args.append("-a");
        args.append(mUserName);
        args.append(groupName);
    } else {
        command = usermod;
        args.append("-a");
        args.append("-G");
        args.append(groupName);
        args.append(mUserName);
    }
    p.execute(command,args);//command是要执行的命令,args是参数
    p.waitForFinished(-1);

    QString output = p.readAllStandardOutput();
    QString error = p.readAllStandardError();
    syslog(LOG_DEBUG, "addUserToSambaGroup output:%s error:%s", output.toLatin1().data(), error.toLatin1().data());
    if (error.isEmpty() && output.isEmpty()) {
        return true;
    } else {
        return false;
    }
}

bool SambaConfigPrivate::setUserPasswd(const QString &pass)
{
    if (mUserName.isEmpty() || -1 == mUserUid || pass.isEmpty()) {
        return false;
    }

    qDebug() << "username:" << mUserName << " change passwd!";

//    g_autofree gchar* cmd = g_strdup_printf ("spawn smbpasswd -a \"%s\";"
//                                             "expect \"*New SMB password*\"; send \"%s\\n\";"
//                                             "expect \"*Retype new SMB password*\"; send \"%s\\n\";"
//                                             "expect eof; exit", mUserName.toUtf8().constData(),
//                                             pass.toUtf8().constData(), pass.toUtf8().constData());
    //
    //QProcess::execute("expect", QStringList() << "-c" << cmd);


    QProcess p2;

    p2.start(QString("/usr/bin/smbpasswd -a %1").arg(mUserName));

    p2.waitForBytesWritten();
    p2.write(pass.toUtf8().constData());
    p2.write("\n");

    p2.waitForBytesWritten();
    p2.write(pass.toUtf8().constData());
    p2.write("\n");

    p2.waitForFinished();

    QString err = p2.readAllStandardError();
    if (!err.isEmpty()) {
        return false;
    }

    int ret = userInSamba();
    return ret;
}

bool SambaConfigPrivate::checkAuthorization()
{
    bool ret = false;

    g_autoptr(GError) error = nullptr;

    PolkitSubject* proj = polkit_unix_process_new_for_owner (mUserPid, 0, mUserUid);

    PolkitAuthorizationResult* res = polkit_authority_check_authorization_sync (mAuth, proj, "org.lingmo.samba.share.config.authorization", NULL,
                                                                                          POLKIT_CHECK_AUTHORIZATION_FLAGS_ALLOW_USER_INTERACTION, nullptr, &error);
    if (error) {
        qWarning() << error->message;
        goto out;
    }

    if (polkit_authorization_result_get_is_authorized(res)) {
        ret = true;
    }

out:
    if (proj)       g_object_unref (proj);
    if (res)        g_object_unref (res);

    return ret;
}

const SambaConfig *SambaConfig::getInstance()
{
    static SambaConfig sc;

    return &sc;
}

SambaConfig::SambaConfig(QObject *parent)
    : QObject{parent}, d_ptr (new SambaConfigPrivate(this))
{

}

bool SambaConfig::init(QString name, int pid, int uid)
{
    Q_D (SambaConfig);

    qDebug() << "init";
    if (name.isEmpty() || pid < 0 || uid < 0) {
        return false;
    }

    // KVE-2023-1201
    if (name.contains("|"))
        return false;
    QStringList allUsers;
    struct passwd *pw;
    setpwent();
    while ((pw = getpwent()) != nullptr) {
        allUsers << pw->pw_name;
    }
    endpwent();
    if (!allUsers.contains(name)) {
        return false;
    }

    if (d->mLock.tryLock(300)) {
        d->mUserName = name;
        d->mUserPid = pid;
        d->mUserUid = uid;

        bool ret = true;

        // smbd
        if (!isSmbdLaunched()) {
            ret = ret && launchSmbd();
        }

        // nmbd
        if (!isNmbdLaunched()) {
            ret = ret && launchNmbd();
        }

        // sambashare
        if (!userIsInSambaGroup()) {
            ret = ret && addUserInGroup ();
        }

        return ret;
    }

    return false;
}

void SambaConfig::finished()
{
    Q_D (SambaConfig);
    d->mLock.unlock();
    d->mUserName = "";
    d->mUserPid = -1;
    d->mUserUid = -1;

    qDebug() << "finished";

    QCoreApplication::exit(0);
}

bool SambaConfig::launchSmbd()
{
    Q_D (SambaConfig);

    if (d->checkAuthorization()) {
        return d->launchSmbd();
    }

    return false;
}

bool SambaConfig::isSmbdLaunched()
{
    Q_D (SambaConfig);

    return d->smbdIsActive();
}

bool SambaConfig::launchNmbd()
{
    Q_D (SambaConfig);

    if (d->checkAuthorization()) {
        return d->launchNmbd();
    }

    return false;
}

bool SambaConfig::isNmbdLaunched()
{
    Q_D (SambaConfig);

    return d->nmbdIsActive();
}

bool SambaConfig::userIsInSambaGroup()
{
    Q_D (SambaConfig);

    return d->userInSambaGroup();
}

bool SambaConfig::addUserInGroup()
{
    Q_D (SambaConfig);

    if (d->checkAuthorization()) {
        return d->addUserToSambaGroup();
    }

    return false;
}

bool SambaConfig::isContainLegitimacyChar(QString passwd)
{
    //需要用'在shell解释中做强引用
    if (passwd.contains("'"))
        return false;

    if (passwd.contains("&") || passwd.contains(" ")) {
        return false;
    }

    for (QChar ch : passwd){
        if (int(ch.toLatin1() <= 0)){
            return false;
        }
    }
    return true;
}

bool SambaConfig::hasPasswd()
{
    Q_D (SambaConfig);

    if (d->checkAuthorization()) {
        return d->userInSamba();
    }

    return false;
}

bool SambaConfig::setPasswd(QString passwd)
{
    Q_D (SambaConfig);

    if (passwd.isEmpty() || !isContainLegitimacyChar(passwd)) {
        return false;
    }

    if (d->checkAuthorization()) {
        return d->setUserPasswd(passwd);
    }

    return false;
}


