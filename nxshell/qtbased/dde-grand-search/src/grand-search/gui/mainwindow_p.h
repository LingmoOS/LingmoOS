// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_P_H
#define MAINWINDOW_P_H

#include "mainwindow.h"

#include <QWidget>


DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE
class QVBoxLayout;

namespace GrandSearch {
class EntranceWidget;
class ExhibitionWidget;
class HandleVisibility;
class QueryController;
class MatchController;

class MainWindowPrivate
{
public:
    explicit MainWindowPrivate(MainWindow *parent);

    MainWindow *q_p = nullptr;
    HandleVisibility *m_handleVisibility = nullptr;
    EntranceWidget *m_entranceWidget = nullptr;
    ExhibitionWidget *m_exhibitionWidget = nullptr;
    Dtk::Widget::DLabel *m_searchNoContentWidget = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QueryController *m_queryController = nullptr;
    MatchController *m_matchController = nullptr;
};

}

#endif // MAINWINDOW_P_H
