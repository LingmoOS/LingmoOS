// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "cylinderwidget.h"

#include <QDebug>
#include <QMouseEvent>

CylinderWidget::CylinderWidget(QWidget *parent) : QLabel(parent)
{

}

void CylinderWidget::setUserData(const QVariant &userData)
{
    m_userData = userData;
}

QVariant CylinderWidget::getUserData()
{
    return m_userData;
}

void CylinderWidget::enterEvent(QEvent *event)
{
    emit enter();

    return QLabel::enterEvent(event);
}

void CylinderWidget::leaveEvent(QEvent *event)
{
    emit leave();

    return QLabel::leaveEvent(event);
}


