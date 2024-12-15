// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PRIORITY_SLIDER_H
#define PRIORITY_SLIDER_H

#include <DSlider>

DWIDGET_USE_NAMESPACE

/**
 * @brief Priority slider sub class
 */
class PrioritySlider : public DSlider
{
    Q_OBJECT

public:
    /**
     * @brief PrioritySlider Constructor
     * @param orientation Slider orientation
     * @param parent Parent object
     */
    PrioritySlider(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = nullptr);

Q_SIGNALS:
    /**
     * @brief sizeChanged Size changed signal
     * @param size New size
     */
    void sizeChanged(const QSize &size);

protected:
    /**
     * @brief resizeEvent Resize event handler
     * @param event Resize event
     */
    void resizeEvent(QResizeEvent *event) override;
};

#endif  // PRIORITY_SLIDER_H
