// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_SERVICE_PAGE_WIDGET_H
#define SYSTEM_SERVICE_PAGE_WIDGET_H

#include <DFrame>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class MainWindow;
class SystemServiceTableView;

/**
 * @brief Service background frame widget
 */
class SystemServicePageWidget : public DFrame
{
public:
    /**
     * @brief Constrcutor
     * @param parent Parent object
     */
    explicit SystemServicePageWidget(DWidget *parent = nullptr);
    /**
    * @brief Destructor
    */
    ~SystemServicePageWidget();

protected:
    /**
     * @brief paintEvent Paint event handler
     * @param event Paint event
     */
    void paintEvent(QPaintEvent *event);

public Q_SLOTS:

Q_SIGNALS:

private:
    // Service table view instance
    SystemServiceTableView *m_svcTableView {};
};

#endif  // SYSTEM_SERVICE_PAGE_WIDGET_H
