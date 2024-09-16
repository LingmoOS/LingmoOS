// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUTTONTUPLE_H
#define BUTTONTUPLE_H

#include <DSuggestButton>
#include <DPushButton>
#include <DWarningButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxLeftButton : public DPushButton
{
    Q_OBJECT
public:
    FcitxLeftButton() {}
    virtual ~FcitxLeftButton() {}
};

class FcitxRightButton : public DSuggestButton
{
    Q_OBJECT
public:
    FcitxRightButton() {}
    virtual ~FcitxRightButton() {}
};

class FcitxButtonTuple : public QWidget
{
    Q_OBJECT
public:
    enum ButtonType {
        Normal = 0,
        Save = 1,
        Delete = 2,
        None = 3,
    };

    explicit FcitxButtonTuple(ButtonType type = Normal, QWidget *parent = nullptr);

    QPushButton *leftButton();
    QPushButton *rightButton();

    void removeSpacing();

private:
    void createRightButton(const ButtonType type);

Q_SIGNALS:
    void leftButtonClicked();
    void rightButtonClicked();

private:
    QPushButton *m_leftButton;
    QPushButton *m_rightButton;
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // BUTTONTUPLE_H
