// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXWIDGET_H
#define INDEXWIDGET_H

#include <DWidget>

#include <QVBoxLayout>
#include <QLabel>

namespace GrandSearch {

class BlackListWidget;
class IntelligentRetrievalWidget;
class IndexWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit IndexWidget(QWidget *parent = nullptr);
    ~IndexWidget();

    bool onCloseEvent();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    BlackListWidget *m_blackListWidget = nullptr;
    IntelligentRetrievalWidget *m_intelligent = nullptr;
};

}
#endif // INDEXWIDGET_H
