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

#ifndef DEVREMOVEDIALOG_H
#define DEVREMOVEDIALOG_H

#include <QFontMetrics>
#include <QPushButton>
#include <QGSettings>
#include <QVariant>
#include <QPalette>
#include <QPainter>
#include <QDialog>
#include <QColor>
#include <QLabel>
#include <QDebug>
#include <QIcon>
#include <QFont>

#include "config.h"

class DevRemoveDialog final : public QDialog
{
    Q_OBJECT
public:
    enum REMOVE_INTERFACE_TYPE
    {
        REMOVE_NO_PIN_DEV = 0,
        REMOVE_HAS_PIN_DEV = 1,
        REMOVE_MANY_TIMES_CONN_FAIL_DEV = 2
    };
    Q_ENUM(REMOVE_INTERFACE_TYPE)

    explicit DevRemoveDialog(REMOVE_INTERFACE_TYPE mode , QWidget *parent = nullptr);

    ~DevRemoveDialog();

    void initUI();

    void initGsettings();

    void setDialogText(const QString & );

private slots:
    void gsettingsSlot(const QString &);

protected:
    void paintEvent(QPaintEvent *);

private:
    bool isblack = false;

    QLabel *tipLabel  = nullptr;

    QLabel *txtLabel  = nullptr;

    QLabel *iconLabel = nullptr;

    QLabel *title_icon = nullptr;

    QLabel *title_text = nullptr;

    QGSettings *gsettings  = nullptr;

    QPushButton *closeBtn  = nullptr;

    QPushButton *acceptBtn = nullptr;

    QPushButton *rejectBtn = nullptr;

    REMOVE_INTERFACE_TYPE m_mode = REMOVE_HAS_PIN_DEV;
};

#endif // DEVREMOVEDIALOG_H
