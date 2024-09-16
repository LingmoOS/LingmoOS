// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>
#include <QHBoxLayout>

class ToolBar : public QWidget
{

public:
    explicit ToolBar(QWidget *parent = nullptr);
    ~ToolBar();

    void setTabbar(QWidget *w);

private:
    QHBoxLayout *m_layout {nullptr};
};

#endif
