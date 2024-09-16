// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINTABWIDGETEX_H
#define MAINTABWIDGETEX_H

#include "BaseWidget.h"
#include "DocSheet.h"

#include <DLabel>

#include <QMap>
#include <QPointer>

class FileDataModel;
class DocummentProxy;
class QStackedLayout;
class DocTabBar;
class SlideWidget;
class QVBoxLayout;

/**
 * @brief The CentralDocPage class
 * 嵌入当前窗体中心控件的文档浏览页面
 */
class CentralDocPage : public BaseWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(CentralDocPage)

public:
    explicit CentralDocPage(DWidget *parent = nullptr);

    ~CentralDocPage() override;

    friend class DocTabBar;

public:
    /**
     * @brief firstThumbnail
     * 为第三方提供,将文档的第一页的缩略图生成到指定路径
     * @param filePath 需要处理的文档路径
     * @param thumbnailPath 缩略图图片生成路径
     */
    static bool firstThumbnail(QString filePath, QString thumbnailPath);     //生成第一页的缩略图

    /**
     * @brief 根据路径增加一个sheet页后异步进行读取，成功保留，不成功删除,如果路径已经存在则切换到对应的页面
     * @param filePath 需要添加的文档路径
     */
    void addFileAsync(const QString &filePath);

    /**
     * @brief saveCurrent
     * 保存当前文档,如果被改变则提示保存
     * @return 保存是否成功
     */
    bool saveCurrent();

    /**
     * @brief saveAsCurrent
     * 另存为当前文档,如果被改变则提示保存
     * @return 保存是否成功
     */
    bool saveAsCurrent();

    /**
     * @brief addSheet
     * 直接添加一个文档页
     * @param sheet
     */
    void addSheet(DocSheet *sheet);        //直接添加sheet

    /**
     * @brief enterSheet
     * 进入一个文档页，会进行信号父窗口等重新绑定
     * @param sheet
     */
    void enterSheet(DocSheet *sheet);      //直接添加sheet (tabbar需要自行判断)

    /**
     * @brief leaveSheet
     * 离开一个文档页，会进行信号父窗口等解绑
     * @param sheet
     */
    void leaveSheet(DocSheet *sheet);      //直接删除sheet (tabbar需要自行判断)

    /**
     * @brief closeSheet
     * 关闭sheet,成功返回true，失败或者取消返回false。
     * @param sheet (关闭成功后会被删除)
     * @param needToBeSaved
     * @return
     */
    bool closeSheet(DocSheet *sheet, bool needToBeSaved);

    /**
     * @brief closeAllSheets
     * 进行关闭，会逐个关闭每个sheet，全部关闭完成返回成功
     * @param needToBeSaved 是否需要提示保存
     * @return
     */
    bool closeAllSheets(bool needToBeSaved);

    /**
     * @brief hasSheet
     * 当前窗口是否含有sheet
     * @param sheet
     * @return 是否存在
     */
    bool hasSheet(DocSheet *sheet);   //

    /**
     * @brief showSheet
     * 显示传入的文档页
     * @param sheet
     */
    void showSheet(DocSheet *sheet);  //显示sheet和对应标签

    /**
     * @brief getSheets
     * 获取sheet
     * @return
     */
    QList<DocSheet *> getSheets();

    /**
     * @brief getCurSheet
     * 返回当前显示的sheet,不存在返回nullptr
     * @return
     */
    DocSheet *getCurSheet();

    /**
     * @brief getSheet
     * 根据路径返回该路径的sheet,不存在返回nullptr
     * @param 文件路径
     * @return
     */
    DocSheet *getSheet(const QString &filePath);

    /**
     * @brief zoomIn
     * 当前文档放大
     */
    void zoomIn();

    /**
     * @brief zoomOut
     * 当前文档缩小
     */
    void zoomOut();

    /**
     * @brief handleShortcut
     * 处理快捷键
     * @param 快捷键名称
     */
    void handleShortcut(const QString &);

    /**
     * @brief showTips
     * 通知父窗口弹出提示框
     * @param tips 显示内容
     * @param iconIndex 图片索引
     */
    void showTips(QWidget *parent, const QString &tips, int iconIndex = 0);

    /**
     * @brief openMagnifer
     * 对当前文档打开放大镜
     */
    void openMagnifer();

    /**
     * @brief quitMagnifer
     * 退出当前文档的放大镜
     */
    void quitMagnifer();

    /**
     * @brief openSlide
     * 当前文档打开幻灯片
     */
    void openSlide();

    /**
     * @brief openSlide
     * 退出当前文档的幻灯片
     */
    void quitSlide();

    /**
     * @brief 是否是幻灯片状态
     */
    bool isSlide();

    /**
     * @brief isFullScreen
     * 当前窗口是否是全屏模式
     * @return
     */
    bool isFullScreen();

    /**
     * @brief openFullScreen
     * 当前窗体进入全屏
     */
    void openFullScreen();

    /**
     * @brief openFullScreen
     * 当前窗体进入退出
     */
    bool quitFullScreen(bool force = false);

    /**
     * @brief openCurFileFolder
     * 打开当前文档所在文件夹并选中当前文档
     */
    void openCurFileFolder();

    /**
     * @brief handleSearch
     * 当前文档进入搜索模式
     */
    void prepareSearch();

    /**
     * @brief getTitleLabel
     * 获取当前文档名称的控件，用于显示在标题栏或者tab栏
     * @return 对应控件
     */
    QWidget *getTitleLabel();

    /**
     * @brief handleBlockShutdown   处理是否阻塞关机
     */
    void handleBlockShutdown();

signals:
    /**
     * @brief sigSheetCountChanged
     * 当前窗体的文档数量改变
     * @param count 当前窗体的文档数量
     */
    void sigSheetCountChanged(int);

    /**
     * @brief sigNeedOpenFilesExec
     * 请求进行阻塞式选择文档打开
     */
    void sigNeedOpenFilesExec();

    /**
     * @brief sigCurSheetChanged
     * 当前文档改变
     * @param sheet 当前的最新文档sheet
     */
    void sigCurSheetChanged(DocSheet *sheet);

    /**
     * @brief sigFindOperation
     * 搜索操作被触发
     * @param operation 触发的操作索引
     */
    void sigFindOperation(const int &operation);

    /**
     * @brief sigNeedShowTips
     * 请求弹出提示
     * @param text 显示内容
     * @param iconIndex 显示图标索引
     */
    void sigNeedShowTips(QWidget *parent, const QString &text, int iconIndex);

    /**
     * @brief sigNeedClose
     * 请求关闭当前窗体
     */
    void sigNeedClose();

    /**
     * @brief sigNeedClose
     * 请求活跃当前窗体
     */
    void sigNeedActivateWindow();

public slots:
    /**
     * @brief 文档被打开后进行处理
     * @param sheet 被打开的文档
     * @param result 打开结果
     */
    void onOpened(DocSheet *sheet, deepin_reader::Document::Error error);

    /**
     * @brief onTabChanged
     * tab上触发的sheet被切换后处理
     * @param sheet
     */
    void onTabChanged(DocSheet *sheet);

    /**
     * @brief onTabMoveIn
     * tab上触发的sheet被移入后处理
     * @param sheet
     */
    void onTabMoveIn(DocSheet *sheet);

    /**
     * @brief onTabClosed
     * tab上触发的sheet被关闭后处理
     * @param sheet
     */
    void onTabClosed(DocSheet *sheet);

    /**
     * @brief onTabMoveOut
     * tab上触发的sheet被移走后处理
     * @param sheet
     */
    void onTabMoveOut(DocSheet *sheet);

    /**
     * @brief onTabNewWindow
     * tab上触发的sheet被移出程序后处理,会新建窗口进行显示
     * @param sheet
     */
    void onTabNewWindow(DocSheet *sheet);

    /**
     * @brief onCentralMoveIn
     * 正文部分上触发的sheet被移入后处理
     * @param sheet
     */
    void onCentralMoveIn(DocSheet *sheet);

    /**
     * @brief onSheetFileChanged
     * sheet内容被修改后处理
     * @param sheet
     */
    void onSheetFileChanged(DocSheet *sheet);

    /**
     * @brief onSheetFileChanged
     * sheet被操作后处理
     * @param sheet
     */
    void onSheetOperationChanged(DocSheet *sheet);

    /**
     * @brief onSheetCountChanged
     * 当前窗体的文档数量改变后处理
     * @param count 当前窗体的文档数量
     */
    void onSheetCountChanged(int count);

    /**
     * @brief 刷新tabLabel文本
     */
    void onUpdateTabLabelText();

private:
    QPointer<DocSheet>  m_magniferSheet = nullptr;
    DLabel              *m_tabLabel = nullptr;
    QStackedLayout      *m_stackedLayout = nullptr;
    DocTabBar           *m_tabBar = nullptr;
    SlideWidget         *m_slideWidget = nullptr;
    QVBoxLayout         *m_mainLayout = nullptr;
};

#endif // MAINTABWIDGETEX_H
