// Copyright (C) 2021 ~ 2021 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYLABEL_H
#define KEYLABEL_H

#include <QLabel>
#include <DLabel>
#include <DFrame>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <DApplicationHelper>
#include "publisher/publisherfunc.h"

using namespace Dtk::Widget;
namespace oceanui_fcitx_configtool {
namespace widgets {
class FcitxKeyLabel : public DFrame
{
public:
    FcitxKeyLabel(QString text, QWidget *parent = nullptr)
        : DFrame(parent)
    {
        label = new DLabel(text);
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(layout->contentsMargins().left(), 0, layout->contentsMargins().right(), 0);
        layout->setSpacing(0);
        label->setForegroundRole(DPalette::LightLively);
        layout->addWidget(label);
    }

    QString text() { return label->text(); }

protected:
    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)
        QStyleOptionFrame opt;
        initStyleOption(&opt);
        QPainter p(this);
        drawShadow(&p, event->rect() - contentsMargins(), QColor(0, 0, 0, 20));

        opt.features |= QStyleOptionFrame::Rounded;

        const DPalette &dp = DApplicationHelper::instance()->palette(this);

        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            p.setBackground(QColor(255, 255, 255));
        } else {
            p.setBackground(QColor(40, 40, 40));
        }

        p.setPen(QPen(dp.frameBorder(), opt.lineWidth));
        style()->drawControl(QStyle::CE_ShapedFrame, &opt, &p, this);
    }

    void drawShadow(QPainter *p, const QRect &rect, const QColor &color) const
    {
        DStyle dstyle;
        int frame_radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);
        int shadow_xoffset = dstyle.pixelMetric(DStyle::PM_ShadowHOffset);
        int shadow_yoffset = dstyle.pixelMetric(DStyle::PM_ShadowVOffset);

        QRect shadow = rect;
        QPoint pointOffset(rect.center().x() + shadow_xoffset, rect.center().y() + shadow_yoffset);
        shadow.moveCenter(pointOffset);

        p->setBrush(color);
        p->setPen(Qt::NoPen);
        p->setRenderHint(QPainter::Antialiasing);
        p->drawRoundedRect(shadow, frame_radius, frame_radius);
    }

private:
    DLabel *label;
};
} // namespace widgets
} // namespace oceanui_fcitx_configtool
#endif // KEYLABEL_H
