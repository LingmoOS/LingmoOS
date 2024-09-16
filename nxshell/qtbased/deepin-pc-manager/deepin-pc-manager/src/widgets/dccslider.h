// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFSLIDER_H
#define DEFSLIDER_H

#include <DSlider>

namespace def {
namespace widgets {

class DCCSlider : public DTK_WIDGET_NAMESPACE::DSlider
{
    Q_OBJECT
public:
    enum SliderType { Normal, Vernier, Progress };

public:
    explicit DCCSlider(SliderType type = Normal, QWidget *parent = nullptr);
    explicit DCCSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

    inline DCCSlider *slider() const { return const_cast<DCCSlider *>(this); }

    QSlider *qtSlider();

    void setType(SliderType type);
    void setRange(int min, int max);
    void setTickPosition(QSlider::TickPosition tick);
    void setTickInterval(int ti);
    void setSliderPosition(int Position);
    void setAnnotations(const QStringList &annotations);
    void setOrientation(Qt::Orientation orientation);

protected:
    void wheelEvent(QWheelEvent *e);

private:
    QSlider::TickPosition tickPosition = QSlider::TicksBelow;
};

} // namespace widgets
} // namespace def

#endif // DEFSLIDER_H
