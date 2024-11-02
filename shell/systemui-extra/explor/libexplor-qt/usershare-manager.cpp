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
 * Authors: Yang Ling <yangling@kylinos.cn>
 *
 */

#include "usershare-manager.h"
#include "file-utils.h"

#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

#include <glib.h>

using namespace Peony;

UserShareInfoManager* UserShareInfoManager::g_shareInfo = nullptr;
QMutex SharedDeleteInfoThread::m_mutex;

static void         parseShareInfo (ShareInfo& shareInfo, QString& content);
static QString      exectueCommand (QStringList& args, bool* ret /* out */);

ShareInfo& ShareInfo::operator =(const ShareInfo* oth)
{
    if (oth == this) {
        return *this;
    }

    this->name = oth->name;
    this->comment = oth->comment;
    this->isShared = oth->isShared;
    this->readOnly = oth->readOnly;
    this->allowGuest = oth->allowGuest;
    this->originalPath = oth->originalPath;

    return *this;
}

UserShareInfoManager* UserShareInfoManager::getInstance()
{
    if (!g_shareInfo) {
        g_shareInfo = new UserShareInfoManager;
        //gShareInfo->run();
    }

    return g_shareInfo;
}


static void parseShareInfo (ShareInfo& shareInfo, QString& content)
{
    auto lines = content.split('\n');

    for (auto line : lines) {
        if (line.startsWith("path")) {
            shareInfo.originalPath = line.split('=').last();
        } else if (line.startsWith("comment")) {
            shareInfo.comment = line.split('=').last();
        } else if (line.startsWith("usershare_acl")) {
            shareInfo.readOnly = line.contains("Everyone:R");
        } else if (line.startsWith("guest_ok")) {
            shareInfo.allowGuest = line.split('=').last() == "y";
        }
    }
}

QString UserShareInfoManager::exectueCommand (QStringList& args, bool* retb /* out */, QString sharedPath)
{
    Q_UNUSED(sharedPath);
    QProcess proc;
    proc.open();

    // Check whether sambashare exists and contains the current user
    //QProcess::execute ("bash pkexec /usr/bin/explor-share.sh", QStringList() << g_get_user_name () << sharedPath);

    // Shared folder
    args.prepend ("net");
    proc.start("/usr/bin/bash");
//    args.prepend("pkexec");
    proc.waitForStarted();
    QString cmd = args.join(" ");
    QString err;
    proc.write(cmd.toUtf8() + "\n");
    proc.waitForFinished(500);
    err = proc.readAllStandardError();
    if (retb) {
        if (err.isEmpty()) {
            *retb = true;
        } else {
            *retb = false;
        }
    }

    if (!err.isEmpty() && cmd.contains("usershare add")) {
        proc.close();
        QMessageBox::warning(nullptr, tr("Warning"), err, QMessageBox::Ok);
        return err;
    }

    QString all = proc.readAllStandardOutput();
    proc.close();

    return all;
}

QString UserShareInfoManager::exectueSetAclCommand(QStringList &args, bool *ret)
{
    QProcess proc;
    proc.open();

    proc.start("/usr/bin/bash");
    proc.waitForStarted();
    QString cmd = args.join(" ");
    QString error;
    proc.write(cmd.toUtf8() + "\n");
    proc.waitForFinished(500);
    error = proc.readAllStandardError();

    if (ret) {
        if (error.isEmpty()) {
            *ret = true;
        } else {
            *ret = false;
        }
    }

    if (!error.isEmpty()) {
        proc.close();
        QMessageBox::warning(nullptr, tr("Warning"), error, QMessageBox::Ok);
        return error;
    }

    QString all = proc.readAllStandardOutput();
    proc.close();

    return all;
}

bool UserShareInfoManager::updateShareInfo(ShareInfo &shareInfo)
{
    if ("" == shareInfo.name
            || shareInfo.name.isEmpty()
            || shareInfo.originalPath.isEmpty()) {
        return false;
    }

    bool ret = false;
    QStringList args;
    ShareInfo* sharedInfo = new ShareInfo;
    sharedInfo->name = shareInfo.name;
    sharedInfo->comment = shareInfo.comment;
    sharedInfo->isShared = shareInfo.isShared;
    sharedInfo->readOnly = shareInfo.readOnly;
    sharedInfo->allowGuest = shareInfo.allowGuest;
    sharedInfo->originalPath = shareInfo.originalPath;

    m_mutex.lock();
    bool isShare = true;
    if (m_sharedInfoMap.contains(sharedInfo->name)
            && nullptr != m_sharedInfoMap[sharedInfo->name]) {
        if(sharedInfo->isShared == m_sharedInfoMap[sharedInfo->name]->isShared){
            isShare = false;
        }
        delete m_sharedInfoMap[sharedInfo->name];
    }
    m_sharedInfoMap[sharedInfo->name] = sharedInfo;
    m_mutex.unlock();

    args << "usershare" << "add";
    args << QString("\"%1\"").arg(sharedInfo->name);
    args << QString("\"%1\"").arg(sharedInfo->originalPath);
    args << (sharedInfo->comment.isNull() ? "Peony-Qt-Share-Extension" : sharedInfo->comment);
    args << (sharedInfo->readOnly ? "Everyone:R" : "Everyone:F");
    args << (sharedInfo->allowGuest ? "guest_ok=y" : "guest_ok=n");

    exectueCommand (args, &ret);
    if(isShare)
        Q_EMIT signal_addSharedFolder(*sharedInfo, ret);
    return ret;
}

bool UserShareInfoManager::updateShareInfo(ShareInfo &shareInfo, const QString usershareAcl)
{
    if ("" == shareInfo.name
            || shareInfo.name.isEmpty()
            || shareInfo.originalPath.isEmpty()) {
        return false;
    }

    bool readOnly = shareInfo.readOnly;
    if (!usershareAcl.isEmpty()
            && (usershareAcl.compare("Everyone:F", Qt::CaseInsensitive) == 0
                || usershareAcl.compare("Everyone:D", Qt::CaseInsensitive) == 0)) {
        readOnly = false;
    }

    bool ret = false;
    QStringList args;
    ShareInfo* sharedInfo = new ShareInfo;
    sharedInfo->name = shareInfo.name;
    sharedInfo->comment = shareInfo.comment;
    sharedInfo->isShared = shareInfo.isShared;
    sharedInfo->readOnly = readOnly;
    sharedInfo->allowGuest = shareInfo.allowGuest;
    sharedInfo->originalPath = shareInfo.originalPath;

    m_mutex.lock();
    bool isShare = true;
    if (m_sharedInfoMap.contains(sharedInfo->name)
            && nullptr != m_sharedInfoMap[sharedInfo->name]) {
        if(sharedInfo->isShared == m_sharedInfoMap[sharedInfo->name]->isShared){
            isShare = false;
        }
        delete m_sharedInfoMap[sharedInfo->name];
    }
    m_sharedInfoMap[sharedInfo->name] = sharedInfo;
    if (m_usershareAclMap.contains(sharedInfo->name) && !usershareAcl.isEmpty()) {
        m_usershareAclMap.remove(sharedInfo->name);
    }
    m_usershareAclMap.insert(sharedInfo->name, usershareAcl);

    m_mutex.unlock();

    args << "usershare" << "add";
    args << QString("\"%1\"").arg(sharedInfo->name);
    args << QString("\"%1\"").arg(sharedInfo->originalPath);
    args << (sharedInfo->comment.isNull() ? "Peony-Qt-Share-Extension" : sharedInfo->comment);
    if (usershareAcl.isEmpty()) {
         args << (sharedInfo->readOnly ? "Everyone:R" : "Everyone:F");
    } else {
        args << usershareAcl;
    }
    args << (sharedInfo->allowGuest ? "guest_ok=y" : "guest_ok=n");

    exectueCommand (args, &ret);
    if(isShare)
        Q_EMIT signal_addSharedFolder(*sharedInfo, ret);
    return ret;
}

void UserShareInfoManager::removeShareInfoAcl(QString &name)
{
    m_mutex.lock();
    QString originalPath;
    if (m_sharedInfoMap.contains(name)) {
        if (nullptr != m_sharedInfoMap[name])
        {
            originalPath=m_sharedInfoMap[name]->originalPath;
            delete m_sharedInfoMap[name];
        }
        m_sharedInfoMap.remove(name);
    }
    if (m_usershareAclMap.contains(name)) {
        if (!m_usershareAclMap[name].isEmpty())
        {
             m_usershareAclMap.remove(name);
        }
    }
    m_mutex.unlock();

    QStringList args;
    args << "usershare" << "delete" << QString("\"%1\"").arg(name);

    bool ret = false;
    exectueCommand (args, &ret);
    Q_EMIT signal_deleteSharedFolder(originalPath, ret);
}

const ShareInfo* UserShareInfoManager::getShareInfo(QString &name)
{
    if (nullptr == name || name.isEmpty()) {
        qDebug() << "invalid param";
        return nullptr;
    }

    if (!m_bInit) {
        bool            ret;
        QStringList     args;
        args << "usershare" << "info" << QString("\"%1\"").arg(name);
        QString result = exectueCommand (args, &ret);
        if (!ret && result.isEmpty()) {
            return nullptr;
        }

        // parse UserShared
        ShareInfo* shareInfo = new ShareInfo;
        shareInfo->name = name;
        parseShareInfo(*shareInfo, result);
        if (!addShareInfo(shareInfo)) {
            delete shareInfo;
        }
    }

    m_mutex.lock();
    if (!m_sharedInfoMap.contains(name)) {
        m_mutex.unlock();
        return nullptr;
    }

    m_mutex.unlock();

    return m_sharedInfoMap[name];
}

QString UserShareInfoManager::getUserShareAcl(QString &name)
{
    QString acl;
    if (nullptr == name || name.isEmpty()) {
        qDebug() << "invalid param";
        return acl;
    }

    if (!m_bInit) {
        bool            ret;
        QStringList     args;
        args << "usershare" << "info" << QString("\"%1\"").arg(name);
        QString result = exectueCommand (args, &ret);
        if (!ret && result.isEmpty()) {
            return acl;
        }

        QString usershareAcl  = parseUserShareAcl(result);
        if (!addUserShareAcl(name, usershareAcl)) {
            qDebug() << "Add usershare failed or usershare isExist";
        }
    }

    m_mutex.lock();
    if (!m_usershareAclMap.contains(name)) {
        m_mutex.unlock();
        return acl;
    }

    m_mutex.unlock();
    return m_usershareAclMap[name];
}

bool UserShareInfoManager::addUserShareAcl(QString &name, QString &acl)
{
    if (nullptr == name || name.isEmpty() || acl.isEmpty()) {
        return false;
    }

    m_mutex.lock();
    if (m_usershareAclMap.contains(name)) {
        m_mutex.unlock();
        return false;
    }

    m_usershareAclMap[name] = acl;
    m_mutex.unlock();

    return true;
}

QString UserShareInfoManager::parseUserShareAcl(QString &content)
{
    auto lines = content.split('\n');
    QString acl;

    for (auto line : lines) {
        if (line.startsWith("usershare_acl")) {
            acl = line;
            acl.remove(0, 14);
        }
    }
    return acl;
}

bool UserShareInfoManager::checkDirAdvancedShare(QString &name)
{
    bool ret = true;
    if (m_usershareAclMap[name].isEmpty()
            || 11 == m_usershareAclMap[name].size()
            || 10 == m_usershareAclMap[name].size()) {
        ret = false;
    }
    return ret;
}

QStringList UserShareInfoManager::getUsershareLists()
{
    return m_usersharelists;
}

UserShareInfoManager::UserShareInfoManager(QObject *parent) : QObject(parent)
{
    QString filePath = "/var/lib/samba/usershares";
    QString usersharesUri = "file://" + filePath;
    m_watcher = std::make_shared<FileWatcher>(usersharesUri);
    QDir dir(filePath);
    QFileInfoList infoList = dir.entryInfoList(QDir::Files);
    for (QFileInfo fileInfo : infoList) {
        m_usersharelists.append(fileInfo.fileName());
    }

    qDebug() << __func__ << __LINE__ << m_usersharelists;
    connect(m_watcher.get(), &FileWatcher::fileCreated, this, [=](QString uri){
        QString name = Peony::FileUtils::urlDecode(uri).split("/").last();
        if (!name.contains(":")) {
            m_usersharelists.append(name);
            m_usersharelists.removeDuplicates();
        }
    });
    connect(m_watcher.get(), &FileWatcher::fileDeleted, this, [=](QString uri){
        QString name = Peony::FileUtils::urlDecode(uri).split("/").last();
        if (!name.contains(":") && m_usersharelists.contains(name)) {
            m_usersharelists.removeOne(name);
        }
    });

    m_watcher->startMonitor();
}

bool UserShareInfoManager::addShareInfo(ShareInfo* shareInfo)
{
    if (nullptr == shareInfo
            || shareInfo->name.isEmpty()
            || shareInfo->originalPath.isEmpty()) {
        return false;
    }

    m_mutex.lock();
    if (m_sharedInfoMap.contains(shareInfo->name)) {
        m_mutex.unlock();
        return false;
    }

    shareInfo->isShared = true;
    m_sharedInfoMap[shareInfo->name] = shareInfo;
    m_mutex.unlock();

    return true;
}

void UserShareInfoManager::removeShareInfo(QString &name)
{
    m_mutex.lock();
    QString originalPath;
    if (m_sharedInfoMap.contains(name)) {
        if (nullptr != m_sharedInfoMap[name])
        {
            originalPath=m_sharedInfoMap[name]->originalPath;
            delete m_sharedInfoMap[name];
        }
        m_sharedInfoMap.remove(name);
    }
    m_mutex.unlock();

    QStringList args;
    args << "usershare" << "delete" << QString("\"%1\"").arg(name);

    bool ret = false;
    exectueCommand (args, &ret);
    Q_EMIT signal_deleteSharedFolder(originalPath, ret);
}

SharedDeleteInfoThread::SharedDeleteInfoThread(const QString uri)
    : m_uri(uri)
{

}

void SharedDeleteInfoThread::run()
{
    QString name = FileUtils::getUriBaseName(m_uri);
    QMutexLocker locker(&m_mutex);
    UserShareInfoManager::getInstance()->removeShareInfo(name);
    qDebug() << __func__ << "name:" << name;

}
