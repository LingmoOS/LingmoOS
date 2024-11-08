/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "klineframe.h"
#include "themeController.h"
#include <QApplication>
#include <QDebug>

namespace kdk {
class KHLineFramePrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KHLineFrame)

public:
    KHLineFramePrivate(KHLineFrame* parent);

    void changeTheme();
private:
    KHLineFrame* q_ptr;
    bool m_isFollowPalette;
};

class KVLineFramePrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KVLineFrame)

public:
    KVLineFramePrivate(KVLineFrame* parent);

    void changeTheme();
private:
    KVLineFrame* q_ptr;
    bool m_isFollowPalette;
};

KHLineFrame::KHLineFrame(QWidget *parent)
    :QFrame(parent)
  ,d_ptr(new KHLineFramePrivate(this))
{
    setContentsMargins(0,10,0,10);
    this->setFrameShape(QFrame::Shape::HLine);
    setLineWidth(1);
    QPalette palette =qApp->palette();
    QColor color(ThemeController::mixColor(Qt::gray,Qt::white,0.1));
    color.setAlphaF(0.2);
    palette.setColor(QPalette::Window,color);
    setPalette(palette);
    setFixedHeight(1);
    setAutoFillBackground(true);
}

KHLineFrame::~KHLineFrame()
{

}

void KHLineFrame::setFollowPalette(bool flag)
{
    Q_D(KHLineFrame);
    d->m_isFollowPalette = flag;
}

KHLineFramePrivate::KHLineFramePrivate(KHLineFrame *parent)
    :q_ptr(parent)
{
    Q_Q(KHLineFrame);
    m_isFollowPalette = true;
    connect(m_gsetting,&QGSettings::changed,this,[=](){
        changeTheme();
    });
}

void KHLineFramePrivate::changeTheme()
{
    Q_Q(KHLineFrame);
    if(m_isFollowPalette)
    {
        if(ThemeController::themeMode() == LightTheme)
        {
            QPalette palette =qApp->palette();
             QColor color(ThemeController::mixColor(Qt::gray,Qt::white,0.1));
             color.setAlphaF(0.2);
             palette.setColor(QPalette::Window,color);
             q->setPalette(palette);
        }
        else
        {
            QPalette palette =qApp->palette();
             QColor color(ThemeController::mixColor(Qt::gray,Qt::white,0.1));
             color.setAlphaF(0.3);
             palette.setColor(QPalette::Window,color);
             q->setPalette(palette);
        }
    }
}

KVLineFrame::KVLineFrame(QWidget *parent)
    :QFrame(parent)
    ,d_ptr(new KVLineFramePrivate(this))
{
    setContentsMargins(10,0,10,0);
    setLineWidth(1);
    this->setFrameShape(QFrame::Shape::VLine);
    QPalette palette =qApp->palette();
    QColor color(ThemeController::mixColor(Qt::gray,Qt::white,0.1));
    color.setAlphaF(0.2);
    palette.setColor(QPalette::Window,color);
    setPalette(palette);
    setFixedWidth(1);
    setAutoFillBackground(true);
}

KVLineFrame::~KVLineFrame()
{

}

void KVLineFrame::setFollowPalette(bool flag)
{
    Q_D(KVLineFrame);
    d->m_isFollowPalette = flag;
}

KVLineFramePrivate::KVLineFramePrivate(KVLineFrame *parent)
    :q_ptr(parent)
{
    Q_Q(KVLineFrame);
    m_isFollowPalette = true;
    connect(m_gsetting,&QGSettings::changed,this,[=](){
        changeTheme();
    });
}

void KVLineFramePrivate::changeTheme()
{
    Q_Q(KVLineFrame);
    if(m_isFollowPalette)
    {
        if(ThemeController::themeMode() == LightTheme)
        {
            QPalette palette =qApp->palette();
             QColor color(ThemeController::mixColor(Qt::gray,Qt::white,0.1));
             color.setAlphaF(0.2);
             palette.setColor(QPalette::Window,color);
             q->setPalette(palette);
        }
        else
        {
            QPalette palette =qApp->palette();
             QColor color(ThemeController::mixColor(Qt::gray,Qt::white,0.1));
             color.setAlphaF(0.3);
             palette.setColor(QPalette::Window,color);
             q->setPalette(palette);
        }
    }
}

}


#include "klineframe.moc"
#include "moc_klineframe.cpp"
