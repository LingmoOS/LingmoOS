// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file messagebox.cpp
 *
 * @brief 提示窗口类
 *
 * @date 2020-06-09 10:52
 *
 * Author: bulongwei  <bulongwei@uniontech.com>
 *
 * Maintainer: bulongwei  <bulongwei@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "messagebox.h"
#include "settings.h"
#include <QPushButton>

MessageBox::MessageBox(QWidget *parent)
    : DDialog(parent)
{
    setObjectName("messageBox");
    setMaximumWidth(600);
}

void MessageBox::setWarings(QString warningMsg, QString surebtntext, QString cancelbtntext, int sameurlCount, QList<QString> sameUrlList)
{
    setIcon(QIcon::fromTheme(":/icons/icon/ndm_messagebox_logo_32px.svg"));

    setTitle(tr("Warning"));
    setObjectName("messageBox");

    addLabel(warningMsg);
    addSpacing(10);
    if (sameurlCount != 0) {
        DTextEdit *urlText = new DTextEdit(this);
        urlText->setReadOnly(true);
        urlText->setFixedSize(QSize(454, 154));

        QPalette pal;
        pal.setColor(QPalette::Base, QColor(0, 0, 0, 20));
        urlText->setPalette(pal);
        for (int i = 0; i < sameUrlList.size(); i++) {
            urlText->append(sameUrlList.at(i));
        }
        addContent(urlText);
    }
    if (!cancelbtntext.isEmpty()) {
        QAbstractButton *btn1 = getButton(addButton(cancelbtntext));
        btn1->setObjectName("cancel");
        btn1->setAccessibleName("cancel");
        QAbstractButton *btn2 = getButton(addButton(surebtntext, true, ButtonType::ButtonWarning));
        btn2->setObjectName("Confirm");
        btn2->setAccessibleName("Confirm");
    } else {
        QAbstractButton *btn2 = getButton(addButton(surebtntext, true, ButtonType::ButtonWarning));
        btn2->setObjectName("OK");
        btn2->setObjectName("OK");
    }

    connect(this, &MessageBox::buttonClicked, this,
            [=]() {
                close();
            });
}

void MessageBox::setRedownload(const QString sameUrl, bool ret, bool isShowRedownload)
{
    setIcon(QIcon::fromTheme(":/icons/icon/ndm_messagebox_logo_32px.svg"));
    if (ret) {
        setTitle(tr("Download Again"));
        addLabel(tr("Do you want to delete the downloaded files and download again?"));
    } else {
        setTitle(tr("These tasks already exist"));
        if (!isShowRedownload) {
            addLabel(tr("Delete the downloaded files and download again?"));
        }
    }
    addSpacing(10);
    DTextEdit *urlText = new DTextEdit(this);
    urlText->setReadOnly(true);
    urlText->setFixedSize(QSize(454, 154));
    urlText->setText(sameUrl);
    QPalette pal;
    pal.setColor(QPalette::Base, QColor(0, 0, 0, 20));
    addContent(urlText);

    if (isShowRedownload) {
        QAbstractButton *btn1 = getButton(addButton(tr("OK")));
        btn1->setObjectName("OK");
        btn1->setAccessibleName("OK");
    } else {
        QAbstractButton *btn1 = getButton(addButton(tr("Cancel")));
        btn1->setObjectName("cancel");
        btn1->setAccessibleName("cancele");
        QAbstractButton *btn2 = getButton(addButton(tr("Download Again"), true, ButtonType::ButtonWarning));
        btn2->setObjectName("redownload");
        btn2->setAccessibleName("redownload");
    }
}

void MessageBox::setUnusual(const QString &taskId, QString taskList)
{
    setIcon(QIcon::fromTheme(":/icons/icon/ndm_messagebox_logo_32px.svg"));

    setTitle(tr("Warning"));

    addLabel(tr("Download error"));
    addSpacing(10);
    DTextEdit *urlText = new DTextEdit(this);
    urlText->setReadOnly(true);
    urlText->setFixedSize(QSize(454, 154));
    urlText->setText(taskList);
    addContent(urlText);
    //if(taskList.split("\n").count() <= 1) {
    QAbstractButton *btn = getButton(addButton(tr("Download Again")));
    btn->setObjectName("DownloadAgain");
    btn->setAccessibleName("DownloadAgain");
    //}
    QAbstractButton *btn1 = getButton(addButton(tr("Delete All")));
    btn1->setObjectName("DeleteAll");
    btn1->setAccessibleName("DeleteAll");
    connect(this, &MessageBox::buttonClicked, this,
            [=](int index) {
                emit unusualConfirm(index, taskId);
                close();
            });
}

void MessageBox::setDelete(bool permanentl, bool checked)
{
    setIcon(QIcon::fromTheme(":/icons/icon/ndm_messagebox_logo_32px.svg"));
    m_DeleteFlag = permanentl;

    if (permanentl) {
        QString show_title = tr("Are you sure you want to delete this download task permanently?");
        setTitle(show_title);
        addSpacing(10);
        QString show_msg = tr("Local files will be deleted at the same time.");
        addLabel(show_msg);
    } else {
        QString showTitle = tr("Are you sure you want to delete this download task?");
        setTitle(showTitle);
        addSpacing(10);
        if (checked) {
            addCheckbox(tr("Delete local files"), true);
        } else {
            addCheckbox(tr("Delete local files"));
        }
    }
    addSpacing(10);

    QAbstractButton *btn = getButton(addButton(tr("Cancel")));
    btn->setObjectName("cancel");
    btn->setAccessibleName("cancel");
    if (permanentl) {
        QAbstractButton *btn = getButton(addButton(tr("Permanently Delete"), true, ButtonType::ButtonWarning));
        btn->setObjectName("delete");
        btn->setAccessibleName("delete");
    } else {
        QAbstractButton *btn = getButton(addButton(tr("Delete"), true, ButtonType::ButtonWarning));
        btn->setObjectName("delete");
        btn->setAccessibleName("delete");
    }
    connect(this, &MessageBox::buttonClicked, this, &MessageBox::onDeleteBtnClicked);
}
void MessageBox::setClear()
{
    setIcon(QIcon::fromTheme(":/icons/icon/ndm_messagebox_logo_32px.svg"));
    QString show_title = tr("Are you sure you want to delete all tasks in the trash?");
    addLabel(show_title);
    addSpacing(10);
    addCheckbox(tr("Delete local files"));
    QAbstractButton *btn1 = getButton(addButton(tr("Cancel")));
    btn1->setObjectName("cancel");
    btn1->setAccessibleName("cancel");
    QAbstractButton *btn2 = getButton(addButton(tr("Empty"), true, ButtonType::ButtonWarning));
    btn2->setObjectName("empty");
    btn2->setAccessibleName("empty");
    connect(this, &MessageBox::buttonClicked, this, &MessageBox::onClearBtnClicked);
}

void MessageBox::setExit()
{
    setIcon(QIcon(":/icons/icon/downloader5.svg"));

    setTitle(tr("Please choose your action"));
    addSpacing(10);
    addRadioGroup(tr("Exit"), tr("Minimize to system tray"));
    addSpacing(10);
    addCheckbox(tr("Do not ask again"));
    addButton(tr("Cancel"));
    addButton(tr("Confirm"));

    connect(this, &MessageBox::buttonClicked, this, &MessageBox::onExitBtnClicked);
}

void MessageBox::addLabel(QString text)
{
    DLabel *title = new DLabel(this);
    title->setAccessibleName("messageBoxTittle");
    title->setText(text);
    addContent(title, Qt::AlignHCenter);
}

void MessageBox::addRadioGroup(QString quitText, QString minText)
{
    int status = Settings::getInstance()->getCloseMainWindowSelected();
    m_ButtonQuit = new DRadioButton(quitText);
    m_ButtonMin = new DRadioButton(minText);
    addContent(m_ButtonQuit, Qt::AlignLeft);
    addSpacing(5);
    addContent(m_ButtonMin, Qt::AlignLeft);
    if (status) {
        m_ButtonQuit->setChecked(true);
        m_ButtonMin->setChecked(false);
    } else {
        m_ButtonMin->setChecked(true);
        m_ButtonQuit->setChecked(false);
    }
    connect(m_ButtonQuit, &DRadioButton::clicked, this,
            [=]() {
                m_ButtonMin->setChecked(false);
            });
    connect(m_ButtonMin, &DRadioButton::clicked, this,
            [=]() {
                m_ButtonQuit->setChecked(false);
            });
}

void MessageBox::addCheckbox(QString checkboxText, bool checked)
{
    m_CheckBox = new DCheckBox(this);
    m_CheckBox->setText(checkboxText);
    m_CheckBox->setAccessibleName("CheckBox");
    if (checked) {
        m_CheckBox->setCheckState(Qt::Checked);
    }
    addContent(m_CheckBox, Qt::AlignLeft);
}

void MessageBox::onRenamelineeditChanged(const QString &text)
{
    QString real_name = QString(text).left(text.lastIndexOf('.'));

    if (!text.isEmpty() && !real_name.isEmpty()) {
        m_RenameSureButton->setEnabled(true);
    } else {
        m_RenameSureButton->setEnabled(false);
    }
}

void MessageBox::onRenameSureBtnClicked()
{
    QString newname = m_NewnameLineedit->text();
    if (newname.contains("\\") || newname.contains("/")) {
        MessageBox *msg = new MessageBox();
        msg->setWarings(tr("The file name cannot contain a backslash (\\) or slash (/)"), tr("OK"));
        msg->exec();
        return;
    }
    emit Rename(newname);
    close();
}

void MessageBox::onClearBtnClicked(int index)
{
    if (index == 1) {
        bool ischecked;
        ischecked = m_CheckBox->isChecked();
        emit Clearrecycle(ischecked);
    }
    close();
}

void MessageBox::onDeleteBtnClicked(int index)
{
    if (index == 1) {
        QAbstractButton *button = getButton(index);
        button->setEnabled(false);
        if (m_DeleteFlag)
            emit Deletedownload(true, m_DeleteFlag);
        else {
            bool ischecked;
            ischecked = m_CheckBox->isChecked();
            emit Deletedownload(ischecked, m_DeleteFlag);
        }
    }
    close();
}

void MessageBox::onExitBtnClicked(int index)
{
    if (index == 1) {
        if (m_CheckBox->isChecked()) {
            Settings::getInstance()->setIsShowTip(false);
            if (m_ButtonMin->isChecked()) {
                Settings::getInstance()->setCloseMainWindowSelected(0);
            } else {
                Settings::getInstance()->setCloseMainWindowSelected(1);
            }
        }
        emit closeConfirm(m_ButtonMin->isChecked() ? 0 : 1);
    }
    close();
}

void MessageBox::setFolderDenied()
{
    setIcon(QIcon::fromTheme(":/icons/icon/ndm_messagebox_logo_32px.svg"));

    setTitle(tr("Permission denied"));

    addLabel(tr("Please try another folder"));
    addSpacing(10);
    addButton(tr("OK"));
    connect(this, &MessageBox::buttonClicked, this,
            [=]() {
                close();
    });
}

void MessageBox::setFolderNotExists()
{
    setIcon(QIcon::fromTheme(":/icons/icon/ndm_messagebox_logo_32px.svg"));

    setTitle(tr("Path Error"));

    addLabel(tr("The storage folder does not exist, please check and try again"));
    addSpacing(10);
    addButton(tr("OK"));
    connect(this, &MessageBox::buttonClicked, this,
            [=]() {
                close();
    });
}

void MessageBox::setNetWorkError(QString warningMsg)
{
    setIcon(QIcon::fromTheme(":/icons/icon/ndm_messagebox_logo_32px.svg"));

   // setTitle(warningMsg);
    setObjectName("messageBox");

    addLabel(warningMsg);
    addSpacing(10);
    addButton(tr("OK"));
//        QAbstractButton *btn2 = getButton(addButton(surebtntext, true, ButtonType::ButtonWarning));
//        btn2->setObjectName("OK");

    connect(this, &MessageBox::buttonClicked, this,
            [=]() {
                close();
            });
}
