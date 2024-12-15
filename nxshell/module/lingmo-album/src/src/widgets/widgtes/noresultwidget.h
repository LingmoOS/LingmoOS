// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NORESULTWIDGET_H
#define NORESULTWIDGET_H


#include <QWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class NoResultWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoResultWidget(QWidget *parent);
    ~NoResultWidget()override;

protected:
    void showEvent(QShowEvent *ev) override;
private:
    void initNoSearchResultView();
    void changeTheme();

private:
    DLabel *pNoResult;
};

#endif // NORESULTWIDGET_H
