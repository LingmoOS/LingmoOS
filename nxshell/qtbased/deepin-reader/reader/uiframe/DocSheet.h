// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DocSheet_H
#define DocSheet_H

#include "Model.h"

#include <DSplitter>
#include <QSet>
#include <QReadWriteLock>
#include <QThread>

class SheetSidebar;
class SlideWidget;
class EncryptionPage;
class QPropertyAnimation;
class QPrinter;
class PageSearchThread;
struct SheetOperation {
    Dr::LayoutMode layoutMode   = Dr::SinglePageMode;
    Dr::MouseShape mouseShape   = Dr::MouseShapeNormal;
    Dr::ScaleMode scaleMode     = Dr::FitToPageWorHMode;
    Dr::Rotation rotation       = Dr::RotateBy0;
    qreal scaleFactor           = 1.0;
    bool sidebarVisible         = false;
    int  sidebarIndex           = 0;
    int  currentPage            = 1;
};

DWIDGET_BEGIN_NAMESPACE
class DPrinter;
DWIDGET_END_NAMESPACE

class QTemporaryDir;
class SheetBrowser;
class SheetRenderer;
/**
 * @brief The DocSheet class
 * 嵌入当前窗体中心控件，一个类表示并显示操作一个文档
 */
class DocSheet : public Dtk::Widget::DSplitter
{
    Q_OBJECT
    Q_DISABLE_COPY(DocSheet)

    friend class SheetSidebar;
    friend class SheetBrowser;
    friend class Database;
public:
    explicit DocSheet(const Dr::FileType &fileType, const QString &filePath, QWidget *parent = nullptr);

    ~DocSheet() override;

public:
    /**
     * @brief firstThumbnail
     * 获取对应文档的第一页
     * @param filePath 需要处理的文档路径
     */
    static QImage firstThumbnail(const QString &filePath);

    /**
     * @brief existFileChanged
     * 进程所有的文档是否存在被修改的
     * @return
     */
    static bool existFileChanged();

    /**
     * @brief getUuid
     * 根据文档返回对应的uuid
     * @param sheet
     * @return
     */
    static QUuid getUuid(DocSheet *sheet);

    /**
     * @brief existSheet
     * 进程是否存在该文档
     * @param sheet
     * @return
     */
    static bool existSheet(DocSheet *sheet);

    /**
     * @brief getSheet
     * 根据uuid返回sheet
     * @param uuid
     * @return
     */
    static DocSheet *getSheet(QString uuid);

    /**
     * @brief getSheet
     * 根据uuid返回sheet
     * @param filePath
     * @return
     */
    static DocSheet *getSheetByFilePath(QString filePath);

    /**
     * @brief getSheets
     * 返回所有sheet
     * @return
     */
    static QList<DocSheet *> getSheets();

    /**
     * @brief 全局对象判断读写锁
     */
    static QReadWriteLock g_lock;

    static QStringList g_uuidList;

    static QList<DocSheet *> g_sheetList;

    static QString g_lastOperationFile; //保存最后一次操作的文件

public:
    /**
     * @brief 阻塞式打开文档
     * @param password
     */
    bool openFileExec(const QString &password);

    /**
     * @brief 异步式打开文档，完成后会发出sigFileOpened
     * @param password 文档密码
     * @return
     */
    void openFileAsync(const QString &password);

    /**
     * @brief 获取文档总页数
     * @return
     */
    int pageCount();

    /**
     * @brief currentPage
     * 获取当前文档页数 base 1
     * @return
     */
    int currentPage();

    /**
     * @brief currentPage
     * 获取当前文档索引值 base 0
     * @return
     */
    int currentIndex();

    /**
     * @brief jumpToIndex
     * 跳转到索引
     * @param index 索引值
     */
    void jumpToIndex(int index);

    /**
     * @brief jumpToPage
     * 跳转到页
     * @param page 页码
     */
    void jumpToPage(int page);

    /**
     * @brief jumpToFirstPage
     * 跳转到第一页
     */
    void jumpToFirstPage();

    /**
     * @brief jumpToLastPage
     * 跳转到最后一页
     */
    void jumpToLastPage();

    /**
     * @brief jumpToNextPage
     * 跳转到下一页
     */
    void jumpToNextPage();

    /**
     * @brief jumpToPrevPage
     * 跳转到上一页
     */
    void jumpToPrevPage();

    /**
     * @brief rotateLeft
     * 左旋转
     */
    void rotateLeft();

    /**
     * @brief rotateRight
     * 右旋转
     */
    void rotateRight();

    /**
     * @brief outline
     * 获取跳转链接列表
     * @return
     */
    deepin_reader::Outline outline();

    /**
     * @brief jumpToOutline
     * 跳转到指定位置
     * @param left 距离左边距离
     * @param top 距离上边距离
     * @param index 跳转的页码
     */
    void jumpToOutline(const qreal  &left, const qreal &top, int index);

    /**
     * @brief jumpToHighLight
     * 跳转到注释位置
     * @param annotation 注释
     * @param index 跳转的索引值
     */
    void jumpToHighLight(deepin_reader::Annotation *annotation, const int index);

    /**
     * @brief setBookMark
     * 设置书签
     * @param index 被设置的索引
     * @param state 被设置的状态
     */
    void setBookMark(int index, int state);

    /**
     * @brief setBookMarks
     * 批量设置书签
     * @param indexlst 被设置的索引列表
     * @param state 被设置的状态
     */
    void setBookMarks(const QList<int> &indexlst, int state);

    /**
     * @brief setLayoutMode
     * 批量布局类型
     * @param mode
     */
    void setLayoutMode(Dr::LayoutMode mode);

    /**
     * @brief setScaleMode
     * 批量缩放类型
     * @param mode
     */
    void setScaleMode(Dr::ScaleMode mode);

    /**
     * @brief setScaleFactor
     * 批量缩放因子;base 1;设置后自动取消自适应
     * @param scaleFactor
     */
    void setScaleFactor(qreal scaleFactor);

    /**
     * @brief setMouseShape
     * 设置鼠标样式
     * @param mode
     */
    void setMouseShape(Dr::MouseShape shape);

    /**
     * @brief setAnnotationInserting
     * 设置注释插入状态;设置后点击文档可以在鼠标位置插入图标注释
     * @param inserting
     */
    void setAnnotationInserting(bool inserting);

    /**
     * @brief thumbnail
     * 获取缩略图(需要先设置)
     * @param index
     * @return
     */
    QPixmap thumbnail(int index);

    /**
     * @brief setThumbnail
     * @param index
     * @param pixmap
     */
    void setThumbnail(int index, QPixmap pixmap);

    /**
     * @brief openMagnifier
     * 调用browser打开放大镜
     */
    void openMagnifier();

    /**
     * @brief closeMagnifier
     * 调用browser关闭放大镜
     */
    void closeMagnifier();

    /**
     * @brief magnifierOpened
     * 获取放大镜是否被打开
     * @return
     */
    bool magnifierOpened();

    /**
     * @brief fileChanged
     * 文档是否被改变
     * @return 是否被改变
     */
    bool fileChanged();

    /**
     * @brief saveData
     * 保存文档
     * @return 是否成功
     */
    bool saveData();

    /**
     * @brief saveData
     * 另存为文档
     * @param targetFilePath 另存为路径
     * @return 是否成功
     */
    bool saveAsData(QString targetFilePath);

    /**
     * @brief handlePageModified
     * 处理page修改
     */
    void handlePageModified(int index);

    /**
     * @brief copySelectedText
     * 复制选中文字到剪贴板
     */
    void copySelectedText();

    /**
     * @brief highlightSelectedText
     * 高亮选中文字
     */
    void highlightSelectedText();

    /**
     * @brief addSelectedTextHightlightAnnotation
     * 对选中文字添加高亮注释
     */
    void addSelectedTextHightlightAnnotation();

    /**
     * @brief getImage
     * @param index 索引
     * @param width
     * @param height
     * @param slice
     * @return
     */
    QImage getImage(int index, int width, int height, const QRect &slice = QRect());

    /**
     * @brief defaultFocus
     * 焦点到默认控件上去
     */
    void defaultFocus();

    /**
     * @brief annotations
     * 获取所有的注释
     * @return
     */
    QList<deepin_reader::Annotation *> annotations();

    /**
     * @brief removeAnnotation
     * 删除注释并提示
     * @param annotation 被删除的注释指针
     * @param tips 是否提示
     * @return 是否执行成功
     */
    bool removeAnnotation(deepin_reader::Annotation *annotation, bool tips = true);

    /**
     * @brief removeAnnotation
     * 删除所有注释
     * @return 是否执行成功
     */
    bool removeAllAnnotation();

    /**
     * @brief scaleFactorList
     * 获取缩放因子列表
     * @return
     */
    QList<qreal> scaleFactorList();

    /**
     * @brief maxScaleFactor
     * 获取最大的缩放因子 (产品文档中根据文档大小对缩放因子有限制)
     * @return
     */
    qreal maxScaleFactor();

    /**
     * @brief filter
     * 文件选择框过滤
     * @return
     */
    QString filter();

    /**
     * @brief format
     * 获取类型名称
     * @return
     */
    QString format();

    /**
     * @brief getBookMarkList
     * 获取书签列表
     * @return
     */
    QSet<int> getBookMarkList() const;

    /**
     * @brief operation
     * 获取用户操作
     * @return
     */
    SheetOperation operation() const;

    /**
     * @brief fileType
     * 获取文件类型
     * @return
     */
    Dr::FileType fileType();

    /**
     * @brief filePath
     * 用户想要打开的文档路径
     * @return
     */
    QString filePath();

    /**
     * @brief openedFilePath
     * 实际打开的文档路径
     * @return
     */
    QString openedFilePath();

    /**
     * @brief convertedFileDir
     * 转换后打开的文档目录
     * @return
     */
    QString convertedFileDir();

    /**
     * @brief hasBookMark
     * 查看是否含有对应页书签
     * @return
     */
    bool hasBookMark(int index);

    /**
     * @brief zoomin
     * 放大一级
     */
    void zoomin();

    /**
     * @brief zoomout
     * 缩小一级
     */
    void zoomout();

    /**
     * @brief setSidebarVisible
     * 设置左侧栏显示
     * @param isVisible 显示
     * @param notify 是否通知操作变化
     */
    void setSidebarVisible(bool isVisible, bool notify = true);

    /**
     * @brief openSlide
     * 通知父窗口打开幻灯片
     */
    void openSlide();

    /**
     * @brief openSlide
     * 通知父窗口关闭幻灯片
     */
    void closeSlide();

    /**
     * @brief isFullScreen
     * 获取是否是全屏
     * @return
     */
    bool isFullScreen();

    /**
     * @brief openFullScreen
     * 打开全屏模式
     */
    void openFullScreen();

    /**
     * @brief closeFullScreen
     * 关闭全屏模式
     */
    bool closeFullScreen(bool force = false);

    /**
     * @brief setDocumentChanged
     * 设置文档本身被修改
     * @param chenged
     */
    void setDocumentChanged(bool changed);

    /**
     * @brief setBookmarkChanged
     * 设置文档书签被修改(书签存在数据库不随文件)
     * @param chenged
     */
    void setBookmarkChanged(bool changed);

    /**
     * @brief setOperationChanged
     * 设置用户操作信息被修改
     */
    void setOperationChanged();

    /**
     * @brief prepareSearch
     * 通知browser进入搜索模式,未开始真正搜索
     */
    void prepareSearch();

    /**
     * @brief handleFindContent
     * 开始搜索
     * @param strFind 被搜索关键词
     */
    void startSearch(const QString &strFind);

    /**
     * @brief handleFindExit
     * 停止搜索 未退出搜索状态
     */
    void stopSearch();

    /**
     * @brief jumpToNextSearchResult
     * 跳转下一个搜索结果
     */
    void jumpToNextSearchResult();

    /**
     * @brief jumpToPrevSearchResult
     * 跳转上一个搜索结果
     */
    void jumpToPrevSearchResult();

    /**
     * @brief showEncryPage
     * 显示解锁页面
     */
    void showEncryPage();

    /**
     * @brief opened
     * 获取是否被打开
     * @return
     */
    bool opened();

    /**
     * @brief getPageLableIndex
     * 获取文档下标的索引
     * @param pageLable 下标
     * @return  索引
     */
    int getIndexByPageLable(const QString &pageLable);

    /**
     * @brief getPageLabelByIndex
     * 获取索引的文档下标
     * @param index 索引
     * @return 下标
     */
    QString getPageLabelByIndex(const int &index);

    /**
     * @brief 获取PAGE真实大小
     * @param index
     * @return
     */
    QSizeF pageSizeByIndex(int index);

    /**
     * @brief renderer
     * 获取渲染器
     * @return
     */
    SheetRenderer *renderer();

public slots:
    /**
     * @brief 阻塞打印
     */
    void onPopPrintDialog();

    /**
     * @brief 阻塞弹出文件信息
     */
    void onPopInfoDialog();

private:
    /**
     * @brief operationRef
     * 获取操作的引用(可以直接修改)
     * @return
     */
    SheetOperation &operationRef();

    /**
     * @brief 重置sidebar,browser的parent
     */
    void resetChildParent();

public slots:
    /**
     * @brief showTips
     * 通知父窗口弹出提示框
     * @param tips 显示内容
     * @param iconIndex 图片索引
     */
    void showTips(const QString &tips, int iconIndex = 0);

    /**
     * @brief 打印 1040
     * @param printer 打印机设备
     */
    void onPrintRequested(Dtk::Widget::DPrinter *printer, const QVector<int> &pageRange);

    /**
     * @brief 打印 1030
     * @param printer 打印机设备
     */
    void onPrintRequested(Dtk::Widget::DPrinter *printer);

private slots:
    /**
     * @brief onSearchResultComming
     * 搜索结果的处理
     * @param searchResult 搜索结果
     */
    void onSearchResultComming(const deepin_reader::SearchResult &searchResult);

    /**
     * @brief onSearchFinished
     * 搜索结束的处理
     */
    void onSearchFinished();

    /**
     * @brief onSearchResultNotEmpty
     * 搜索结果非空的处理
     */
    void onSearchResultNotEmpty();

    /**
     * @brief onSideAniFinished
     * 左侧动画结束
     */
    void onSideAniFinished();

    /**
     * @brief onOpened
     * 打开结果
     * @param error
     */
    void onOpened(deepin_reader::Document::Error error);

public:
    /**
     * @brief haslabel
     * 获取是否有文档页码
     * @return
     */
    bool haslabel();

    /**
     * @brief docBasicInfo
     * 获取文档信息
     * @param fileInfo 用于传出的文档信息
     */
    void docBasicInfo(deepin_reader::FileInfo &fileInfo);

    /**
     * @brief getSheetBrowser
     * 获取右侧内容视图对象指针
     */
    SheetBrowser *getSheetBrowser() const;

signals:
    /**
     * @brief sigFindOperation
     * 请求查找操作
     * @param operation 查找操作
     */
    void sigFindOperation(const int &operation);

    /**
     * @brief sigPageModified
     * 页码被修改
     */
    void sigPageModified(int index);

    /**
     * @brief 文档被打开
     * @param result 结果
     * @param error 错误
     */
    void sigFileOpened(DocSheet *sheet, deepin_reader::Document::Error error);

    /**
     * @brief sigFileChanged
     * 文档内容变化通知
     * @param sheet 被变化的文档
     */
    void sigFileChanged(DocSheet *sheet);         //被修改了 书签 注释等

    /**
     * @brief sigOperationChanged
     * 文档操作变化通知
     * @param sheet 被变化的文档
     */
    void sigOperationChanged(DocSheet *sheet);    //被修改了 缩放 当前页等

private slots:
    /**
     * @brief onBrowserPageChanged
     * 当前页变化处理
     * @param page 最新页码
     */
    void onBrowserPageChanged(int page);

    /**
     * @brief onBrowserPageFirst
     * 跳转到第一页请求的处理
     */
    void onBrowserPageFirst();

    /**
     * @brief onBrowserPagePrev
     * 跳转到上一页请求的处理
     */
    void onBrowserPagePrev();

    /**
     * @brief onBrowserPageNext
     * 跳转到下一页请求的处理
     */
    void onBrowserPageNext();

    /**
     * @brief onBrowserPageLast
     * 跳转到最后一页请求的处理
     */
    void onBrowserPageLast();

    /**
     * @brief onBrowserBookmark
     * 书签变化请求的处理
     * @param index 索引
     * @param state 书签状态
     */
    void onBrowserBookmark(int index, bool state);

    /**
     * @brief onBrowserOperaAnnotation
     * 注释操作的处理
     * @param type 操作类型
     * @param index 索引
     * @param Annotation 注释指针
     */
    void onBrowserOperaAnnotation(int type, int index, deepin_reader::Annotation *annotation);

    /**
     * @brief onExtractPassword
     * 密码执行处理
     * @param password
     */
    void onExtractPassword(const QString &password);

private:
    /**
     * @brief setAlive
     * 设置当前sheet是否存活
     * @param alive
     */
    void setAlive(bool alive);

    /**
     * @brief readLastFileOperation 读取最后一次操作文件的配置
     */
    bool readLastFileOperation();

protected:
    void resizeEvent(QResizeEvent *event) override;

    void childEvent(QChildEvent *c) override;

private:
    SheetOperation  m_operation;
    QSet<int>       m_bookmarks;

    //document
    deepin_reader::Document *m_document = nullptr;

    SheetSidebar   *m_sidebar  = nullptr;        //操作左侧ui
    SheetBrowser   *m_browser  = nullptr;        //操作右侧ui
    SheetRenderer  *m_renderer = nullptr;        //数据渲染器

    QString         m_filePath;
    QString         m_password;
    Dr::FileType    m_fileType;
    QString         m_uuid;
    QTemporaryDir  *m_tempDir = nullptr;        //存放临时数据
    QMap<int, QPixmap>  m_thumbnailMap;

    bool m_documentChanged = false;
    bool m_bookmarkChanged = false;
    bool m_fullSiderBarVisible = false;
    PageSearchThread *m_searchTask = nullptr;
    EncryptionPage  *m_encryPage = nullptr;
    QPropertyAnimation *m_sideAnimation = nullptr;

public:
    QProcess *m_process = nullptr; //当前调用的命令的进程地址

    /**
     * @brief The LoadingWidget class 打印时的加载窗口和相关功能
     * 父类parent不能为空，一般为qApp->activeWindow()
     * 函数getImage会在子线程内获取对应的图片，被loop阻塞
     * 通过下面方法 停止parent【tab键和点击按钮】的后续响应
     * 1.阻塞方式ExcludeSocketNotifiers
     * 2.parent->setEnabled(false)
     */
    class LoadingWidget : public QWidget {
    public:
        explicit LoadingWidget(QWidget *parent);
        ~LoadingWidget() override;

        /**
         * @brief getImage 子线程获取image
         */
        QImage getImage(DocSheet *doc, int index, int width, int height);

    protected:
        void paintEvent(QPaintEvent */*event*/) override;

    private:
        QWidget *m_parentWidget = nullptr;
    };
};

#endif // DocSheet_H
