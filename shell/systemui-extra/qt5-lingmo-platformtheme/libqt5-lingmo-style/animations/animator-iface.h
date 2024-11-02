/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef ANIMATORIFACE_H
#define ANIMATORIFACE_H

#include "animator-plugin-iface.h"
#include <QVariant>

class QWidget;

class AnimatorIface
{
public:
    virtual ~AnimatorIface() {}

    virtual bool bindWidget(QWidget *w) {return false;}
    virtual bool unboundWidget() {return false;}
    virtual QWidget *boundedWidget() {return nullptr;}

    virtual QVariant value(const QString &property) {return QVariant();}
    virtual bool setAnimatorStartValue(const QString &property, const QVariant &value) {return false;}
    virtual bool setAnimatorEndValue(const QString &property, const QVariant &value) {return false;}
    virtual QVariant animatorStartValue(const QString &property) {return QVariant();}
    virtual QVariant animatorEndValue(const QString &property) {return QVariant();}
    virtual bool setAnimatorDuration(const QString &property, int duration) {return false;}

    virtual void setAnimatorDirectionForward(const QString &property = nullptr, bool forward = true) {}
    virtual bool isRunning(const QString &property = nullptr) {return false;}
    virtual void startAnimator(const QString &property = nullptr) {}
    virtual void stopAnimator(const QString &property = nullptr) {}
    virtual int currentAnimatorTime(const QString &property = nullptr) {return 0;}
    virtual void setAnimatorCurrentTime(const QString &property = nullptr, const int msecs = 0) {}
    virtual int totalAnimationDuration(const QString &property = nullptr) {return 0;}

    virtual void setExtraProperty(const QString &property, const QVariant &value){}
    virtual QVariant getExtraProperty(const QString &property) {return QVariant();}

};

#endif // ANIMATORIFACE_H
