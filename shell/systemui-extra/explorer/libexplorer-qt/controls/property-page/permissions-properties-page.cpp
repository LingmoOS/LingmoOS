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

#include "permissions-properties-page.h"

#include "linux-pwd-helper.h"
#include "file-watcher.h"
#include "file-info.h"
#include "file-info-job.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QProcess>
#include <QPushButton>
#include <QCheckBox>

#include <QUrl>
#include <QStandardPaths>

#include <QDebug>
#include <QFileInfo>
#include <QGSettings>

using namespace Peony;

#define OWNER 0
#define GROUP 1
#define OTHERS 2
#define USER 0

#define READABLE 2
#define WRITEABLE 3
#define EXECUTEABLE 4
//440 - 16 - 16 = 408 ,右侧有字符被遮挡，再减去6px -_-;
#define TARGET_LABEL_WIDTH 402

PermissionsPropertiesPage::PermissionsPropertiesPage(const QStringList &uris, QWidget *parent) : PropertiesWindowTabIface(parent)
{
    m_uri = uris.first();
    QUrl url(m_uri);

    //note:请查看：BasicPropertiesPage::getFIleInfo(QString uri) - Look BasicPropertiesPage::getFIleInfo(QString uri)
    if (m_uri.startsWith("favorite://")) {
        m_uri = "file://" + url.path();
        url = QUrl(m_uri);
    }

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 0, 8, 0);
    m_layout->setSpacing(0);
    this->setLayout(m_layout);

    m_label = new QLabel(this);

    QString str = tr("Target: %1").arg(url.path());
    updateLabelShow(str);

    m_label->setMinimumHeight(60);
    m_label->setContentsMargins(16, 0, 16, 0);

    m_layout->addWidget(m_label);

    m_message = new QLabel(this);
    m_message->setAlignment(Qt::AlignCenter);

    this->initTableWidget();

    m_layout->addWidget(m_message);
    m_message->setVisible(false);

    m_watcher = std::make_shared<FileWatcher>(m_uri);
    connect(m_watcher.get(), &FileWatcher::locationChanged, this, &PermissionsPropertiesPage::queryPermissionsAsync);
    connect(this, &PermissionsPropertiesPage::checkBoxChanged, this, &PermissionsPropertiesPage::changePermission);

    queryPermissionsAsync(nullptr, m_uri);

    this->addAdvancedLayout();

    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings *gSetting = new QGSettings("org.lingmo.style", QByteArray(), this);
        connect(gSetting, &QGSettings::changed, this, [=](const QString &key) {
            if ("systemFontSize" == key) {
                updateLabelShow(str);
            }
        });
    }
}

PermissionsPropertiesPage::~PermissionsPropertiesPage()
{

}

void PermissionsPropertiesPage::initTableWidget()
{
    m_table = new QTableWidget(this);
    m_table->setContentsMargins(0, 0, 0, 0);
    m_table->setRowCount(4);
    //属性窗口重构，可读、可写、可执行 改成 读写、只读
    m_table->setColumnCount(3);
    m_table->verticalHeader()->setVisible(false);
    m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_table->horizontalHeader()->setFrameShape(QFrame::NoFrame);
    m_table->setFrameShape(QFrame::NoFrame);
    m_table->horizontalHeader()->setSelectionMode(QTableWidget::NoSelection);
    m_table->setSelectionMode(QTableWidget::NoSelection);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setShowGrid(false);

    m_table->horizontalHeader()->setMinimumHeight(34);
    m_table->rowHeight(34);

    m_table->setAlternatingRowColors(true);

    auto l = QStringList();
    //属性窗口重构，类型、可读、可写、可执行 改成 读写、只读
    l<<tr("Group or User")<<tr("Read and Write")<<tr("Readonly");
    m_table->setHorizontalHeaderLabels(l);
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);
    //开启手动设置宽度 - Enable manual width setting
    m_table->horizontalHeader()->setMinimumSectionSize(30);
    m_table->horizontalHeader()->setMaximumSectionSize(400);

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    //m_table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);

    m_table->setColumnWidth(0, 150);
    m_table->setColumnWidth(1, 75);
    m_table->setColumnWidth(2, 120);
    m_layout->addWidget(m_table);   
}

void PermissionsPropertiesPage::queryPermissionsAsync(const QString &, const QString &uri)
{
    m_uri = uri;
    QUrl url = m_uri;
    m_label->setText(m_label->fontMetrics().elidedText(tr("Target: %1").arg(url.path()), Qt::ElideMiddle,TARGET_LABEL_WIDTH));
    m_table->setEnabled(false);

    GFile *file = g_file_new_for_uri(m_uri.toUtf8().constData());
    g_file_query_info_async(file,
                            "owner::*," "access::*," "unix::mode",
                            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                            0,
                            nullptr,
                            GAsyncReadyCallback(async_query_permisson_callback),
                            this);
    g_object_unref(file);
}

GAsyncReadyCallback PermissionsPropertiesPage::async_query_permisson_callback(GObject *obj, GAsyncResult *res, PermissionsPropertiesPage *p_this)
{
    GError *err = nullptr;
    auto info = g_file_query_info_finish(G_FILE(obj), res, &err);

    if (!info) {
        if (p_this) {
            p_this->m_table->setVisible(false);
            p_this->m_message->setText(tr("Can not get the permission info."));
            p_this->m_message->setVisible(true);
        }
    }

    if (err) {
        qDebug()<<err->message;
        if (p_this) {
            p_this->m_table->setVisible(false);
            p_this->m_message->setText(tr("Can not get the permission info."));
            p_this->m_message->setVisible(true);
        }
        g_error_free(err);
    }

    if (info) {
        if (p_this) {
            bool enable = true;
            auto table = p_this->m_table;
            auto user = g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_OWNER_USER);
            //auto owner = g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_OWNER_USER_REAL);
            QString userString = user;
            QString groupName = g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_OWNER_GROUP);
            QString userNameDisplayString = user;

            bool current_user_readable = g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
            bool current_user_writeable = g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
            bool current_user_executable = g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);

            p_this->m_has_unix_mode = g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_UNIX_MODE);
            guint32 mode = 0;
            if (p_this->m_has_unix_mode)
                mode = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_UNIX_MODE);

            auto owner_readable  = mode & S_IRUSR;
            auto owner_writeable = mode & S_IWUSR;
            auto owner_executable = mode & S_IXUSR;

            //read
            p_this->m_permissions[0][0] = owner_readable;
            //write
            p_this->m_permissions[0][1] = owner_writeable;
            //executable
            p_this->m_permissions[0][2] = owner_executable;

            auto group_readable  = mode & S_IRGRP;
            auto group_writeable = mode & S_IWGRP;
            auto group_executable = mode & S_IXGRP;

            p_this->m_permissions[1][0] = group_readable;
            p_this->m_permissions[1][1] = group_writeable;
            p_this->m_permissions[1][2] = group_executable;

            auto other_readable  = mode & S_IROTH;
            auto other_writeable = mode & S_IWOTH;
            auto other_executable = mode & S_IXOTH;

            p_this->m_permissions[2][0] = other_readable;
            p_this->m_permissions[2][1] = other_writeable;
            p_this->m_permissions[2][2] = other_executable;

            qDebug()<<current_user_readable<<current_user_writeable<<current_user_executable;

            uid_t uid = geteuid();
            struct passwd *pw = getpwuid(uid);
            if( pw ) {
                bool isSelf = false;
                bool isSameGroup = false;
                auto username = pw->pw_name;
                if (userString == username) {
                    userNameDisplayString += tr("(Current User)");
                    isSelf = true;
                }
                /*
                if (userNameDisplayString.isEmpty())
                    userNameDisplayString = tr("Unkwon");
                if (groupName == pw->pw_gecos)
                    isSameGroup = true;
                if (groupName.isEmpty())
                    groupName = tr("Unkwon");
                    */

                if (!isSelf && !isSameGroup) {
                    qDebug()<<"the uid not permit";
                    enable = false;
                }

                if (pw->pw_uid == 0) {
                    QFileInfo file("/usr/sbin/security-switch");
                    if(file.exists() == true) {
                        QProcess shProcess;
                        shProcess.start("/usr/sbin/security-switch --get");
                        if (!shProcess.waitForStarted()) {
                            qDebug()<<"wait get security state start timeout";
                        } else {
                            if (!shProcess.waitForFinished()) {
                                qDebug()<<"wait get security state finshed timeout";
                            } else {
                                QString secState = shProcess.readAllStandardOutput();
                                qDebug()<<"security-switch get test "<< secState;
                                if (secState.contains("strict")) {
                                    qDebug()<<"now it is in strict mode, so root is not super";
                                } else {
                                    qDebug()<<"pw uid is 0, it is super";
                                    enable = true;
                                }
                            }
                        }
                    } else {
                        qDebug()<<"security-switch is not support, so it is super";
                        enable = true;
                    }
                   /*
                    if (!kysec_is_disabled() && kysec_get_3adm_status()) {
                        qDebug()<<"now it is in strict mode, so root is not super";
                    } else {
                        qDebug()<<"pw uid is 0, it is super";
                        enable = true;
                    }*/
                }

            } else {
                enable = false;
            }

            if (enable) {
                table->setRowCount(3);
                //更新表格选中情况
                p_this->updateCheckBox();

                table->setItem(0, 0, nullptr);
                QTableWidgetItem* itemR0C0 = new QTableWidgetItem(QIcon::fromTheme("emblem-personal"), userNameDisplayString);
                table->setItem(0, 0, itemR0C0);
                itemR0C0->setToolTip(userNameDisplayString);

                table->setItem(1, 0, nullptr);
                QTableWidgetItem* itemR1C0 = new QTableWidgetItem(QIcon::fromTheme("emblem-people"), groupName);
                table->setItem(1, 0, itemR1C0);
                itemR1C0->setToolTip(groupName);

                table->setItem(2, 0, nullptr);
                QTableWidgetItem* itemR2C0 = new QTableWidgetItem(QIcon::fromTheme("emblem-people"), tr("Others"));
                table->setItem(2, 0, itemR2C0);
                itemR2C0->setToolTip(tr("Others"));

                table->showRow(0);
                table->showRow(1);
                table->showRow(2);
            } else {
                p_this->m_message->setText(tr("You can not change the access of this file."));
                p_this->m_message->show();
                table->setRowCount(1);

                QTableWidgetItem *itemR0C0 = new QTableWidgetItem(QIcon::fromTheme("emblem-personal"), tr("Current User"));
                table->setItem(0, 0, nullptr);
                table->setItem(0, 0, itemR0C0);

                for (int i = 0; i < 2; i++) {
                    table->setCellWidget(0, i + 1, nullptr);
                    QWidget *w = new QWidget(table);
                    QHBoxLayout *l = new QHBoxLayout(w);
                    l->setMargin(0);
                    w->setLayout(l);
                    l->setAlignment(Qt::AlignCenter);
                    QRadioButton* radioButton = new QRadioButton(w);
                    l->addWidget(radioButton);
                    table->setCellWidget(0, i + 1, w);

                    switch (i) {
                    case 0:
                        radioButton->setChecked(current_user_writeable&&current_user_readable);
                        break;
                    case 1:
                        radioButton->setChecked((!current_user_writeable)&&current_user_readable);
                        break;
                    }
                }
            }

            table->setEnabled(enable);
            //防止误修改
            p_this->m_enable = enable;
        }

        g_object_unref(info);
    }
    return nullptr;
}

void PermissionsPropertiesPage::changePermission(int row, int column, bool checked)
{
    if(!m_enable)
        return;

    bool havePermission = false;
    if(0 == column)
    {
        havePermission = !(m_permissions[row][1]&&m_permissions[row][0]);
        havePermission = checked? havePermission:checked;
        m_permissions[row][0] = havePermission;
        m_permissions[row][1] = havePermission;
    }
    else
    {
        havePermission = !((!m_permissions[row][1])&&m_permissions[row][0]);
        havePermission = checked? havePermission:checked;
        m_permissions[row][0] = havePermission;
        m_permissions[row][1] = false;
    }
    this->thisPageChanged();

    //this->updateCheckBox();
}

/*!
 * update file ermissions
 * \brief PermissionsPropertiesPage::savePermissions
 */
void PermissionsPropertiesPage::savePermissions()
{
    /*!
      \bug
      even though directory know the file's attributes have been changed, and
      model request updated the data, the view doesn't paint the current emblems correctly.
    */
    //FIXME: should use g_file_set_attribute() with mode info?
    if(!m_enable)
        return;

    mode_t mod = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            bool b = m_permissions[i][j];
            if (b) {
                int rc = i*10 + j;
                switch (rc) {
                case 0: {
                    mod |= S_IRUSR;
                    break;
                }
                case 1: {
                    mod |= S_IWUSR;
                    break;
                }
                case 2: {
                    mod |= S_IXUSR;
                    break;
                }

                case 10: {
                    mod |= S_IRGRP;
                    break;
                }
                case 11: {
                    mod |= S_IWGRP;
                    break;
                }
                case 12: {
                    mod |= S_IXGRP;
                    break;
                }
                case 20: {
                    mod |= S_IROTH;
                    break;
                }
                case 21: {
                    mod |= S_IWOTH;
                    break;
                }
                case 22: {
                    mod |= S_IXOTH;
                    break;
                }
                }
            }
        }
    }

    if (m_has_unix_mode) {
        g_autoptr(GFile) pfile = g_file_new_for_uri(m_uri.toUtf8().constData());
        g_file_set_attribute_uint32(pfile, G_FILE_ATTRIBUTE_UNIX_MODE, (guint32)mod, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
    }

    auto fileInfo = FileInfo::fromUri(m_uri);
    FileInfoJob *job = new FileInfoJob(fileInfo);
    job->setAutoDelete(true);
    job->querySync();
}

void PermissionsPropertiesPage::saveAllChange()
{
    if(this->m_thisPageChanged)
        this->savePermissions();

    qDebug() << "PermissionsPropertiesPage::saveAllChange()" << this->m_thisPageChanged;
}

void PermissionsPropertiesPage::thisPageChanged()
{
    this->m_thisPageChanged = true;
}

void PermissionsPropertiesPage::updateCheckBox()
{
    for (int i = 0; i < 3; i++) {
        //修改文件管理器属性界面的权限页，设置权限为读写，可读
        QButtonGroup* permissionsBtGroup = new QButtonGroup(this);
        for (int j = 0; j < 2; j++) {
            m_table->setCellWidget(i, j + 1, nullptr);
            QWidget *w = new QWidget(m_table);
            QHBoxLayout *l = new QHBoxLayout(w);
            l->setMargin(0);
            w->setLayout(l);
            l->setAlignment(Qt::AlignCenter);
            QRadioButton* radioButton = new QRadioButton(w);
            l->addWidget(radioButton);
            permissionsBtGroup->addButton(radioButton);
            permissionsBtGroup->setId(radioButton,j);
            m_table->setCellWidget(i, j + 1, w);
            bool checkType = false;
            if(0 == j)
            {
                checkType = this->m_permissions[i][1]&&this->m_permissions[i][0];
            }
            else
            {
                checkType = (!this->m_permissions[i][1])&&this->m_permissions[i][0];
            }
            radioButton->setAutoExclusive(false);
            radioButton->setChecked(checkType);
            radioButton->setAutoExclusive(true);

            //disable home path
            bool check_enable = true;
            bool check_enable_filesafe = true;
            QString uri = m_uri;

            if(uri.startsWith("filesafe:///")){
                QStringList list = uri.split("/");
                if(list.size()==4){
                    check_enable = false;
                }

                if(list.size()>=5){
                    check_enable_filesafe = false;
                }
            }

            QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            if (this->m_uri == homeUri || !check_enable || (!check_enable_filesafe && i > 0))
                radioButton->setDisabled(true);
            else
                radioButton->setDisabled(false);

        }
        connect(permissionsBtGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [=](int id){
            if (i == 1) {
                auto info = FileInfo::fromUri(m_uri);
                QStringList args;
                bool ret;
                args << "getfacl" << "-p" << QString("\"%1\"").arg(info->filePath());
                QString acl = UserShareInfoManager::getInstance()->exectueSetAclCommand(args, &ret);
                if (!ret && !acl.isEmpty()) {
                    this->close();
                    return;
                }
                if (acl.count("user:") >= 2 && !m_isShow) {
                    m_isShow = true;
                    auto res = QMessageBox::question(nullptr, tr("Permissions modify tip"), tr("The current file or folder has already set ACL permissions. Modifying user group permissions may cause the permissions set in ACL to be unusable. Do you want to continue modifying user group permissions?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    if (QMessageBox::No == res) {
                        //按钮编号只有0-1，所以使用!id反转选择
                        auto checkbox = permissionsBtGroup->button(!id);
                        checkbox->setChecked(!checkbox->isChecked());
                        return;
                    }
                }
            }
            this->checkBoxChanged(i, id, permissionsBtGroup->button(id)->isChecked());
        });
    }
}

void PermissionsPropertiesPage::addAdvancedLayout()
{
    QHBoxLayout *hboxLayout = new QHBoxLayout();
    hboxLayout->setContentsMargins(16, 16, 16, 16);
    m_advancedBtn = new QPushButton(tr("Permission refinement settings"));
    m_layout->addWidget(m_advancedBtn);

    if (m_uri.startsWith("filesafe://")
            || m_uri.startsWith("smb://")
            || m_uri.startsWith("sftp://")
            || m_uri.startsWith("ftp://")
            || m_uri.startsWith("kmre://")) {
        m_advancedBtn->setVisible(false);
    }

    connect(m_advancedBtn, &QPushButton::clicked, this, [=](){

        auto info = FileInfo::fromUri(m_uri);
        auto displayname = info->displayName();
        bool isAdvancedShare = UserShareInfoManager::getInstance()->checkDirAdvancedShare(displayname);
        AdvancedPermissionsPage *page = new AdvancedPermissionsPage(m_uri);
        connect(page, &AdvancedPermissionsPage::updatePermissions, this, [=](){
            queryPermissionsAsync(nullptr, m_uri);
        });
        if (isAdvancedShare) {
            auto result = QMessageBox::question(nullptr, tr("Permission refinement settings"),
                                                tr("The current user has set advanced sharing. If you still need to modify permissions, advanced sharing may not be available. Do you want to continue setting?"),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
            if (QMessageBox::Yes == result) {
                page->show();
            }
        } else {
            page->show();
        }

    });
    hboxLayout->addWidget(m_advancedBtn);
    hboxLayout->addStretch(2);
    m_layout->addLayout(hboxLayout);
}

void PermissionsPropertiesPage::updateLabelShow(const QString &str)
{
    int fontSize = m_label->fontMetrics().width(str);
    QString tmp = str;

    if(fontSize > TARGET_LABEL_WIDTH) {
        m_label->setToolTip(str);
        tmp = m_label->fontMetrics().elidedText(str, Qt::ElideMiddle, TARGET_LABEL_WIDTH);
    }
    m_label->setText(tmp);
}

QWidget *PermissionsPropertiesPage::createCellWidget(QWidget *parent, QIcon icon, QString text)
{
    QWidget *widget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->setContentsMargins(22, 0, 0, 0);
    //组件间距 - Widget spacing
    layout->setSpacing(9);

    QPushButton *cellIcon = new QPushButton(widget);
    cellIcon->setStyleSheet("QPushButton{"
                        "border-radius: 8px; "
                        "background-color: transparent;"
                        "max-width:16px;"
                        "max-height:16px;"
                        "min-width:16px;"
                        "min-height:16px;"
                        "}");
    cellIcon->setEnabled(false);

    cellIcon->setIcon(icon);
    cellIcon->setIconSize(QSize(16, 16));

    layout->addWidget(cellIcon);

    QLabel *label = new QLabel(widget);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFontMetrics fontMetrics = label->fontMetrics();
    int fontSize = fontMetrics.width(text);
    QString str = text;
    //widget宽度200px;设计稿在左边空出22px;icon宽度16px，icon右侧9px;
    //200-22-16-9 = 153 , widget：剩下的3px给右侧留空 -_-；
    if(fontSize > 150) {
        widget->setToolTip(text);
        str = fontMetrics.elidedText(text, Qt::ElideRight, 150);
    }
    label->setText(str);

    layout->addWidget(label);

    return widget;
}

AdvancedPermissionsPage::AdvancedPermissionsPage(const QString &uri, QWidget *parent)
{
    m_uri = uri;
    this->init();
}

AdvancedPermissionsPage::~AdvancedPermissionsPage()
{

}

void AdvancedPermissionsPage::init()
{
    this->setWindowTitle(tr("Permission refinement settings"));
    this->setWindowIcon(QIcon::fromTheme("system-file-manager"));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setFixedSize(460, 600);
    this->setContentsMargins(0, 10, 0, 0);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint & ~Qt::WindowSystemMenuHint);
    this->setWindowModality(Qt::ApplicationModal);

    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    this->getUserInfo();

    this->initFloorOne();
    this->initFloorTwo();
    this->addSeparate();
    this->initFloorThree();
    this->addSeparate();
    this->initFloorFour();
    this->initCheckState();

    connect(m_tabWidget, &QTableWidget::cellEntered, this, &AdvancedPermissionsPage::updateDelAclBtn);
    connect(m_tabWidget, &QTableWidget::cellClicked, this, &AdvancedPermissionsPage::updateDelAclBtn);

    connect(m_listWidget, &QListWidget::currentTextChanged, this, [=](QString currentIndex){
       if (!m_userInfo.contains(currentIndex)) {
           m_addUserBtn->setEnabled(true);
       } else {
           m_addUserBtn->setEnabled(false);
       }
       QFontMetrics fontWidth(m_listLabel->font());
       QString elideNote = fontWidth.elidedText(currentIndex, Qt::ElideMiddle, 300);
       m_listLabel->setText(elideNote);
       m_listLabel->setToolTip(currentIndex);
    });

    connect(m_addUserBtn, &QPushButton::clicked, this, [=](){
        QListWidgetItem *item = m_listWidget->currentItem();
        if (nullptr != item) {
            QString name = item->text();
            //增加用户默认acl
            m_mutex.lock();
            m_userInfo.insert(name, "rwx");
            m_mutex.unlock();

            m_addUserBtn->setEnabled(false);
            m_inheritsBox->setEnabled(true);

            int rowCount = m_tabWidget->rowCount();
            m_tabWidget->insertRow(rowCount);
            QTableWidgetItem* itemC = new QTableWidgetItem(name);
            itemC->setFlags(itemC->flags() | Qt::ItemIsSelectable);
            itemC->setToolTip(name);
            m_tabWidget->setItem(rowCount, 0, itemC);
            for (int i = 1; i < 4; ++i) {
                m_tabWidget->setCellWidget(rowCount, i, nullptr);
                QWidget *w = new QWidget(m_tabWidget);
                QHBoxLayout *l = new QHBoxLayout(w);
                l->setMargin(0);
                w->setLayout(l);
                l->setAlignment(Qt::AlignCenter);
                auto checkbox = new QCheckBox(w);
                bool check = updateCheckBox(i, name);
                checkbox->setChecked(check);
                l->addWidget(checkbox);
                m_tabWidget->setCellWidget(rowCount, i, w);
            }
        }
    });

    connect(m_delUserBtn, &QPushButton::clicked, this, [=](){
        QTableWidgetItem *item = m_tabWidget->currentItem();
        if (nullptr != item) {
            QString name = item->text();

            if (m_userInfo.contains(name)) {
                m_mutex.lock();
                m_userInfo.remove(name);
                m_mutex.unlock();

                int currentRow = m_tabWidget->currentRow();
                m_tabWidget->removeRow(currentRow);
                if (m_tabWidget->rowCount() <= 0) {
                    m_delUserBtn->setEnabled(false);
                }
            }
        }

        if (m_tabWidget->rowCount() == 0) {
            m_inheritsBox->setEnabled(false);
        }
    });

    connect(m_cancelBtn, &QPushButton::clicked, this, [=](){
        m_userInfo.clear();
        m_defaultAcl.clear();
        this->close();
    });

    connect(m_saveBtn, &QPushButton::clicked, this, [=](){
        for (int row = 0; row < m_tabWidget->rowCount(); ++row) {
            for (int col = 0; col < m_tabWidget->columnCount(); ++col) {
                if (col == 0) {
                    continue;
                }
                QWidget *w = m_tabWidget->cellWidget(row, col);
                QCheckBox *box = w->findChild<QCheckBox*>();
                updateUserInfo(row, col, box->isChecked());
            }
        }

        auto result = QMessageBox::question(nullptr, tr("Permission refinement settings tip"), tr("Setting ACL permissions will result in a change in the user group permissions for basic permissions. Do you need to continue setting ACL permissions?"),
                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
        if (result == QMessageBox::Yes) {
            this->checkInheritsBoxInfo();
            this->saveAclPermissions();
            qDebug() << __func__ << "userInfo" << m_userInfo;
            Q_EMIT this->updatePermissions();
            this->close();
        }
    });
}

void AdvancedPermissionsPage::initTableWidget()
{
    m_tabWidget = new QTableWidget(this);
    m_tabWidget->setColumnCount(4);
    m_tabWidget->verticalHeader()->setVisible(false);
    m_tabWidget->verticalHeader()->setMinimumSectionSize(12);
    m_tabWidget->horizontalHeader()->setFrameShape(QFrame::NoFrame);
    m_tabWidget->setFrameShape(QFrame::NoFrame);
    m_tabWidget->horizontalHeader()->setSelectionMode(QTableWidget::NoSelection);
    m_tabWidget->verticalHeader()->setSelectionMode(QTableWidget::NoSelection);
    m_tabWidget->setSelectionMode(QTableWidget::SingleSelection);
    m_tabWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_tabWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tabWidget->setShowGrid(false);

    m_tabWidget->horizontalHeader()->setMinimumHeight(34);
    m_tabWidget->rowHeight(34);

    m_tabWidget->setAlternatingRowColors(true);
    auto l = QStringList();
    l<<tr("User")<<tr("Read")<<tr("Write")<<tr("Executable");
    m_tabWidget->setHorizontalHeaderLabels(l);
    m_tabWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    m_tabWidget->horizontalHeader()->setMinimumSectionSize(30);
    m_tabWidget->horizontalHeader()->setMaximumSectionSize(400);

    m_tabWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_tabWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tabWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tabWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    //m_tabWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);

    m_tabWidget->setColumnWidth(0, 100);
    m_tabWidget->setColumnWidth(1, 75);
    m_tabWidget->setColumnWidth(3, 120);

    m_layout->addWidget(m_tabWidget);

    int rowCount = m_userInfo.count();
    m_tabWidget->setRowCount(rowCount);

    //Add all user acl
    QMap<QString, QString>::iterator iter;
    int row = 0;
    for (iter = m_userInfo.begin(); iter != m_userInfo.end(); iter++) {
        QString key = iter.key();
        QTableWidgetItem* itemC0 = new QTableWidgetItem(key);
        itemC0->setFlags(itemC0->flags() | Qt::ItemIsSelectable);
        itemC0->setToolTip(key);
        m_tabWidget->setItem(row, 0, itemC0);
        for (int j = 1; j < 4; j++) {
            m_tabWidget->setCellWidget(row, j, nullptr);
            QWidget *w = new QWidget(m_tabWidget);
            QHBoxLayout *l = new QHBoxLayout(w);
            l->setMargin(0);
            w->setLayout(l);
            l->setAlignment(Qt::AlignCenter);
            auto checkbox = new QCheckBox(w);
            bool check = updateCheckBox(j, key);
            checkbox->setChecked(check);
            l->addWidget(checkbox);
            m_tabWidget->setCellWidget(row, j, w);
        }
        ++row;
    }
}

void AdvancedPermissionsPage::initListWidget()
{
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->setContentsMargins(22, 0, 22, 0);
    m_listWidget = new QListWidget(this);
    m_listWidget->setUniformItemSizes(true);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_sysAccounts = new SystemDbusAccounts(this);
    QStringList m_userNames = m_sysAccounts->getAllUserNames();
    QString loginName = qgetenv("USER");
    if (!m_userNames.isEmpty()) {
        for (QString user : m_userNames) {
            if (0 != user.compare(loginName)) {
                auto item = new QListWidgetItem(user, m_listWidget);
                item->setToolTip(user);
                m_listWidget->addItem(item);
            }
        }
    }
    vBoxLayout->addWidget(m_listWidget);
    m_layout->addLayout(vBoxLayout);
}

void AdvancedPermissionsPage::initFloorOne()
{
    m_label = new QLabel(tr("Permission refinement settings"), this);
    m_label->setContentsMargins(22, 0, 22, 0);
    m_layout->addWidget(m_label);
}

void AdvancedPermissionsPage::initFloorTwo()
{
    this->initTableWidget();
    m_layout->addSpacing(10);
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(22, 0, 22, 0);
    hBoxLayout->setSpacing(0);
    m_delUserBtn = new QPushButton(tr("delete"));
    m_delUserBtn->setEnabled(false);
    m_inheritsBox = new QCheckBox(tr("Inherit permission"));
    hBoxLayout->addWidget(m_inheritsBox, 2);
    hBoxLayout->addStretch(1);
    hBoxLayout->addWidget(m_delUserBtn, 1);
    m_layout->addLayout(hBoxLayout);
    m_layout->addSpacing(10);
}

void AdvancedPermissionsPage::initFloorThree()
{
    m_layout->addSpacing(10);
    this->initListWidget();
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(22, 0, 22, 0);
    hBoxLayout->setSpacing(0);
    m_addUserBtn = new QPushButton(tr("Add"));
    m_listLabel = new QLabel;
    m_addUserBtn->setEnabled(false);
    hBoxLayout->addWidget(m_listLabel, 3);
    hBoxLayout->addSpacing(10);
    hBoxLayout->addWidget(m_addUserBtn, 1);
    m_layout->addSpacing(10);
    m_layout->addLayout(hBoxLayout);
    m_layout->addSpacing(10);
}

void AdvancedPermissionsPage::initFloorFour()
{
    m_layout->addSpacing(10);
    QHBoxLayout *hBoxLayout2 = new QHBoxLayout;
    hBoxLayout2->setContentsMargins(22, 0, 22, 0);
    hBoxLayout2->setSpacing(0);
    m_saveBtn = new QPushButton(tr("Apply"));
    m_cancelBtn = new QPushButton(tr("Cancel"));
    hBoxLayout2->addStretch(1);
    hBoxLayout2->addWidget(m_saveBtn);
    hBoxLayout2->addSpacing(10);
    hBoxLayout2->addWidget(m_cancelBtn);
    m_layout->addLayout(hBoxLayout2);
    m_layout->addSpacing(16);
}

void AdvancedPermissionsPage::addSeparate()
{
    QPushButton *separate = new QPushButton;
    separate->setFixedHeight(1);
    separate->setFocusPolicy(Qt::NoFocus);
    separate->setEnabled(false);
    m_layout->addWidget(separate);
}

void AdvancedPermissionsPage::getUserInfo()
{
    if (m_userInfo.isEmpty()) {
        auto info = FileInfo::fromUri(m_uri);
        QStringList args;
        bool ret;
        args << "getfacl" << "-p" << QString("\"%1\"").arg(info->filePath());
        QString acl = UserShareInfoManager::getInstance()->exectueSetAclCommand(args, &ret);
        if (!ret && !acl.isEmpty()) {
            this->close();
            return;
        }
        parseUserInfoAcl(acl);
    }
}

void AdvancedPermissionsPage::parseUserInfoAcl(QString strAcl)
{
    if (!strAcl.isEmpty()) {
        QStringList lists = strAcl.split('\n');
        for (int i = 0; i < lists.size();) {
            QString list = lists.at(i);
            if (list.startsWith("#")
                    || list.isEmpty()
                    || list.contains("user::")
                    || list.contains("group::")
                    || list.contains("mask::")
                    || list.contains("other::")
                    || list.startsWith("default:")) {
                if (list.startsWith("default:user:") && !list.contains("user::")) {
                    QString tmp = list;
                    tmp = tmp.remove(1, 6);
                    m_defaultAcl.append(tmp);
                    m_defaultAcl.append(",");
                }
                lists.removeOne(list);
            } else if (list.contains("#effective:")) {
                int ind = lists.indexOf(list);
                int index = list.indexOf("\t");
                int size = list.size();
                QString tmp = list.remove(index, size);
                lists.replace(ind, tmp);
            } else {
                i++;
            }
        }

        for (QString l : lists) {
            //当前格式为:user:lingmo:rwx,user:lingmo2:r-x
            QStringList tmpLists = l.split(":");
            QString name = tmpLists.at(1);
            QString per = tmpLists.at(2);
            m_mutex.lock();
            m_userInfo.insert(name, per);
            m_mutex.unlock();
        }
        qDebug() << __func__ << "init userInfo" << m_userInfo << m_defaultAcl;
    }
}

bool AdvancedPermissionsPage::updateCheckBox(int col, QString &name)
{
    bool ret = false;
    if (name.isEmpty() || col < 0) {
        return ret;
    }

    if (col == 1 && m_userInfo.value(name).contains("r")) {
        ret = true;
    } else if (col == 2 && m_userInfo.value(name).contains("w")) {
        ret = true;
    } else if (col == 3 && m_userInfo.value(name).contains("x")) {
        ret = true;
    }
    return ret;
}

void AdvancedPermissionsPage::updateUserInfo(int row, int col, bool checked)
{
    if (checked && col >= 1 && col <= 3) {
        QTableWidgetItem *item = m_tabWidget->item(row, 0);
        if (nullptr != item && m_userInfo.contains(item->text())) {
            QString name = item->text();
            QString perm = m_userInfo.value(name);
            if (col == 1) {
                perm = perm.replace(col - 1, 1, "r");
            } else if (col == 2) {
                perm = perm.replace(col - 1, 1, "w");
            } else if (col == 3) {
                perm = perm.replace(col - 1, 1, "x");
            }

            m_userInfo.remove(name);
            m_userInfo.insert(name, perm);
        }
    } else if (!checked && col >= 1 && col <= 3) {
        QTableWidgetItem *item = m_tabWidget->item(row, 0);
        if (nullptr != item && m_userInfo.contains(item->text())) {
            QString name = item->text();
            QString perm = m_userInfo.value(name);
            perm = perm.replace(col - 1, 1, "-");
            m_userInfo.remove(name);
            m_userInfo.insert(name, perm);
        }
    }
}

void AdvancedPermissionsPage::checkInheritsBoxInfo()
{
    if (m_inheritsBox->isChecked()) {
        m_defaultAcl.clear();
        QMap<QString, QString>::iterator iter;
        for (iter = m_userInfo.begin(); iter != m_userInfo.end(); ++iter) {
            QString tmp = "d:u:" + iter.key() + ":" + iter.value() + ",";
            m_defaultAcl.append(tmp);
        }
    } else {
        m_defaultAcl.clear();
    }
}

void AdvancedPermissionsPage::initCheckState()
{
    if (m_tabWidget->rowCount() == 0) {
        m_inheritsBox->setEnabled(false);
        m_inheritsBox->setChecked(false);
    } else if (m_defaultAcl.isEmpty() && m_tabWidget->rowCount() > 0) {
        m_inheritsBox->setEnabled(true);
        m_inheritsBox->setChecked(false);
    } else {
        m_inheritsBox->setEnabled(true);
        m_inheritsBox->setChecked(true);
    }
}

void AdvancedPermissionsPage::saveAclPermissions()
{
    bool retb;
    QStringList args;
    auto info = FileInfo::fromUri(m_uri);
    args << "setfacl" << "-b" << QString("\"%1\"").arg(info->filePath());
    QString result = UserShareInfoManager::exectueSetAclCommand(args, &retb);
    if (!retb && !result.isEmpty()) {
        return;
    }

    if (!m_userInfo.isEmpty()) {
        QString tmpUserInfo = m_defaultAcl;
        QMap<QString, QString>::iterator iter;
        for (iter = m_userInfo.begin(); iter != m_userInfo.end(); ++iter) {
            QString tmp = QString("u:%1:%2,").arg(iter.key()).arg(iter.value());
            tmpUserInfo.append(tmp);
        }

        bool retm;
        args.clear();
        args << "setfacl" << "-m" << tmpUserInfo << QString("\"%1\"").arg(info->filePath());
        qDebug() << __func__ << tmpUserInfo;
        result = UserShareInfoManager::getInstance()->exectueSetAclCommand(args, &retm);
        if (!retm && !result.isEmpty()) {
            return;
        }
    }
}

void AdvancedPermissionsPage::updateDelAclBtn(int row, int col)
{
    if (col >= 1) {
        m_delUserBtn->setEnabled(false);
    } else if (col == 0) {
        m_delUserBtn->setEnabled(true);
    }
}
