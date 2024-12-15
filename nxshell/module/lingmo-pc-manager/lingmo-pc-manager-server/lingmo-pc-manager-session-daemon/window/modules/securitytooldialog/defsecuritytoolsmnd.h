// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFSECURITYTOOLSMND_H
#define DEFSECURITYTOOLSMND_H

#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class DefSecurityToolsMnd : public DMainWindow
{
    Q_OBJECT
public:
    explicit DefSecurityToolsMnd(QWidget *pWidget = nullptr);
    virtual ~DefSecurityToolsMnd() override;

protected:
    void addContent(QWidget *pWidget);
    void moveToCenter();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    void initUI();
};

#endif // DEFSECURITYTOOLSMND_H
