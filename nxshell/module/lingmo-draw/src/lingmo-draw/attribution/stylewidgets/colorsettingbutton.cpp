// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorsettingbutton.h"
#include "colorpickwidget.h"

ColorPickWidget *CColorSettingButton::colorPick()
{
    static ColorPickWidget *cp = nullptr;
    if (cp == nullptr) {
        cp = new ColorPickWidget(nullptr);
    }
    return cp;
}

void CColorSettingButton::mousePressEvent(QMouseEvent *event)
{
    QMetaObject::invokeMethod(this, [ = ]() {
        ColorPickWidget *colorPick = this->colorPick();
        if (!_connectedColorPicker) {
            connect(colorPick, &ColorPickWidget::colorChanged, this, [ = ](const QColor & color, int phase) {
                if (colorPick->caller() == this)
                    setColor(color, phase);
            });
            _connectedColorPicker = true;
        }

        colorPick->setCaller(this);
        colorPick->setColor(getColor(), EChanged);

        QPoint btnPos = mapToGlobal(pos());
        QPoint pos(btnPos.x() + this->width() - 30,
                   btnPos.y() - 10);

        colorPick->setShowPos(pos);
        colorPick->show(pos.x(), pos.y());

    }, Qt::QueuedConnection);
    event->accept();
}
