/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */
#ifndef SCREENTYPE_H
#define SCREENTYPE_H
#include <QWidget>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QButtonGroup>
class ScreenType : public QWidget
{
    Q_OBJECT
public:
    //ScreenType();
    explicit ScreenType(QWidget *parent = nullptr);
    QButtonGroup *button_type;
    QVBoxLayout *vbox;
    QRadioButton *MP4_TYPE,*GIF_TYPE,*MKV_TYPE,*WEBM_TYPE,*OGG_TYPE;
    QString type;
public slots:
     void onRadioClickType();
signals:
     void Type_change(QString s);
};

#endif // SCREENTYPE_H
