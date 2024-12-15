// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORSETTINGBUTTON_H
#define COLORSETTINGBUTTON_H
#include "attributewidget.h"

class ColorPickWidget;
class CColorSettingButton: public ColorSettingButton
{
    Q_OBJECT
public:
    using ColorSettingButton::ColorSettingButton;
    ColorPickWidget *colorPick();
protected:
    void  mousePressEvent(QMouseEvent *event) override;
private:
    bool        _connectedColorPicker = false;

};


#endif // COLORSETTINGBUTTON_H
