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

#ifndef DEVRENAMEDIALOG_H
#define DEVRENAMEDIALOG_H

#include <QIcon>
#include <QLabel>
#include <QDebug>
#include <QDialog>
#include <QVariant>
#include <QPalette>
#include <QPainter>
#include <QLineEdit>
#include <QKeyEvent>
#include <QGSettings>
#include <QPushButton>
#include <QHBoxLayout>

#include <ukcc/widgets/titlelabel.h>

#include "config.h"

#define INPUT_STRNAME_MIN 1
#define INPUT_STRNAME_MAX 32

class DevRenameDialog : public QDialog
{
    Q_OBJECT
public:
    enum DEV_RENAME_DIALOG_TYPE
    {
        DEVRENAMEDIALOG_ADAPTER = 0,
        DEVRENAMEDIALOG_BT_DEVICE = 1
    };
    Q_ENUM(DEV_RENAME_DIALOG_TYPE)

    explicit DevRenameDialog(QWidget *parent = nullptr);

    ~DevRenameDialog();

    void setDevName(const QString &);
    void setRenameInterface(DEV_RENAME_DIALOG_TYPE mode);

    void initGsettings();

    QLabel * titleLabel = nullptr;
    QLabel * textLabel  = nullptr;
    DEV_RENAME_DIALOG_TYPE _mMode = DEVRENAMEDIALOG_ADAPTER;
private slots:
    void gsettingsSlot(const QString &);

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);

public slots:
    void lineEditSlot(const QString &);

signals:
    void nameChanged(QString);

private:
    void initUI();

    int  _fontSize;

    bool isblack = false;

    QString adapterOldName;

    QLabel *tipLabel   = nullptr;

    QGSettings *gsettings  = nullptr;

    QPushButton *closeBtn  = nullptr;

    QPushButton *acceptBtn = nullptr;

    QPushButton *rejectBtn = nullptr;

    QLineEdit   *lineEdit  = nullptr;
};

#endif // DEVRENAMEDIALOG_H
