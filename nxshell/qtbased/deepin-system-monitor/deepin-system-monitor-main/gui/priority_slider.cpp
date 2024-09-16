// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "priority_slider.h"

#include <QResizeEvent>

// constructor
PrioritySlider::PrioritySlider(Qt::Orientation orientation, QWidget *parent)
    : DSlider(orientation, parent)
{
}

// resize event handler
void PrioritySlider::resizeEvent(QResizeEvent *event)
{
    DSlider::resizeEvent(event);
    // emit size changed signal
    Q_EMIT sizeChanged(event->size());
}
