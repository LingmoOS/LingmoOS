/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KGROUPBUTTON_H
#define KGROUPBUTTON_H

#include "../style/usmproxystyle.h"
#include <QPushButton>

class KGroupButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KGroupButton(QWidget *parent = nullptr);
    explicit KGroupButton(const QString &text, QWidget *parent = nullptr);
    KGroupButton(const QIcon& icon, const QString &text, QWidget *parent = nullptr);

    inline GroupButtonStyleOption::ButtonPosition position()
    { return mPosition; }
    inline void setPosition(GroupButtonStyleOption::ButtonPosition pos)
    { mPosition = pos; }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    GroupButtonStyleOption::ButtonPosition mPosition;
};

#endif // KGROUPBUTTON_H
