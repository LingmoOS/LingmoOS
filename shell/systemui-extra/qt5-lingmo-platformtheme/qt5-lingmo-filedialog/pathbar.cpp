/*
 * KWin Style LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "debug.h"

#include "pathbar.h"
#include <PeonyPathCompleter>
#include <PeonyPathBarModel>
#include <QDebug>
#include <QObject>
#include <QDBusInterface>
#include <QDBusMessage>
#include <explor-qt/file-utils.h>
#include "debug.h"
#include "settings/lingmo-style-settings.h"
using namespace LINGMOFileDialog;

FileDialogPathBar::FileDialogPathBar(QWidget *parent) : QWidget(parent)//Peony::AdvancedLocationBar(parent)//QLineEdit(parent)//
{
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(0,0,0,0);
    m_pathBar = new Peony::AdvancedLocationBar(this);

    QDBusInterface *interFace = new QDBusInterface(SERVICE, PATH, INTERFACE, QDBusConnection::sessionBus());
    if(interFace->isValid()){
        connect(interFace, SIGNAL(mode_change_signal(bool)), this, SLOT(updateTableModel(bool)));
    }


    QDBusMessage message = QDBusMessage::createMethodCall("com.lingmo.statusmanager.interface",
                                                           "/",
                                                           "com.lingmo.statusmanager.interface",
                                                           "get_current_tabletmode");
    QDBusMessage ret = QDBusConnection::sessionBus().call(message);
    if (ret.type() != QDBusMessage::ReplyMessage)
    {
        //从返回参数获取返回值
        pDebug << "complex type failed!";
        updateTableModel(false);
    }
    else
    {
        updateTableModel(ret.arguments()[0].value<bool>());
    }
}

FileDialogPathBar::~FileDialogPathBar()
{
    m_pathBar->deleteLater();
    m_pathBar = nullptr;
}

void FileDialogPathBar::updatePath(const QString &uri)
{
    pDebug << "m_pathBar........ updatePath:" << uri << Peony::FileUtils::getEncodedUri(uri);
    m_pathBar->updateLocation(Peony::FileUtils::urlEncode(uri));
//    setText(uri);
//    clearFocus();
}

void FileDialogPathBar::resizeEvent(QResizeEvent *e)
{
    m_pathBar->setFixedWidth(this->width());
    m_pathBar->setGeometry(0, (this->height() - m_pathBar->height()) / 2, this->width(), m_pathBar->height());
}

Peony::AdvancedLocationBar *FileDialogPathBar::getPathBar()
{
    return m_pathBar;
}

void FileDialogPathBar::updateTableModel(bool isTable)
{
    pDebug << "updateTableModel...." << isTable;
//    m_pathBar->updateTabletModeValue(isTable);
    if(isTable){
        m_pathBar->setFixedHeight(48);
        this->setFixedHeight(48);
    }
    else{
        m_pathBar->setFixedHeight(36);
        this->setFixedHeight(36);
    }
    pDebug << "m_pathBar height1111:" << m_pathBar->height() << this->height();
}
