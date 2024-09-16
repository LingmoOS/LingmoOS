// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INFOCOMMANDLINKBUTTON_H
#define INFOCOMMANDLINKBUTTON_H
#include <DCommandLinkButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class InfoCommandLinkButton : public DCommandLinkButton
{
public:
    explicit InfoCommandLinkButton(QString text, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif  // INFOCOMMANDLINKBUTTON_H
