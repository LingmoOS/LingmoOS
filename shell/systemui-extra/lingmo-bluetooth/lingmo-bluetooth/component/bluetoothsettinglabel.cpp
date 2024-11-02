/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "bluetoothsettinglabel.h"

BluetoothSettingLabel::BluetoothSettingLabel(QWidget *parent) : QLabel(parent)
{
    _MStatus = Normal;
    this->setFixedSize(420,50);
    if(QGSettings::isSchemaInstalled("org.lingmo.style")){
        StyleSettings = new QGSettings("org.lingmo.style");
        connect(StyleSettings,&QGSettings::changed,this,[=]{
            this->update();
        });
    }
    const QByteArray transparency_id(TRANSPARENCY_SETTINGS);
    if(QGSettings::isSchemaInstalled(transparency_id)){
        transparency_gsettings = new QGSettings(transparency_id);
    }
    if(QGSettings::isSchemaInstalled(transparency_id)){
       tran=transparency_gsettings->get(TRANSPARENCY_KEY).toDouble()*255;
       connect(transparency_gsettings, &QGSettings::changed, this, [=] {
           tran=transparency_gsettings->get(TRANSPARENCY_KEY).toDouble()*255;
           this->update();
       });
    }
}

BluetoothSettingLabel::~BluetoothSettingLabel() {

}

QColor BluetoothSettingLabel::getPainterBrushColor()
{
    QColor color;
    switch (_MStatus) {
    case Status::Normal:
        if (StyleSettings->get("style-name").toString() == "lingmo-default")
            color = QColor(217,217,217);
        else
            color = this->palette().color(QPalette::Active, QPalette::ButtonText);
        break;
    case Status::Hover:
        color = this->palette().color(QPalette::Active, QPalette::Highlight);
        break;
    case Status::Click:
        color = this->palette().color(QPalette::Active, QPalette::Highlight).darker(105);
        break;
    default:
        color = this->palette().color(QPalette::Active, QPalette::ButtonText);
        break;
    }
    return color;
}

void BluetoothSettingLabel::setFocusStyle(bool val) {
    focused = val;
    if(val)
        _MStatus = Hover;
    else
        _MStatus = Normal;
    this->update();
}

void BluetoothSettingLabel::enterEvent(QEvent *event) {
    Q_UNUSED(event);
    if (focused)
        return;
    _MStatus = Hover;
    setCursor(Qt::PointingHandCursor);
    this->update();
}

void BluetoothSettingLabel::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    if (focused)
        return;
    _MStatus = Normal;
    this->update();
}

void BluetoothSettingLabel::mousePressEvent(QMouseEvent *ev) {
    Q_UNUSED(ev);
    _MStatus = Click;
    this->update();
}

void BluetoothSettingLabel::mouseReleaseEvent(QMouseEvent *ev) {
    Q_UNUSED(ev);
    _MStatus = Normal;
    this->update();
//    Config::OpenBluetoothSettings();
    emit clickSignal();
    emit hideFeature();
}

void BluetoothSettingLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(Qt::transparent));
    painter.drawRoundedRect(this->rect(),12,12,Qt::AbsoluteSize);
    //painter.drawRect(this->rect());
    painter.restore();
    painter.setPen(getPainterBrushColor());
    painter.drawText(25,10,420,50,Qt::AlignLeft,tr("Bluetooth Settings"));

    //焦点预选框
    if (!focused)
        return;
    QPainter pt(this);
    QColor color = QColor("#818181");
    pt.setPen(color);
    QRect rect = this->rect();
    rect.setWidth(this->rect().width());
    rect.setHeight(this->rect().height() - 1);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.drawRoundedRect(rect,12,12,Qt::AbsoluteSize);
}
