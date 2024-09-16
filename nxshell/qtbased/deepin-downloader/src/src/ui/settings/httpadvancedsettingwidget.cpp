// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file httpadvancedsettingwidget.cpp
 *
 * @brief http高级设置
 *
 *@date 2020-07-29 11:00
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

#include "httpadvancedsettingwidget.h"

#include <DPushButton>
#include <QAction>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QFile>
#include "func.h"
#include "config.h"

HttpAdvancedSettingWidget::HttpAdvancedSettingWidget(QWidget *parent)
    : DDialog(parent)
{
    setFixedSize(475, 345);
    m_configPath = QString("%1/%2/%3/httpAdvanced.json")
                       .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                       .arg(qApp->organizationName())
                       .arg(qApp->applicationName());
    //判断文件是否存在,如果不存在复制配置文件内容到目录下
    QFileInfo fileInfo(m_configPath);
    if (!fileInfo.exists()) {
        QFile::copy(CONTENT_HTTP_ADVANCED_PATH, m_configPath);
    }
    initUI();

}

void HttpAdvancedSettingWidget::initUI()
{
    m_btnBox = new DButtonBox(this);
    QList<DButtonBoxButton*> list;
    QFont font;
    font.setFamily("Source Han Sans");
    DButtonBoxButton *suffixBtn =new DButtonBoxButton(tr("Edit File Extensions"));  //编辑下载文件扩展名
    suffixBtn->setAccessibleName("EditFileExtensions");
    suffixBtn->setFont(font);
    connect(suffixBtn,&DButtonBoxButton::clicked, this, &HttpAdvancedSettingWidget::onSuffixBtnClicked);
    m_btnBox->setId(suffixBtn, 0);

    DButtonBoxButton *webBtn =new DButtonBoxButton(tr("Edit Unmonitored Sites"));       //辑不接管的网站
    webBtn->setAccessibleName("EditUnmonitoredSites");
    webBtn->setFont(font);
    connect(webBtn,&DButtonBoxButton::clicked, this, &HttpAdvancedSettingWidget::onWebBtnClicked);
    m_btnBox->setId(webBtn, 1);
    list<<suffixBtn<< webBtn;
    m_btnBox->setButtonList(list,true);
    addContent(m_btnBox, Qt::AlignHCenter);
    addSpacing(10);

    m_textEdit = new DTextEdit(this);
    m_textEdit->setFont(font);
    addContent(m_textEdit);
    addSpacing(8);

    m_defaultBtn = new DCommandLinkButton(tr("Restore"), this);
    m_defaultBtn->setAccessibleName("httpadvanceRestore");
    connect(m_defaultBtn,&DCommandLinkButton::clicked, this, &HttpAdvancedSettingWidget::onRstoreDefaultClicked);
    m_defaultBtn->setFont(font);
    addContent(m_defaultBtn,  Qt::AlignRight);
    addSpacing(20);

    QWidget *w = new QWidget(this);
    w->setAccessibleName("httpadvanceWidget");
    DPushButton * cancel = new DPushButton(this);
    cancel->setAccessibleName("httpadvanceCancel");
    connect(cancel,&DPushButton::clicked, this, &HttpAdvancedSettingWidget::onCancelClicked);
    cancel->setText(tr("Cancel"));
    cancel->setFont(font);

    DPushButton * sure = new DPushButton(this);
    sure->setAccessibleName("httpadvanceSure");
    connect(sure,&DPushButton::clicked, this, &HttpAdvancedSettingWidget::onSureClicked);
    sure->setText(tr("Confirm", "button"));
    sure->setFont(font);

    QHBoxLayout * hblyt = new QHBoxLayout(w);
    hblyt->setContentsMargins(0,0,0,0);
    hblyt->addWidget(cancel);
    hblyt->addSpacing(20);
    hblyt->addWidget(sure);
    w->setLayout(hblyt);
    addContent(w);


    QFile file(m_configPath);
    if(!file.open(QIODevice::ReadWrite)) {
       m_curSuffixStr = "";
       m_curWebStr = "";
    } else {
        QJsonDocument jdc(QJsonDocument::fromJson(file.readAll()));
        QJsonObject obj = jdc.object();
        m_curSuffixStr = obj.value("CurSuffix").toString();
        m_curWebStr = obj.value("CurWeb").toString();
    }
    suffixBtn->click();
}

void HttpAdvancedSettingWidget::reset()
{
    QString str = ".asf;.avi;.exe;.iso;.mp3;.mpeg;.mpg;.mpga;"
                  ".ra;.rar;.rm;.rmvb;.tar;.wma;.wmp;.wmv;.mov;"
                  ".zip;.3gp;.chm;.mdf;.torrent;.jar;.msi;.arj;."
                  "bin;.dll;.psd;.hqx;.sit;.lzh;.gz;.tgz;.xlsx;"
                  ".xls;.doc;.docx;.ppt;.pptx;.flv;.swf;.mkv;.tp;"
                  ".ts;.flac;.ape;.wav;.aac;.txt;.dat;.7z;.ttf;.bat;"
                  ".xv;.xvx;.pdf;.mp4;.apk;.ipa;.epub;.mobi;.deb;.sisx;.cab;.pxl;";

    QFile file(m_configPath);
    if(!file.open(QIODevice::ReadWrite)) {
        return;
    }
    QJsonDocument jdc(QJsonDocument::fromJson(file.readAll()));
    QJsonObject obj = jdc.object();
    obj["CurSuffix"] = str;
    obj["CurWeb"] = "";
    jdc.setObject(obj);
    file.resize(0);
    file.write(jdc.toJson());
    file.close();
    file.flush();
    close();

}

void HttpAdvancedSettingWidget::onSuffixBtnClicked()
{
    QString curPlaceholderText = m_textEdit->placeholderText();
    if(!(curPlaceholderText == tr("Separate file extensions by semicolons (;)"))
            && !curPlaceholderText.isEmpty()) {
        m_curWebStr = m_textEdit->toPlainText();
    }
    m_textEdit->setPlaceholderText(tr("Separate file extensions by semicolons (;)"));   //请输入正确的文件扩展名，以;分隔
    m_textEdit->setText(m_curSuffixStr);
    m_defaultBtn->show();
}
void HttpAdvancedSettingWidget::onWebBtnClicked()
{
    if(!(m_textEdit->placeholderText() == tr("Please enter one URL per line"))
            && !m_textEdit->placeholderText().isEmpty()) {
        m_curSuffixStr = m_textEdit->toPlainText();
    }
    m_textEdit->setPlaceholderText(tr("Please enter one URL per line"));        //添加多个网站链接时，请确保每行只有一个链接
    m_textEdit->setText(m_curWebStr);
    m_defaultBtn->hide();
}

void HttpAdvancedSettingWidget::onRstoreDefaultClicked()
{
//    QFile file(m_configPath);
//    if(!file.open(QIODevice::ReadWrite)) {
//        return;
//    }
//    QJsonDocument jdc(QJsonDocument::fromJson(file.readAll()));
//    QJsonObject obj = jdc.object();
//    file.close();
//    QString defaultSuffix = obj.value("DefaultSuffix").toString();
//    m_textEdit->setText(defaultSuffix);
    m_textEdit->setText(".asf;.avi;.exe;.iso;.mp3;.mpeg;.mpg;.mpga;"
                        ".ra;.rar;.rm;.rmvb;.tar;.wma;.wmp;.wmv;.mov;"
                        ".zip;.3gp;.chm;.mdf;.torrent;.jar;.msi;.arj;."
                        "bin;.dll;.psd;.hqx;.sit;.lzh;.gz;.tgz;.xlsx;"
                        ".xls;.doc;.docx;.ppt;.pptx;.flv;.swf;.mkv;.tp;"
                        ".ts;.flac;.ape;.wav;.aac;.txt;.dat;.7z;.ttf;.bat;"
                        ".xv;.xvx;.pdf;.mp4;.apk;.ipa;.epub;.mobi;.deb;.sisx;.cab;.pxl;");
}

void HttpAdvancedSettingWidget::onSureClicked()
{
    QFile file(m_configPath);
    if(!file.open(QIODevice::ReadWrite)) {
        return;
    }
    QJsonDocument jdc(QJsonDocument::fromJson(file.readAll()));
    QJsonObject obj = jdc.object();
    QString value = m_textEdit->toPlainText();
    if(m_btnBox->buttonList()[0]->isChecked()){
        obj["CurSuffix"] = value;
    }else {
        obj["CurWeb"] = value;
    }
    jdc.setObject(obj);
    file.resize(0);
    file.write(jdc.toJson());
    file.close();
    file.flush();
    close();
}

void HttpAdvancedSettingWidget::onCancelClicked()
{
    close();
}
