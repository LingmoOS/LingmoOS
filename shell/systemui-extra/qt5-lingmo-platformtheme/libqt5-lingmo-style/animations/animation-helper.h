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

#ifndef ANIMATIONHELPER_H
#define ANIMATIONHELPER_H

#include <QObject>

class QWidget;
class AnimatorIface;

class AnimationHelper : public QObject
{
    Q_OBJECT
public:
    explicit AnimationHelper(QObject *parent = nullptr);
    virtual ~AnimationHelper();

signals:

public slots:
    virtual bool registerWidget(QWidget *) {return false;}
    virtual bool unregisterWidget(QWidget *) {return false;}

protected:
    /*!
     * \brief m_animators
     * \deprecated
     * You should not use this member in newly-written code.
     */
    QHash<const QWidget *, AnimatorIface*> *m_animators = nullptr;
};

#endif // ANIMATIONHELPER_H
