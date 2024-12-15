// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <DMainWindow>
#include <DWidget>

#include <QVBoxLayout>

namespace GrandSearch {

class ScopeWidget;
class CustomWidget;
class IndexWidget;
class ConfigWidget : public Dtk::Widget::DMainWindow
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = nullptr);
    ~ConfigWidget();

    void scrollToAiConfig();

private:
    void initUI();
    void initData();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    QVBoxLayout *m_mainLayout                       = nullptr;      // 界面主体布局
    Dtk::Widget::DScrollArea *m_scrollArea          = nullptr;      // 滚动区域部件
    Dtk::Widget::DWidget *m_scrollAreaContent       = nullptr;      // 滚动区域内容部件
    QVBoxLayout *m_scrollLayout                     = nullptr;      // 滚动区域内部部件布局

    ScopeWidget *m_searchGroupWidget          = nullptr;
    CustomWidget *m_searchCustomWidget        = nullptr;
    IndexWidget *m_indexWidget        = nullptr;
};

}

#endif // CONFIGWIDGET_H
