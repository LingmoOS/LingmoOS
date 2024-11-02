/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "loadinglabel.h"

LoadingLabel::LoadingLabel(QObject *parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    connect(m_timer,&QTimer::timeout,this,&LoadingLabel::refreshIconNum);
    initGsettings();
}

LoadingLabel::~LoadingLabel()
{
    if (m_timer)
        m_timer->deleteLater();
}

void LoadingLabel::setTimerStop()
{  
    m_timer->stop();
}

void LoadingLabel::setTimerStart()
{
    if (!m_timer->isActive())
    {
        index = 0;
        m_timer->start();
    }
}

void LoadingLabel::setTimeReresh(int m)
{
    m_timer->setInterval(m);
}

void LoadingLabel::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::transparent);

    if (_themeIsBlack)
        painter.drawImage(0,0,
                      ukccbluetoothconfig::loadSvgImage(QIcon::fromTheme("lingmo-loading-"+QString("%1").arg(index)+"-symbolic").pixmap(this->width(),this->height()),ukccbluetoothconfig::WHITE),
                      0,0,-1,-1,Qt::ColorMode_Mask);
    else
        painter.drawPixmap(0,0,this->width(),this->height(),QIcon::fromTheme("lingmo-loading-"+QString("%1").arg(index)+"-symbolic").pixmap(this->width(),this->height()));

}

void LoadingLabel::refreshIconNum()
{
    index++;
    if(index > 7 || index < 0)
        index = 0;
    this->update();
}

void LoadingLabel::initGsettings()
{
//    qWarning()<< Q_FUNC_INFO << __LINE__;
    if (QGSettings::isSchemaInstalled(GSETTING_LINGMO_STYLE))
    {
        _mStyle_GSettings = new QGSettings(GSETTING_LINGMO_STYLE);

        if(_mStyle_GSettings->get("styleName").toString() == "lingmo-default" ||
           _mStyle_GSettings->get("style-name").toString() == "lingmo-light")
            _themeIsBlack = false;
        else
            _themeIsBlack = true;

        //_mIconThemeName = _mStyle_GSettings->get("iconThemeName").toString();
    }

    connect(_mStyle_GSettings,&QGSettings::changed,this,&LoadingLabel::mStyle_GSettingsSlot);
}

void LoadingLabel::mStyle_GSettingsSlot(const QString &key)
{
//    qWarning()<< Q_FUNC_INFO << key << __LINE__;

    if ("styleName" == key || "style-name" == key)
    {
        if(_mStyle_GSettings->get("style-name").toString() == "lingmo-default" ||
           _mStyle_GSettings->get("style-name").toString() == "lingmo-light")
        {
            _themeIsBlack = false;
        }
        else
        {
            _themeIsBlack = true;
        }
    }
}
