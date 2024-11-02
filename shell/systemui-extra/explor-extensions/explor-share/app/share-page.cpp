/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "share-page.h"
#include "share-properties-page-plugin.h"
#include "emblem-provider.h"
#include "share-emblem-provider.h"
#include "samba-config-interface.h"
#include "samba-config-thread.h"
#include "global-settings.h"
#include "advanced-share-page.h"
#include "pwd.h"

#include <PeonyFileInfoJob>
#include <QUrl>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QButtonGroup>
#include <PeonyFileInfo>
#include <QtConcurrent>
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>

#include <QDebug>
#include <syslog.h>
#include <fcntl.h>
#include <polkit/polkit.h>

#define NAME_LABEL_WIDTH 350

SharePage::SharePage(const QString &uri, QWidget *parent) : PropertiesWindowTabIface(parent)
{
    qDebug() << "=========="<< __func__ <<uri;
    QFuture<void> initFileInfoJob = QtConcurrent::run([=]() {
        m_fileInfo = Peony::FileInfo::fromUri(uri);
        Peony::FileInfoJob *fileInfoJob = new Peony::FileInfoJob(m_fileInfo);
        fileInfoJob->setAutoDelete(true);
        fileInfoJob->querySync();
    });

    m_theFutureWatcher = new QFutureWatcher<void>();
    m_theFutureWatcher->setFuture(initFileInfoJob);

    connect(m_theFutureWatcher, &QFutureWatcher<void>::finished, this, &SharePage::init);
}

void SharePage::init()
{
    if (m_theFutureWatcher)
        delete m_theFutureWatcher;

    m_shareInfo.name = m_fileInfo->displayName();
    m_shareInfo.originalPath = m_fileInfo->filePath();

    const ShareInfo* stmp = UserShareInfoManager::getInstance()->getShareInfo(m_shareInfo.name);
    if (nullptr != stmp) {
        m_shareInfo = stmp;
        m_shareInfo.isShared = true;
    }

    this->setContentsMargins(0, 0, 0, 0);
    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    this->initFloorOne();

    this->addSeparate();

    this->initFloorTwo();

    this->initFloorThree();

    m_layout->addStretch(1);
}

void SharePage::saveAllChange()
{
    if (!m_thisPageChanged)
        return;

    bool checked = m_switchButton->isChecked();

    if (checked) {
        //Check specail symbols
        bool isSpecial = checkSpecialSymbols(m_fileInfo->displayName());
        if (isSpecial) {
            QMessageBox::warning(nullptr, tr("Warning"), tr("The share name must not contain %1, and cannot start with a dash (-) or whitespace, or end with whitespace.").arg("%<>*?|/\\+=;:,\""), QMessageBox::Ok);
            return;
        }
        //Check username and share name
        if (getpwnam(m_fileInfo->displayName().toUtf8().constData())) {
            QMessageBox::warning(nullptr, tr("Warning"), tr("The share name cannot be the same as the current user name"), QMessageBox::Ok);
            return;
        }

        QDir userDir("/var/lib/samba/usershares");
        QFileInfoList fileInfoList = userDir.entryInfoList(QDir::Files);
        for (auto fileInfo : fileInfoList) {
            if (m_fileInfo->displayName().toLower() == fileInfo.fileName()) {
                QFileInfo shareInfo(m_fileInfo->filePath());
                if (fileInfo.owner() != shareInfo.owner()) {
                    QMessageBox::question(nullptr, tr("question"), tr("The share name is already used by other users. You can rename the folder and then set the share."), QMessageBox::Ok);
                    return;
                }
            }
        }

        auto settings = GlobalSettings::getInstance();
        bool runbackend = settings->getInstance()->getValue(RESIDENT_IN_BACKEND).toBool();
        qApp->setQuitOnLastWindowClosed(false);

        //FIXME:暂时不支持自定义共享名 - Currently does not support custom shared name
        m_shareInfo.name = m_fileInfo->displayName();
        m_shareInfo.readOnly = !m_allowGuestModify->isChecked();
        m_shareInfo.allowGuest = m_shareAllowGuestCheckBox->isChecked();
        m_shareInfo.isShared = true;

        QString uri = m_fileInfo->uri();
        if (!m_shareInfo.readOnly || m_usershareAcl.compare("Everyone:F", Qt::CaseInsensitive) == 0) {
            QMessageBox messageBox(this);
            messageBox.setText(tr("The folder is currently shared in non-read-only mode, and setting up the share requires adding other people's write permissions to the current folder?"));
            messageBox.setWindowTitle(tr("question"));
            messageBox.setIcon(QMessageBox::Question);
            QPushButton *addModeButton = messageBox.addButton(tr("Confirm adding permissions"), QMessageBox::YesRole);
            messageBox.addButton(tr("Cancel"), QMessageBox::NoRole);
            messageBox.exec();
            if (messageBox.clickedButton() == addModeButton) {
                g_autoptr(GFile) gFile = g_file_new_for_uri(uri.toUtf8().constData());
                if (gFile) {
                    g_autoptr(GError) gError = NULL;
                    g_autoptr(GFileInfo) gInfo = g_file_query_info(gFile,
                                                                  "unix::mode",
                                                                  G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                                  nullptr,
                                                                  &gError);
                    if (gInfo) {
                        guint32 mode = 0;
                        bool has_unix_mode = g_file_info_has_attribute(gInfo, G_FILE_ATTRIBUTE_UNIX_MODE);
                        if (has_unix_mode) {
                            mode = g_file_info_get_attribute_uint32(gInfo, G_FILE_ATTRIBUTE_UNIX_MODE);
                            auto other_writeable = mode & S_IWOTH;
                            if (!other_writeable) {
                                mode |= S_IWOTH;
                                g_autoptr(GError) gErr = NULL;
                                g_file_set_attribute_uint32(gFile,
                                                     G_FILE_ATTRIBUTE_UNIX_MODE,
                                                     (guint32)mode,
                                                     G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                     nullptr,
                                                     &gErr);
                                if (gErr) {
                                    qApp->setQuitOnLastWindowClosed(!runbackend);
                                    QMessageBox::warning(nullptr, tr("Warning"), gErr->message, QMessageBox::Ok);
                                    return;
                                }
                            }
                        } else {
                            return;
                        }
                    } else if (gError){
                        qApp->setQuitOnLastWindowClosed(!runbackend);
                        QMessageBox::warning(nullptr, tr("Warning"), gError->message, QMessageBox::Ok);
                        return;
                    }
                } else {
                    return;
                }
            } else {
                return;
            }
        }

        if (m_shareInfo.allowGuest) {
            QMessageBox mBox(this);
            mBox.setText(tr("The folder is currently shared with anonymous access set, do I need to automatically add executable permissions for the current file and all other members of the parent directory?"));
            mBox.setWindowTitle(tr("question"));
            mBox.setIcon(QMessageBox::Question);
            QPushButton *addModeBtn = mBox.addButton(tr("Confirm adding permissions"), QMessageBox::YesRole);
            mBox.addButton(tr("Cancel"), QMessageBox::NoRole);
            mBox.exec();
            if (mBox.clickedButton() == addModeBtn) {
                while (!uri.isEmpty() && uri != "") {
                    g_autoptr(GFile) file = g_file_new_for_uri(uri.toUtf8().constData());
                    if (file) {
                        g_autoptr(GError) error = NULL;
                        g_autoptr(GFileInfo) info = g_file_query_info(file,
                                                                      "unix::mode",
                                                                      G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                                      nullptr,
                                                                      &error);
                        if (info) {
                            guint32 mode = 0;
                            m_has_unix_mode = g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_UNIX_MODE);
                            if (m_has_unix_mode) {
                                mode = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_UNIX_MODE);
                                auto other_executable = mode & S_IXOTH;
                                if (!other_executable) {
                                    mode |= S_IXOTH;
                                    g_autoptr(GError) err = NULL;
                                    g_file_set_attribute_uint32(file,
                                                         G_FILE_ATTRIBUTE_UNIX_MODE,
                                                         (guint32)mode,
                                                         G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                         nullptr,
                                                         &err);
                                    if (err) {
                                        qApp->setQuitOnLastWindowClosed(!runbackend);
                                        QMessageBox::warning(nullptr, tr("Warning"), err->message, QMessageBox::Ok);
                                        return;
                                    }
                                }
                                uri = FileUtils::getParentUri(uri);
                            } else {
                                return;
                            }
                        } else if (error) {
                            qApp->setQuitOnLastWindowClosed(!runbackend);
                            QMessageBox::warning(nullptr, tr("Warning"), error->message, QMessageBox::Ok);
                            return;
                        }
                    } else {
                        return;
                    }
                }
            } else {
                return;
            }
        }

        //handle:
//        SambaConfigInterface si(DBUS_NAME, DBUS_PATH, QDBusConnection::systemBus());

//        if (si.init(g_get_user_name(), getpid(), getuid())) {
//            if (!si.hasPasswd()) {
//                bool ok = false;
//                QString text = QInputDialog::getText(nullptr, tr("Set Password"), tr("Password:"), QLineEdit::Normal, "", &ok);
//                if (ok && !text.isNull() && !text.isEmpty()) {
//                    if (!si.setPasswd(text)) {
//                        QMessageBox::warning(nullptr, tr("Warning"), tr("Samba set password failed, Please re-enter!"));
//                        m_ret = true;
//                    }
//                }
//            }
//        } else {
//            QMessageBox::warning(nullptr, tr("Warning"), tr("Samba service init failed!"), QMessageBox::Ok);
//            m_ret = true;
//        }

//        si.finished();
//        if (m_ret) {
//            return;
//        }

//        UserShareInfoManager::getInstance()->updateShareInfo(m_shareInfo);

        if (!this->checkAuthorization())
            return;

        qDebug()<< __func__ << __LINE__ << m_shareInfo.name << m_shareInfo.isShared << m_shareInfo.originalPath << m_shareInfo.readOnly;
        SambaConfigThread* thread = new SambaConfigThread(m_shareInfo, m_fileInfo->uri(), m_usershareAcl);
        connect(thread, &SambaConfigThread::isInitSignal, UserShareInfoManager::getInstance(), [=](bool initRet){
            qDebug()<< __func__ << __LINE__ << initRet;
            if (!initRet) {
                qApp->setQuitOnLastWindowClosed(!runbackend);
                QMessageBox::warning(nullptr, tr("Warning"), tr("Shared configuration service exception, please confirm if there is an ongoing shared configuration operation, or please reset the share!"), QMessageBox::Ok);
                return;
            }
        }, Qt::BlockingQueuedConnection);

        connect(thread, &SambaConfigThread::isHasPasswdSignal, UserShareInfoManager::getInstance(), [=](bool hasPasswdRet, QString &passwd){
            qDebug()<< __func__ << __LINE__ << hasPasswdRet;
            if (!hasPasswdRet) {
                bool ok = false;
                QInputDialog dlg;
                dlg.setLabelText(tr("Samba password:"));
                dlg.setTextEchoMode(QLineEdit::Password);
                dlg.setWindowTitle(tr("Samba set user password"));
                dlg.setWindowIcon(QIcon::fromTheme("system-file-manager"));
                dlg.setFixedSize(470,150);
                ok = dlg.exec();
                QString text = dlg.textValue();
                if (ok && !text.isNull() && !text.isEmpty()) {
                    passwd = text;
                } else {
                    qApp->setQuitOnLastWindowClosed(!runbackend);
                }
            } else {
                if (UserShareInfoManager::getInstance()->updateShareInfo(*(thread->getShareInfo()), thread->getAcl())) {
                    auto fileInfo = Peony::FileXattrInfo::fromUri(thread->getUri());
                    if(fileInfo){
                        fileInfo->setXattrInfoString(SHARE_EMBLEMS,"emblem-shared", true);
                        EmblemProviderManager::getInstance()->queryAsync(thread->getUri());
                        qApp->setQuitOnLastWindowClosed(!runbackend);
                    }
                }
            }
        }, Qt::BlockingQueuedConnection);
        connect(thread, &SambaConfigThread::isSetPasswdSignal, UserShareInfoManager::getInstance(), [=](bool setPasswdRet){
            qDebug()<< __func__ << __LINE__ << setPasswdRet;
            if (!setPasswdRet) {
                qApp->setQuitOnLastWindowClosed(!runbackend);
                QMessageBox::warning(nullptr, tr("Warning"), tr("Samba set password failed, Please re-enter!"));
                return;
            } else {
                if (UserShareInfoManager::getInstance()->updateShareInfo(*(thread->getShareInfo()), thread->getAcl())) {
                    auto fileInfo = Peony::FileXattrInfo::fromUri(thread->getUri());
                    if(fileInfo){
                        fileInfo->setXattrInfoString(SHARE_EMBLEMS,"emblem-shared", true);
                        EmblemProviderManager::getInstance()->queryAsync(thread->getUri());
                        qApp->setQuitOnLastWindowClosed(!runbackend);
                    }
                }
            }
        }, Qt::BlockingQueuedConnection);
        connect(thread, &SambaConfigThread::finished, thread, &SambaConfigThread::deleteLater);
        thread->start();
    } else {
        auto fileInfo = Peony::FileXattrInfo::fromUri(m_fileInfo->uri());
        if(fileInfo && !fileInfo->getXattrInfoString(SHARE_EMBLEMS).isEmpty()){
            fileInfo->removeXattrInfo(SHARE_EMBLEMS);
            EmblemProviderManager::getInstance()->queryAsync(m_fileInfo->uri());
        }
        //FIXME:需要先判断存在共享吗 - Need to determine whether there is sharing first
        UserShareInfoManager::getInstance()->removeShareInfoAcl(m_shareInfo.name);
        bool ret;
        QStringList args;
        args << "setfacl" << "-b" << QString("\"%1\"").arg(m_fileInfo->filePath());
        UserShareInfoManager::exectueSetAclCommand(args, &ret);
    }
}

bool SharePage::checkAuthorization()
{
    bool ret = false;
    g_autoptr(GError) error = NULL;
    PolkitAuthority* mAuth = polkit_authority_get_sync(NULL, &error);
    if (error) {
        qWarning() << error->message;
        return ret;
    }

    int pid = getpid();
    int uid = getuid();
    PolkitSubject* proj = polkit_unix_process_new_for_owner (pid, 0, uid);

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

bool SharePage::checkSpecialSymbols(const QString &displayName)
{
    if (displayName.startsWith(" ") || displayName.startsWith("-") || displayName.endsWith(" ")) {
        return true;
    }

    QRegExp gSpecialSymbols("[\\\\/:\\*\\?\\<>\\|\\+\\=\\,\\;\\%]");
    if (displayName.contains(gSpecialSymbols)) {
        return true;
    }

    return false;
}

void SharePage::initFloorOne()
{
    QFrame * floor1 = new QFrame(this);
    floor1->setContentsMargins(0, 0, 0, 0);
    floor1->setMinimumHeight(90);

    QHBoxLayout * layout1 = new QHBoxLayout(floor1);
    layout1->setContentsMargins(24, 24, 24, 16);
    layout1->setSpacing(16);
    layout1->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    floor1->setLayout(layout1);

    m_iconLabel = new QLabel(floor1);
    QIcon icon = QIcon::fromTheme(m_fileInfo.get()->iconName());
    m_iconLabel->setFixedSize(QSize(64, 64));
    m_iconLabel->setPixmap(icon.pixmap(QSize(64, 64)));

    layout1->addWidget(m_iconLabel);

    QVBoxLayout * vBoxLayout = new QVBoxLayout(floor1);

    m_folderName = new QLabel(floor1);
    m_sharedState = new QLabel(floor1);

    //fix bug:#82320
    if (QRegExp("^file:///data/usershare(/{,1})$").exactMatch(m_fileInfo->uri())) {
        m_folderName->setText(tr("usershare"));
    } else {
        QFontMetrics fontWidth(m_folderName->font());
        QString elideNote = fontWidth.elidedText(m_fileInfo.get()->displayName(), Qt::ElideMiddle, NAME_LABEL_WIDTH);
        m_folderName->setText(elideNote);
        m_folderName->setToolTip(m_fileInfo.get()->displayName());
    }
    m_sharedState->setText(m_shareInfo.isShared ? tr("share this folder") : tr("don`t share this folder"));

    vBoxLayout->addStretch(1);
    vBoxLayout->addWidget(m_folderName);
    vBoxLayout->addWidget(m_sharedState);
    vBoxLayout->addStretch(1);

    layout1->addLayout(vBoxLayout);
    layout1->addStretch(1);

    m_layout->addWidget(floor1);
}

void SharePage::initFloorTwo()
{
    QFrame * floor2 = new QFrame(this);

    QHBoxLayout *layout2 = new QHBoxLayout(floor2);
    layout2->setAlignment(Qt::AlignLeft);
    layout2->setContentsMargins(24, 16, 24, 0);
    layout2->setSpacing(16);

    floor2->setLayout(layout2);

    bool shared = m_shareInfo.isShared;

    m_switchButton = new kdk::KSwitchButton(floor2);
    m_switchButton->setChecked(shared);

    layout2->addWidget(new QLabel(tr("Share folder"), floor2));
    layout2->addWidget(m_switchButton);

    m_layout->addWidget(floor2);

    connect(m_switchButton, &kdk::KSwitchButton::stateChanged, this, [=](bool checked) {
        this->thisPageChanged();
        if (checked) {
            //FIXME:暂时不支持自定义共享名 - Currently does not support custom shared name
            m_shareInfo.name = m_fileInfo->displayName();
            m_shareInfo.readOnly = !m_allowGuestModify->isChecked();
            m_shareInfo.allowGuest = m_shareAllowGuestCheckBox->isChecked();

            if (m_floor3)
                m_floor3->setVisible(true);

            m_sharedState->setText(tr("share this folder"));
            m_shareInfo.isShared = true;

        } else {
            if (m_floor3)
                m_floor3->setVisible(false);

            m_sharedState->setText(tr("don`t share this folder"));
            m_shareInfo.isShared = false;
        }
    });
}

void SharePage::initFloorThree()
{
    m_floor3 = new QFrame(this);

    QVBoxLayout * layout3 = new QVBoxLayout(m_floor3);
    layout3->setContentsMargins(24, 24, 24, 0);
    layout3->setSpacing(0);

    m_floor3->setLayout(layout3);

    QFormLayout *formLayout = new QFormLayout(m_floor3);
    formLayout->setVerticalSpacing(24);

    m_shareNameEdit = new QLineEdit(m_floor3);
    m_shareNameEdit->setMaxLength(32);
    m_shareNameEdit->setEnabled(false);
    m_shareNameEdit->setText(m_shareInfo.name);

    m_commentEdit = new QLineEdit(m_floor3);
    m_commentEdit->setMaxLength(64);
    m_commentEdit->setText(m_shareInfo.comment);

    formLayout->addRow(tr("Share name:"), m_shareNameEdit);
    formLayout->addRow(tr("Comment:"), m_commentEdit);

    layout3->addLayout(formLayout);

    m_allowGuestModify = new QCheckBox(tr("Allow others to create and delete files"), this);
    m_allowGuestModify->setChecked(!m_shareInfo.readOnly);

    m_shareAllowGuestCheckBox = new QCheckBox(tr("Allow Anonymous"));
    m_shareAllowGuestCheckBox->setChecked(m_shareInfo.allowGuest);

    layout3->addSpacing(24);
    layout3->addWidget(m_allowGuestModify);
    layout3->addWidget(m_shareAllowGuestCheckBox);

    QHBoxLayout *hBoxLayout = new QHBoxLayout(m_floor3);

    m_advanceShareButton = new QPushButton(tr("Advanced Sharing"), this);
    hBoxLayout->addWidget(m_advanceShareButton);
    hBoxLayout->addStretch(2);

    layout3->addLayout(hBoxLayout);

    m_floor3->setVisible(m_shareInfo.isShared);

    m_layout->addWidget(m_floor3);

    connect(m_commentEdit, &QLineEdit::textChanged, this, [=]() {
        m_shareInfo.comment = m_commentEdit->text();
        this->thisPageChanged();
    });

    connect(m_allowGuestModify, &QCheckBox::clicked, this, [=](bool checked) {
        m_shareInfo.readOnly = checked;
        this->thisPageChanged();
    });

    connect(m_shareAllowGuestCheckBox, &QCheckBox::clicked, this, [=](bool checked) {
        m_shareInfo.allowGuest = checked;
        this->thisPageChanged();
    });

    connect(m_advanceShareButton, &QPushButton::clicked, this, [=](){
        if(checkAuthorization()) {
            this->thisPageChanged();
            AdvancedSharePage *page = new AdvancedSharePage(m_fileInfo->uri(), m_userInfoCache);
            connect(page, &AdvancedSharePage::getUserInfo, this, [=](QString acl, QMap<QString, QString>& userInfo){
               m_usershareAcl = acl;
               m_userInfoCache = userInfo;
            });
            page->show();
        }
    });
}
