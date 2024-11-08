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

#ifndef CHECKBOXANIMATOR_H
#define CHECKBOXANIMATOR_H

#include <QObject>
#include <QVariantAnimation>
#include <QParallelAnimationGroup>

#include "animations/animator-iface.h"


namespace LINGMOConfigStyleSpace {

class ConfigCheckBoxAnimator : public QParallelAnimationGroup, public AnimatorIface
{
    Q_OBJECT
public:
    explicit ConfigCheckBoxAnimator(QObject *parent = nullptr);
    ~ConfigCheckBoxAnimator();

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
    void setAnimatorCurrentTime(const QString &property, const int msecs);
    int totalAnimationDuration(const QString &property = nullptr);
    void setExtraProperty(const QString &property, const QVariant &value);
    QVariant getExtraProperty(const QString &property);

signals:

private:
    QWidget* m_widget = nullptr;
    QVariantAnimation *m_onScale = nullptr;
    QVariantAnimation *m_onOpacity = nullptr;
    QVariantAnimation *m_onBase = nullptr;
    QVariantAnimation *m_off = nullptr;
    bool m_noChange = false;
};
}


#endif // CHECKBOXANIMATOR_H
