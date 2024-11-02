
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef DROWNLABEL_H
#define DROWNLABEL_H

#include <QObject>
#include <QLabel>
#include <QWidget>
#include <QMouseEvent>

class DrownLabel : public QLabel
{
    Q_OBJECT
public:
    explicit DrownLabel(QString devName, QWidget * parent = nullptr);
    ~DrownLabel();
    void setDropDownStatus(bool status);
     QString m_devName;
    bool isChecked = true;
private:
    void loadPixmap(bool isChecked);
protected:
    virtual void mouseReleaseEvent(QMouseEvent * event);

Q_SIGNALS:
    void labelClicked();
};

#endif // DROWNLABEL_H
