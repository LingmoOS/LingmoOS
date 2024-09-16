// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/namespace.h"

#include <DFrame>
#include <QWidget>
#include <DPalette>
#include <QIcon>

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
class QHBoxLayout;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class TipWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit TipWidget(QWidget *parent = nullptr);

    void setTitle(QString title = "");
    void setIcon(QIcon icon);
signals:



public slots:

private:
    QLabel  *m_tipTitle = nullptr;
    QLabel  *m_leftIcon = nullptr;

};

//#endif // TIPWIDGET_H
