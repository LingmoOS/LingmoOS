// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DBlurEffectWidget>

#include <QScopedPointer>

namespace GrandSearch {

class MainWindowPrivate;
class MainWindow : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() Q_DECL_OVERRIDE;

    // 必须在主界面显示后再调用该函数，处理业务相关流程
    void connectToController();

private slots:
    // 主屏改变响应槽，移动已显示的主界面到主屏
    void onPrimaryScreenChanged(const QScreen *screen);
    // 主屏分辨率改变响应槽，调整显示的位置
    void onGeometryChanged(const QRect &geometry);
    // 隐藏展示界面
    void onHideExhitionWidget();
    // 重置展示界面
    void onResetExhitionWidget(const QString &missionId);
    // 搜索任务改变
    void onMissionChanged(const QString &missionId, const QString &missionContent);

private:
    void initUI();
    void initConnect();

    void activeMainWindow();
    void updateMainWindowHeight();

    void showExhitionWidget(bool bShow = true);
    void showSerachNoContent(bool bShow = true);
    void showEntranceAppIcon(bool bShow = true);

    void onFocusObjectChanged(QObject *obj);

protected:
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    virtual void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

signals:
    // 显示状态改变信号
    void visibleChanged(const bool visible);
    // 搜索文本改变信号，通知查询控制器发起新的搜索
    void searchTextChanged(const QString &txt);
    // 终止搜索信号，软件退出前通知后端终止搜索
    void terminateSearch();

private:
    QScopedPointer<MainWindowPrivate> d_p;
};

}

#endif // MAINWINDOW_H
