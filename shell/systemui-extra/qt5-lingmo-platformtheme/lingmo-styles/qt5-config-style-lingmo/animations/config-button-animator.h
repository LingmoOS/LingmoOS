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
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#ifndef BUTTONANIMATOR_H
#define BUTTONANIMATOR_H

#include <QObject>
#include <QVariantAnimation>
#include <QParallelAnimationGroup>

#include "animations/animator-iface.h"




namespace LINGMOConfigStyleSpace {

class ConfigButtonAnimator : public QParallelAnimationGroup, public AnimatorIface
{
    Q_OBJECT
public:
    explicit ConfigButtonAnimator(QObject *parent = nullptr);
    ~ConfigButtonAnimator();

    bool bindWidget(QWidget *w);
    bool unboundWidget();
    QWidget *boundedWidget() {return m_widget;}
    QVariant value(const QString &property);
    bool setAnimatorStartValue(const QString &property, const QVariant &value);
    bool setAnimatorEndValue(const QString &property, const QVariant &value);
    bool setAnimatorDuration(const QString &property, int duration);
    void setAnimatorDirectionForward(const QString &property = nullptr, bool forward = true);
    bool isRunning(const QString &property = nullptr);
    void startAnimator(const QString &property = nullptr);
    void stopAnimator(const QString &property = nullptr);
    int currentAnimatorTime(const QString &property = nullptr);
    void setAnimatorCurrentTime(const QString &property = nullptr, const int msecs = 0);
    int totalAnimationDuration(const QString &property = nullptr);
    QVariant endValue(const QString &property = nullptr);

signals:

private:
    QWidget* m_widget = nullptr;
    QVariantAnimation *m_mouseover;
    QVariantAnimation *m_sunken;

};

}

#endif // BUTTONANIMATOR_H
