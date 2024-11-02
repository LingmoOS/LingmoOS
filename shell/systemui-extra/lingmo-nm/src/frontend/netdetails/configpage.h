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
#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include <QWidget>
#include <QFrame>
#include <QLayout>
#include <QFormLayout>
#include <QRadioButton>
#include <QLabel>

#include "kwidget.h"
#include "kborderlessbutton.h"

using namespace kdk;

class ConfigPage : public QFrame
{
    Q_OBJECT
public:
    ConfigPage(QWidget *parent = nullptr);
    ~ConfigPage() = default;

    void setConfigState(int type);
    bool checkIsChanged(int type);
    int getConfigState();

private:
    void initUi();
    void initComponent();

    QLabel *m_descriptionLabel = nullptr;
    QRadioButton *m_publicButton = nullptr;
    QRadioButton *m_privateButton = nullptr;
    QLabel *m_publicLabel = nullptr;
    QLabel *m_privateLabel = nullptr;
    KBorderlessButton *m_congigBtn = nullptr;
    QVBoxLayout *m_vBoxLayout  = nullptr;

private Q_SLOTS:
    void onConfigButtonClicked();

Q_SIGNALS:
    void publicConfig();
    void privateConfig();
};

#endif // CONFIGPAGE_H
