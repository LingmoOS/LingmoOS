// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <DWidget>
#include <DGuiApplicationHelper>
#include <DDialog>
#include <DWidgetUtil>
#include <QMouseEvent>


DWIDGET_USE_NAMESPACE

class TopTilte;
class DrawBoard;
class DrawDialog;
class ProgressLayout;
class PageContext;
class AttributionWidget;
class IAttributionRegister;
class MainWindow: public DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QStringList filePaths); // 需要打开多个文件时的构造函数
    ~MainWindow() override;

    /**
     * @brief activeWindow 激活主窗口
     */
    void activeWindow();

    void readSettings();

    /**
     * @brief openFile　暴露函数接口给其它程序调用，打开多个路径文件
     * @param filePath　路径
     * @param isStartByDDF　是否以打开ＤＤＦ文件方式启动画板软件
     * @param bAdapt 是否做自适?
     */
    Q_INVOKABLE bool openFiles(QStringList filePaths, bool bAdapt = false);

    /**
     * @brief drawBoard
     */
    DrawBoard *drawBoard() const;

    /**
     * @brief topTitle　获取TopToolbar指针用于单列测试
     */
    TopTilte *topTitle() const;
    void loadFiles(const QStringList &filePaths);

public slots:
    /**
     * @brief onViewShortcut
     */
    void onViewShortcut();
    /**
     * @brief slotOnEscButtonClick　ＥＳＣ按钮槽函数
     */
    void slotOnEscButtonClick();
    /**
     * @brief setWindowTitleInfo　设置windowtitle
     */
    void setWindowTitleInfo();


protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e)override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *e)override;
private slots:
    /**
     * @brief slotShowOpenFileDialog　显示打开文件对话框
     */
    void slotShowOpenFileDialog();

private:
    TopTilte *m_topToolbar;
    DrawBoard *m_drawBoard;
    DScrollArea *m_toolManagerScrollArea;
    //AttributionWidget *m_attriWidget;
    int m_titlebarWidth;
    bool m_contrlKey;
    QAction *m_quitMode;
    QAction *m_showCut;
    QString tmpPictruePath;
    DFloatingMessage *pDFloatingMessage = nullptr;    //驻留消息提示
    IAttributionRegister *m_attriRegisterMgr = nullptr;
private:
    /**
     * @brief initConnection　初始化连接
     */
    void initConnection();
    /**
     * @brief initUI　初始化ＵＩ
     */
    void initUI();
    /**
     * @brief initAttributions　初始化显示属性
     */
    void initAttributions();
};

#endif // MAINWINDOW_H
