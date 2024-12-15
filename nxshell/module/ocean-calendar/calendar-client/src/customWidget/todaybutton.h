// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TODYBUTTON_H
#define TODYBUTTON_H

#include <DPushButton>

DWIDGET_USE_NAMESPACE
class CTodayButton : public DPushButton
{
    Q_OBJECT
public:
    explicit CTodayButton(QWidget *parent = nullptr);
protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
};

#endif // TODYBUTTON_H
