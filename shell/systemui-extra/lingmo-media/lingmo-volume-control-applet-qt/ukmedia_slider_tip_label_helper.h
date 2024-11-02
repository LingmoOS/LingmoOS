/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#ifndef SLIDERTIPLABELHELPER_H
#define SLIDERTIPLABELHELPER_H

#include <QObject>
#include <QLabel>
#include "ukmedia_custom_class.h"

class MediaSliderTipLabel:public QLabel
{
  public:
    MediaSliderTipLabel();
    ~MediaSliderTipLabel();
protected:
    void paintEvent(QPaintEvent*);
};

class SliderTipLabelHelper : public QObject
{
    Q_OBJECT
    friend class AppEventFilter;
public:
    SliderTipLabelHelper(QObject *parent = nullptr);
    ~SliderTipLabelHelper() {}
    void registerWidget(QWidget *w);
    void unregisterWidget(QWidget *w);
    bool eventFilter(QObject *obj, QEvent *e);

    void mouseMoveEvent(QObject *obj, QMouseEvent *e);
    void mouseReleaseEvent(QObject *obj, QMouseEvent *e);
    void mousePressedEvent(QObject *obj,QMouseEvent *e);
private:
    MediaSliderTipLabel *m_pTiplabel;
};

class AppEventFilter
{
    friend class SliderTipLabelHelper;
private:
    explicit AppEventFilter();
    ~AppEventFilter() {}

    bool eventFilter(QObject *obj, QEvent *e);

    SliderTipLabelHelper *m_wm = nullptr;
};

#endif // SLIDERTIPLABELHELPER_H
