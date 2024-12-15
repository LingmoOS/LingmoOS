// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file filesavepathchooser.cpp
 *
 * @brief 下载目录窗口，主要实现切换单选按钮时，文件保存路径控件的使能以及设置数据的传递与显示
 *
 *@date 2020-06-09 11:00
 *
 * Author: yuandandan  <yuandandan@uniontech.com>
 *
 * Maintainer: yuandandan  <yuandandan@uniontech.com>
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
#include "filesavepathchooser.h"
#include "messagebox.h"

#include <DSwitchButton>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDebug>
#include <QStandardPaths>
#include <DSuggestButton>

FileSavePathChooser::FileSavePathChooser(const int &currentSelect, const QString &downloadPath)
{
    m_currentSelect = currentSelect;
    m_downloadPath = downloadPath;

    initUI();
    initConnections();
}

void FileSavePathChooser::initUI()
{
    m_fileChooserEdit = new DFileChooserEdit;
    m_autoLastPathRadioButton = new DRadioButton(tr("Last used directory")); // 自动修改为上次使用的目录
    m_autoLastPathRadioButton->setObjectName("lastPathBtn");
    m_customsPathRadioButton = new DRadioButton(tr("Default directory")); // 设置默认目录
    m_customsPathRadioButton->setObjectName("customPathBtn");

    m_fileChooserEdit->lineEdit()->setReadOnly(true);
    m_fileChooserEdit->lineEdit()->setClearButtonEnabled(false);
    m_fileChooserEdit->setFileMode(QFileDialog::FileMode::DirectoryOnly);

    if (m_currentSelect == 1) {
        m_autoLastPathRadioButton->setChecked(true);
        m_customsPathRadioButton->setChecked(false);
        m_fileChooserEdit->setDisabled(true);
    } else {
        m_autoLastPathRadioButton->setChecked(false);
        m_customsPathRadioButton->setChecked(true);
        m_fileChooserEdit->setDisabled(false);
    }

    m_fileChooserEdit->setText(m_downloadPath);

    for(int i = 0; i < m_fileChooserEdit->children().count(); i++){
        for (int j = 0; j < m_fileChooserEdit->children().at(i)->children().count(); j++) {
            DSuggestButton * dsbtn = qobject_cast<DSuggestButton*>(m_fileChooserEdit->children().at(i)->children().at(j));
            if(dsbtn != nullptr){
                connect(dsbtn, &DSuggestButton::released, [=](){
                    for(int k = 0; k < m_fileChooserEdit->children().count(); k++){
                        auto temp = qobject_cast<QFileDialog *>(m_fileChooserEdit->children().at(k));
                        if(temp != nullptr) {
                           temp->setDirectory(m_downloadPath);
                        }
                    }
                });
                break;
            }
        }
    }

    QHBoxLayout *fileChooserLayout = new QHBoxLayout;
    fileChooserLayout->addWidget(m_fileChooserEdit);
    fileChooserLayout->setContentsMargins(28, 0, 0, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_customsPathRadioButton);
    mainLayout->addLayout(fileChooserLayout);
    mainLayout->addWidget(m_autoLastPathRadioButton);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    //    setStyleSheet("background:rgba(249, 249, 249, 1)");

    setLayout(mainLayout);
}

void FileSavePathChooser::initConnections()
{
    connect(m_autoLastPathRadioButton, &DRadioButton::clicked, this, &FileSavePathChooser::onRadioButtonClicked);
    connect(m_customsPathRadioButton, &DRadioButton::clicked, this, &FileSavePathChooser::onRadioButtonClicked);
    connect(m_fileChooserEdit, &DFileChooserEdit::textChanged, this, &FileSavePathChooser::onLineEditTextChanged);
}

void FileSavePathChooser::onRadioButtonClicked()
{
    DRadioButton *radioButton = qobject_cast<DRadioButton *>(sender());

    if (m_autoLastPathRadioButton == radioButton) {
        m_autoLastPathRadioButton->setChecked(true);
        m_customsPathRadioButton->setChecked(false);
        m_fileChooserEdit->setDisabled(true);

        QString text = "auto;" + m_fileChooserEdit->text();

        emit textChanged(text);
    } else if (m_customsPathRadioButton == radioButton) {
        m_autoLastPathRadioButton->setChecked(false);
        m_customsPathRadioButton->setChecked(true);
        m_fileChooserEdit->setDisabled(false);

        QString text = "custom;" + m_fileChooserEdit->text();

        emit textChanged(text);
    }
}

void FileSavePathChooser::onLineEditTextChanged(const QString &text)
{
    QFileInfo fileInfo;

    fileInfo.setFile(text);
    if (!fileInfo.isWritable()) {
        MessageBox messageBox(this);
        messageBox.setFolderDenied();
        m_fileChooserEdit->setText(m_downloadPath);

        messageBox.exec();
    } else {
        QString changedText = "custom;" + text;

        emit textChanged(changedText);
    }
}

bool FileSavePathChooser::setLineEditText(const QString &text)
{
    m_downloadPath = text;
    m_fileChooserEdit->setText(text);
    return true;
}

bool FileSavePathChooser::setCurrentSelectRadioButton(const int &currentSelect)
{
    if (currentSelect == 1) {
        m_autoLastPathRadioButton->setChecked(true);
        m_customsPathRadioButton->setChecked(false);
        m_fileChooserEdit->setDisabled(true);
    } else {
        m_autoLastPathRadioButton->setChecked(false);
        m_customsPathRadioButton->setChecked(true);
        m_fileChooserEdit->setDisabled(false);
    }
    return true;
}
