// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CENTRAL_H
#define CENTRAL_H

#include "BaseWidget.h"

class CentralDocPage;
class CentralNavPage;
class TitleMenu;
class TitleWidget;
class DocSheet;
class QStackedLayout;

/**
 * @brief The BaseWidget class
 * 嵌入mainwindow的中心控件
 */
class Central : public BaseWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(Central)

public:
    explicit Central(QWidget *parent = nullptr);

    ~Central() override;

    /**
     * @brief addFilesWithDialog
     * 打开文件选择对话框选择文件并打开(会进行判断文件是否已经存在在其他窗口)
     * @return
     */
    void addFilesWithDialog();

    /**
     * @brief 根据路径增加一个sheet页后异步进行读取，成功保留，不成功删除,如果路径已经存在则切换到对应的页面
     * @param filePath 需要添加的文档路径
     */
    void addFileAsync(const QString &filePath);

    /**
     * @brief addSheet
     * 直接添加一个文档页
     * @param sheet
     */
    void addSheet(DocSheet *sheet);

    /**
     * @brief hasSheet
     * 本窗口是否含有该文档页
     * @param sheet
     * @return
     */
    bool hasSheet(DocSheet *sheet);

    /**
     * @brief showSheet
     * 显示传入的文档页
     * @param sheet
     */
    void showSheet(DocSheet *sheet);

    /**
     * @brief getSheets
     * 获取sheet
     * @return
     */
    QList<DocSheet *> getSheets();

    /**
     * @brief handleShortcut
     * 本窗口处理快捷键
     * @param shortcut 快捷键字符串
     */
    void handleShortcut(const QString &shortcut);

    /**
     * @brief handleClose
     * 进行关闭，会逐个关闭每个sheet，全部关闭完成返回成功
     * @param needToBeSaved 是否需要提示保存
     * @return
     */
    bool handleClose(bool needToBeSaved);

    /**
     * @brief titleWidget
     * 获取提供给mainwindow的标题控件
     * @return
     */
    TitleWidget *titleWidget();

    /**
     * @brief docPage
     * 获取文档浏览堆栈窗口
     * @return
     */
    CentralDocPage *docPage();

    /**
     * @brief setMenu
     * 设置菜单
     * @return
     */
    void setMenu(TitleMenu *menu);

signals:
    /**
     * @brief sigNeedClose
     * 需要关闭本窗口
     */
    void sigNeedClose();

public slots:
    /**
     * @brief onSheetCountChanged
     * 根据当前文档个数变化后改变显示导航页面还是文档浏览页面
     * @param count 当前文档个数
     */
    void onSheetCountChanged(int count);

    /**
     * @brief onMenuTriggered
     * 菜单功能触发处理
     * @param action 菜单功能名称
     */
    void onMenuTriggered(const QString &action);

    /**
     * @brief onOpenFilesExec
     * 打开文件请求处理
     */
    void onOpenFilesExec();

    /**
     * @brief onNeedActivateWindow
     * 将窗口设为活跃窗口的请求处理
     */
    void onNeedActivateWindow();

    /**
     * @brief onShowTips
     * 弹出提示的请求处理
     * @param text 请求显示文本
     * @param iconIndex 请求显示图标索引
     */
    void onShowTips(QWidget *parent, const QString &text, int iconIndex = 0);

    /**
     * @brief Application::onTouchPadEventSignal
     * 处理触控板手势信号
     * @param name 触控板事件类型(手势或者触摸类型) pinch 捏 tap 敲 swipe 滑 右键单击 单键
     * @param direction 手势方向 触控板上 up 触控板下 down 左 left 右 right 无 none 向内 in 向外 out  触控屏上 top 触摸屏下 bot
     * @param fingers 手指数量 (1,2,3,4,5)
     * 注意libinput接收到触摸板事件后将接收到的数据通过Event广播出去
     */
    void onTouchPadEvent(const QString &name, const QString &direction, int fingers);

    /**
     * @brief onKeyTriggered
     * 快捷键触发
     */
    void onKeyTriggered();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dropEvent(QDropEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    QStackedLayout  *m_layout = nullptr;
    QWidget         *m_mainWidget = nullptr;
    CentralDocPage  *m_docPage = nullptr;
    CentralNavPage  *m_navPage = nullptr;
    TitleMenu       *m_menu = nullptr;
    TitleWidget     *m_widget = nullptr;
};

#endif  // CENTRAL_H
