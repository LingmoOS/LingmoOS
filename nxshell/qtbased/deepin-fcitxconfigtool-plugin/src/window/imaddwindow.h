// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#ifndef IMADDWINDOW_H
#define IMADDWINDOW_H

#include <QVBoxLayout>
#include <QWidget>
#include <fcitxqtinputmethoditem.h>
namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxButtonTuple;
} // namespace widgets
} // namespace dcc_fcitx_configtool

namespace Dtk {
namespace Widget {
class DSearchEdit;
class DCommandLinkButton;
} // namespace Widget
} // namespace Dtk

class AvailWidget;

class IMAddWindow : public QWidget
{
    Q_OBJECT
public:
    explicit IMAddWindow(QWidget *parent = nullptr);
    virtual ~IMAddWindow();
    void updateUI(); //刷新界面
signals:
    void popSettingsWindow(); //弹出设置窗口
    void addIM(const FcitxQtInputMethodItem &item);
    void pushItemAvailwidget(const FcitxQtInputMethodItem &);
private:
    void initUI(); //初始化界面
    void initConnect(); //初始化信号槽
private slots:
    void onAddIM(); //添加输入法
    void onOpenStore(); //打开应用商店
    void doRemoveSeleteIm(const FcitxQtInputMethodItem &);
private:
    Dtk::Widget::DSearchEdit *m_searchLEdit {nullptr}; //搜索框
    AvailWidget *m_availWidget {nullptr}; //可用输入法列表窗口
    Dtk::Widget::DCommandLinkButton *m_storeBtn {nullptr}; //应用商店按钮
    dcc_fcitx_configtool::widgets::FcitxButtonTuple *m_buttonTuple {nullptr}; //取消/添加按钮
    QVBoxLayout *m_mainLayout;
};

#endif // IMADDWINDOW_H
