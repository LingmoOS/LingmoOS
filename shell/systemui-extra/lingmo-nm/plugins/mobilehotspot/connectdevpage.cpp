/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "connectdevpage.h"
#include <QDebug>

#define CONTENTS_MARGINS 0, 0, 0, 0
#define FRAME_MIN_SIZE 550, 60
#define FRAME_MAX_SIZE 16777215, 16777215
#define LINE_MAX_SIZE 16777215, 1
#define LINE_MIN_SIZE 0, 1

#define LOG_HEAD "[ConnectdevPage]"

ConnectdevPage::ConnectdevPage(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *Vlayout = new QVBoxLayout(this);
    Vlayout->setContentsMargins(CONTENTS_MARGINS);
    Vlayout->setSpacing(0);

    m_staistFrame = new QFrame(this);
    m_staistFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_staistFrame->setMaximumSize(FRAME_MAX_SIZE);
    m_staistFrame->setFrameShape(QFrame::Box);

    m_staListLayout = new QVBoxLayout(m_staistFrame);
    m_staListLayout->setContentsMargins(0, 0, 0, 0);
    m_staListLayout->setSpacing(0);

    m_titleLabel = new TitleLabel(this);
    m_titleLabel->setText(tr("Connect device"));

    Vlayout->addWidget(m_titleLabel);
    Vlayout->addSpacing(8);
    Vlayout->addWidget(m_staistFrame);
}

QFrame* ConnectdevPage::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(LINE_MIN_SIZE));
    line->setMaximumSize(QSize(LINE_MAX_SIZE));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

void ConnectdevPage::getConnectStaDevice(QMap<QString, QString> &staMap)
{
    staMap.clear();
    if (m_activePathInterface == nullptr || !m_activePathInterface->isValid()) {
        qDebug() << LOG_HEAD << "dbus interface m_activePathInterface is invaild";
        return;
    }

    QDBusMessage reply = m_activePathInterface->call("Getstainfo");
    if(reply.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << LOG_HEAD << "Getstainfo error:" << reply.errorMessage();
        return;
    }

    if (reply.arguments().isEmpty()
        || reply.arguments().at(0).toString() == ""
        || reply.arguments().at(0).toString() == "[Invalid UTF-8]"
        || reply.arguments().at(1).toString() == "") {
        qDebug() << LOG_HEAD << "Dbus interface call Getstainfo return is empty!";
        return;
    }

    QStringList macList = reply.arguments().at(0).toString().split(";");
    QStringList hostNameList = reply.arguments().at(1).toString().split(";");
    for (int index = 0; index < macList.count() && macList.at(index) != nullptr; index ++) {
        if (!staMap.contains(macList.at(index)) && hostNameList.at(index) != nullptr) {
            staMap[macList.at(index)] = hostNameList.at(index);
        }
    }

}

void ConnectdevPage::initStaDev()
{
    QMap<QString, QString>::const_iterator item = m_staMap.cbegin();
    while (item != m_staMap.cend()) {
        addStaDevFrame(item.key(), item.value());
        item ++;
        if (item != m_staMap.cend()) {
            m_staListLayout->addWidget(myLine());
        }
    }
}

void ConnectdevPage::addStaDevFrame(QString staMac, QString staName)
{
    ConnectDevListItem *itemFrame = new ConnectDevListItem(staMac, staName, m_staListLayout->widget());
    m_staListLayout->addWidget(itemFrame);
    connect(itemFrame, &ConnectDevListItem::onBtnClicked, this, &ConnectdevPage::onDropIntoBlacklistBtnClicked);
}

void ConnectdevPage::clearStaListLayout()
{

    if (m_staListLayout->layout() != NULL) {
        QLayoutItem* layoutItem;
        while ((layoutItem = m_staListLayout->layout()->takeAt(0)) != NULL) {
            delete layoutItem->widget();
            delete layoutItem;
        }
    }
}

void ConnectdevPage::onStaDevAdded(bool istrue, QString staMac, QString staName)
{
    if (!m_staMap.contains(staMac)) {
        m_staMap.insert(staMac, staName);
        clearStaListLayout();
        initStaDev();
        resetLayoutHight();
    }
}

void ConnectdevPage::onStaDevRemoved(bool istrue, QString staMac, QString staName)
{
    if (m_staMap.contains(staMac)) {
        if (m_staMap.remove(staMac)) {
            clearStaListLayout();
            initStaDev();
            resetLayoutHight();
        }
    }
}

void ConnectdevPage::resetLayoutHight()
{
    int height = 0;
    for (int i = 0; i < m_staListLayout->count(); i ++) {
        QWidget *w = m_staListLayout->itemAt(i)->widget();
        if (w != nullptr) {
            height += w->height();
        }
    }
    m_staistFrame->setFixedHeight(height);

    if (m_staMap.isEmpty()) {
        this->hide();
    } else {
        this->show();
    }
    this->update();
}

void ConnectdevPage::refreshStalist()
{
    m_staMap.clear();
    getConnectStaDevice(m_staMap);
    clearStaListLayout();
    initStaDev();
    resetLayoutHight();
}

void ConnectdevPage::onDropIntoBlacklistBtnClicked(QString staMac, QString staName)
{
    if (staMac.isNull()
        || staMac.isEmpty()
        || staName.isNull()
        || staName.isEmpty()) {
        qDebug() << LOG_HEAD <<"On drop into blacklist button clicked error! sta mac or name is empty!";
        return;
    }

    emit setStaIntoBlacklist(staMac, staName);
}
