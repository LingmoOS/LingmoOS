/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include <QHeaderView>
#include <QTableView>
#include <QCheckBox>
#include <QButtonGroup>
#include <QtConcurrent>
#include <PeonyFileInfoJob>
#include <PeonyFileInfo>
#include <QListWidgetItem>
#include <QAbstractItemView>
#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include "advanced-share-page.h"
#include "usershare-manager.h"
#include "samba-config-interface.h"
#include "global-settings.h"

static const QString everyone = "Everyone";

AdvancedSharePage::AdvancedSharePage(const QString &uri, const QMap<QString, QString> &userInfo, QWidget *parent)
    : QWidget(parent)
{
    m_userInfo = userInfo;
    m_uri = uri;
    this->init();
}

AdvancedSharePage::~AdvancedSharePage()
{

}

void AdvancedSharePage::init()
{
    this->setWindowTitle(tr("Advanced share"));
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

    this->updateShareUserInfo();

    this->initFloorOne();
    this->initFloorTwo();
    this->addSeparate();
    this->initFloorThree();
    this->addSeparate();
    this->initFloorFour();

    connect(m_listWidget, &QListWidget::currentTextChanged, this, [=](QString currentText){
        if (!m_userInfo.contains(currentText)) {
            m_addShareUserBtn->setEnabled(true);
        } else {
            m_addShareUserBtn->setEnabled(false);
        }
        m_listLabel->setText(currentText);
    });

    connect(m_addShareUserBtn, &QPushButton::clicked, this, [=](){
        QListWidgetItem *item = m_listWidget->currentItem();
        if (nullptr != item) {
            QString name = item->text();
            QString username = g_get_user_name();

            auto settings = GlobalSettings::getInstance();
            bool runbackend = settings->getInstance()->getValue(RESIDENT_IN_BACKEND).toBool();
            qApp->setQuitOnLastWindowClosed(false);

            //没有设置samba密码：1）设置当前用户samba密码 2）提示其他用户需要设置samba密码
            SambaConfigInterface si(DBUS_NAME, DBUS_PATH, QDBusConnection::systemBus());
            bool initRet = si.init(name, getpid(), getuid());
            bool ret = false;
            if (initRet) {
                if (!si.hasPasswd() && username == name) {
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
                        if (!si.setPasswd(text)) {
                            qApp->setQuitOnLastWindowClosed(!runbackend);
                            QMessageBox::warning(nullptr, tr("Warning"), tr("Samba set password failed, Please re-enter!"));
                            ret = true;
                        }
                    } else {
                        qApp->setQuitOnLastWindowClosed(!runbackend);
                        ret  = true;
                    }
                } else if (!si.hasPasswd() && username != name) {
                    QMessageBox::information(nullptr, tr("Tips"), tr("The user has not set the samba password. If you need to log in to %1, you can set it in the upper right menu of the file manager").arg(name));
                }
            } else {
                qApp->setQuitOnLastWindowClosed(!runbackend);
                QMessageBox::warning(nullptr, tr("Warning"), tr("Shared configuration service exception, please confirm if there is an ongoing shared configuration operation, or please reset the share!"), QMessageBox::Ok);
                ret = true;
            }
            si.finished();
            if (ret) {
                return;
            }

            //增加用户默认设置
            m_mutex.lock();
            m_userInfo.insert(name, "r");
            m_mutex.unlock();

            m_addShareUserBtn->setEnabled(false);

            int rowCount = m_tabWidget->rowCount();
            m_tabWidget->insertRow(rowCount);
            QTableWidgetItem* itemC = new QTableWidgetItem(name);
            itemC->setFlags(itemC->flags() | Qt::ItemIsSelectable);
            m_tabWidget->setItem(rowCount, 0, itemC);
            QButtonGroup *group = new QButtonGroup;
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
                group->addButton(checkbox);
                l->addWidget(checkbox);
                m_tabWidget->setCellWidget(rowCount, i, w);
            }
        }
    });

    connect(m_delShareUserBtn, &QPushButton::clicked, this, [=](){
        QTableWidgetItem *item = m_tabWidget->currentItem();
        if (nullptr != item) {
            QString name = item->text();

            if (m_userInfo.contains(name)) {
                m_mutex.lock();
                m_userInfo.remove(name);
                m_mutex.unlock();

                int currentRow = m_tabWidget->currentRow();
                m_tabWidget->removeRow(currentRow);
                if (m_tabWidget->rowCount() <= 1) {
                    m_delShareUserBtn->setEnabled(false);
                }
            }
        }
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

        QString acl;
        QString rejectStr;
        QString otherStr;
        QMap<QString, QString>::iterator iter;
        for(iter = m_userInfo.begin();iter != m_userInfo.end();iter++){
            QString str = iter.key() + ":" + iter.value() + ",";
            if (iter.value().compare("d", Qt::CaseInsensitive) == 0) {
                rejectStr.append(str);
            } else {
                otherStr.append(str);
            }
        }
        acl.append(rejectStr);
        acl.append(otherStr);

        QString aclPerms = parseSetAcl(m_userInfo);
        auto info = FileInfo::fromUri(m_uri);
        //Clear previous acl permissions
        bool retb;
        bool retm;
        QStringList args;
        args << "setfacl" << "-b" << QString("\"%1\"").arg(info->filePath());
        QString result = UserShareInfoManager::exectueSetAclCommand(args, &retb);
        if (!retb && !result.isEmpty()) {
            return;
        }

        args.clear();
        args << "setfacl" << "-m" << aclPerms << QString("\"%1\"").arg(info->filePath());
        result = UserShareInfoManager::exectueSetAclCommand(args, &retm);
        if (!retm && !result.isEmpty()) {
            return;
        }

        acl.chop(1);
        Q_EMIT getUserInfo(acl, m_userInfo);
        this->close();
    });

    connect(m_cancelBtn, &QPushButton::clicked, this, [=](){
        m_userInfo.clear();
        this->close();
    });
    connect(m_tabWidget, &QTableWidget::cellEntered, this, &AdvancedSharePage::updateDelBtnState);
    connect(m_tabWidget, &QTableWidget::cellClicked, this, &AdvancedSharePage::updateDelBtnState);
    connect(m_tabWidget, &QTableWidget::itemClicked, this, [=](QTableWidgetItem *item){
        if (nullptr != item) {
            m_tabLabel->setText(item->text());
        }
    });
}

void AdvancedSharePage::initTableWidget()
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
    l<<tr("User")<<tr("Writable")<<tr("Readonly")<<tr("Reject");
    m_tabWidget->setHorizontalHeaderLabels(l);
    m_tabWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    m_tabWidget->horizontalHeader()->setMinimumSectionSize(30);
    m_tabWidget->horizontalHeader()->setMaximumSectionSize(400);

    m_tabWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_tabWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tabWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tabWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_tabWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);

    m_tabWidget->setColumnWidth(0, 100);
    m_tabWidget->setColumnWidth(3, 80);

    m_layout->addWidget(m_tabWidget);
    if (m_userInfo.isEmpty()) {
        m_userInfo.insert(everyone, "r");
    }

    int rowCount = m_userInfo.count();
    m_tabWidget->setRowCount(rowCount);

    //Add Everyone
    QTableWidgetItem* itemR0C0 = new QTableWidgetItem(everyone);
    itemR0C0->setFlags(itemR0C0->flags() | Qt::ItemIsSelectable);
    m_tabWidget->setItem(0, 0, itemR0C0);
    QButtonGroup *group = new QButtonGroup;
    for (int i = 1; i < 4; ++i) {
        m_tabWidget->setCellWidget(0, i, nullptr);
        QWidget *w = new QWidget(m_tabWidget);
        QHBoxLayout *l = new QHBoxLayout(w);
        l->setMargin(0);
        w->setLayout(l);
        l->setAlignment(Qt::AlignCenter);
        auto checkbox = new QCheckBox(w);
        bool check = updateCheckBox(i, const_cast<QString&>(everyone));
        checkbox->setChecked(check);
        group->addButton(checkbox);
        l->addWidget(checkbox);
        m_tabWidget->setCellWidget(0, i, w);
    }

    //All usershare
    QMap<QString, QString>::iterator iter;
    int row = 1;
    for(iter = m_userInfo.begin();iter != m_userInfo.end();iter++){
        QString key = iter.key();
        if (iter.key() == everyone) {
            continue;
        }
        QTableWidgetItem* itemC0 = new QTableWidgetItem(key);
        itemC0->setFlags(itemC0->flags() | Qt::ItemIsSelectable);
        m_tabWidget->setItem(row, 0, itemC0);
        QButtonGroup *group = new QButtonGroup;
        for (int j = 1; j < 4; ++j) {
            m_tabWidget->setCellWidget(row, j, nullptr);
            QWidget *w = new QWidget(m_tabWidget);
            QHBoxLayout *l = new QHBoxLayout(w);
            l->setMargin(0);
            w->setLayout(l);
            l->setAlignment(Qt::AlignCenter);
            auto checkbox = new QCheckBox(w);
            bool check = updateCheckBox(j, key);
            checkbox->setChecked(check);
            group->addButton(checkbox);
            l->addWidget(checkbox);
            m_tabWidget->setCellWidget(row, j, w);
        }
        ++row;
    }
}

void AdvancedSharePage::initListWidget()
{
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->setContentsMargins(22, 0, 22, 0);
    m_listWidget = new QListWidget(this);
    m_listWidget->setUniformItemSizes(true);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_sysAccounts = new SystemDbusAccounts(this);
    QStringList m_userNames = m_sysAccounts->getAllUserNames();
    if (!m_userNames.isEmpty()) {
        for (QString user : m_userNames) {
            auto item = new QListWidgetItem(user, m_listWidget);
            m_listWidget->addItem(item);
        }
    }
    vBoxLayout->addWidget(m_listWidget);
    m_layout->addLayout(vBoxLayout);
}

void AdvancedSharePage::initFloorOne()
{
    m_label = new QLabel(tr("Share permission settings"), this);
    m_label->setContentsMargins(22, 0, 22, 0);
    m_layout->addWidget(m_label);
}

void AdvancedSharePage::initFloorTwo()
{
    this->initTableWidget();
    m_layout->addSpacing(10);
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(22, 0, 22, 0);
    hBoxLayout->setSpacing(0);
    m_delShareUserBtn = new QPushButton(tr("delete"));
    m_tabLabel = new QLabel;
    m_delShareUserBtn->setEnabled(false);
    hBoxLayout->addWidget(m_tabLabel, 3);
    hBoxLayout->addSpacing(10);
    hBoxLayout->addWidget(m_delShareUserBtn, 1);
    m_layout->addLayout(hBoxLayout);
    m_layout->addSpacing(10);
}

void AdvancedSharePage::initFloorThree()
{
    m_layout->addSpacing(10);
    this->initListWidget();
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(22, 0, 22, 0);
    hBoxLayout->setSpacing(0);
    m_addShareUserBtn = new QPushButton(tr("Add"));
    m_listLabel = new QLabel;
    m_addShareUserBtn->setEnabled(false);
    hBoxLayout->addWidget(m_listLabel, 3);
    hBoxLayout->addSpacing(10);
    hBoxLayout->addWidget(m_addShareUserBtn, 1);
    m_layout->addSpacing(10);
    m_layout->addLayout(hBoxLayout);
    m_layout->addSpacing(10);
}

void AdvancedSharePage::initFloorFour()
{
    m_layout->addSpacing(10);
    QHBoxLayout *hBoxLayout2 = new QHBoxLayout;
    hBoxLayout2->setContentsMargins(22, 0, 22, 0);
    hBoxLayout2->setSpacing(0);
    m_saveBtn = new QPushButton(tr("Save"));
    m_cancelBtn = new QPushButton(tr("Cancel"));
    hBoxLayout2->addStretch(1);
    hBoxLayout2->addWidget(m_saveBtn);
    hBoxLayout2->addSpacing(10);
    hBoxLayout2->addWidget(m_cancelBtn);
    m_layout->addLayout(hBoxLayout2);
    m_layout->addSpacing(16);
}

void AdvancedSharePage::updateShareUserInfo()
{
    if (m_userInfo.isEmpty()) {
        auto info = FileInfo::fromUri(m_uri);
        auto manager = UserShareInfoManager::getInstance();
        QString displayName = info->displayName();
        QString acl = manager->getUserShareAcl(displayName);
        parseUserShareAcl(acl);
    }
}

void AdvancedSharePage::parseUserShareAcl(QString strAcl)
{
    //传参格式为：XWJ-PC\\xwj:D,Everyone:F,S-1-5-21-120585688-1972774468-4199965856-1003:F,Unix User\\samba:D,
    //或者为:xwj:D,Everyone:F,lingmo:F,samba:D,
    if (!strAcl.isEmpty()) {
        QStringList strAcls = strAcl.split(',');
        if (strAcls.last().isEmpty()) {
            strAcls.removeLast();
        }
        for (QString acl : strAcls) {
            QStringList users = acl.split(':');
            QString name = users.at(0);
            QString usershareAcl = users.at(1);
            if (name.contains("\\")) {
                //处理用户为：Unix User\\samba:D的情况
                int index = name.indexOf("\\");
                name.remove(0, index + 1);
            }
            //统一处理用户
            m_mutex.lock();
            m_userInfo.insert(name, usershareAcl);
            m_mutex.unlock();
        }
    }
}

bool AdvancedSharePage::updateCheckBox(int col, QString &name)
{
    bool ret = false;
    if (name.isEmpty() || col < 0) {
        return ret;
    }
    if (col == 1 && m_userInfo.value(name).compare("f", Qt::CaseInsensitive) == 0) {
        ret = true;
    } else if (col == 2 && m_userInfo.value(name).compare("r", Qt::CaseInsensitive) == 0) {
        ret = true;
    } else if (col == 3 && m_userInfo.value(name).compare("d", Qt::CaseInsensitive) == 0) {
        ret = true;
    }
    return ret;
}

void AdvancedSharePage::updateUserInfo(int row, int col, bool checked)
{
    if (checked && col >= 1) {
        QTableWidgetItem *item = m_tabWidget->item(row, 0);
        if (nullptr != item && m_userInfo.contains(item->text())) {
            if (col == 1) {
                m_userInfo.remove(item->text());
                m_userInfo.insert(item->text(), "f");
            } else if (col == 2) {
                m_userInfo.remove(item->text());
                m_userInfo.insert(item->text(), "r");
            } else if (col == 3) {
                m_userInfo.remove(item->text());
                m_userInfo.insert(item->text(), "d");
            }
        }
    }
}

QString AdvancedSharePage::parseSetAcl(QMap<QString, QString> &userInfo)
{
    QString aclPerms;
    QMap<QString, QString>::iterator iter;
    for(iter = userInfo.begin();iter != userInfo.end();iter++){
        QString tmp;
        QString tmp2;
        QString perm = converPermission(iter.value());
        if (iter.key() == everyone) {
            tmp = "d:u:nobody:" + perm;
            tmp2 = "u:nobody:" + perm;
        } else {
            tmp = "d:u:" + iter.key() + ":" + perm;
            tmp2 = "u:" + iter.key() + ":" + perm;
        }
        aclPerms.append(tmp + ",");
        aclPerms.append(tmp2 + ",");
    }
    return aclPerms;
}

QString AdvancedSharePage::converPermission(QString &usershareAcl)
{
    QString acls;
    if (!usershareAcl.isEmpty()) {
        if (usershareAcl.compare("f", Qt::CaseInsensitive) == 0) {
            acls = "rwx";
        } else if (usershareAcl.compare("r", Qt::CaseInsensitive) == 0) {
            acls = "r-x";
        } else if (usershareAcl.compare("d", Qt::CaseInsensitive) == 0) {
            acls = "--x";
        }
    }
    return acls;
}

void AdvancedSharePage::addSeparate()
{
    QPushButton *separate = new QPushButton;
    separate->setFixedHeight(1);
    separate->setFocusPolicy(Qt::NoFocus);
    separate->setEnabled(false);
    m_layout->addWidget(separate);
}

void AdvancedSharePage::updateDelBtnState(int row, int col)
{
    if (col >= 1) {
        m_delShareUserBtn->setEnabled(false);
    } else if (col == 0 && row != 0) {
        m_delShareUserBtn->setEnabled(true);
    } else if (col == 0 && row == 0) {
        m_delShareUserBtn->setEnabled(false);
    }

    if (col >= 1) {
        m_tabLabel->setText("");
    }
}
