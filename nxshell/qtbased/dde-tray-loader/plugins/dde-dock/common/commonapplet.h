// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AIRPLANEMODEAPPLET_H
#define AIRPLANEMODEAPPLET_H

#include "jumpsettingbutton.h"

#include <QWidget>

#include <DSwitchButton>
#include <DLabel>

class CommonApplet : public QWidget
{
    Q_OBJECT

public:
    explicit CommonApplet(QWidget *parent = nullptr);
    void setEnabled(bool enable);
    void setTitle(const QString &title);
    void setIcon(const QIcon &icon);
    void setDescription(const QString &description);
    void setDccPage(const QString &first, const QString &second);
    void hideSettingButton();

signals:
    void enableChanged(bool enable);
    void requestHideApplet();

private:
    Dtk::Widget::DLabel *m_title;
    Dtk::Widget::DSwitchButton *m_switchBtn;
    JumpSettingButton *m_settingButton;
};

#endif // AIRPLANEMODEAPPLET_H
