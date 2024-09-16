// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SheetBrowser_H
#define SheetBrowser_H

#include "Model.h"
#include "Global.h"
#include "FindWidget.h"

#include <DGraphicsView>

#include <QLabel>
#include <QMutex>
#include <QGestureEvent>
#include <QPanGesture>
#include <QPinchGesture>
#include <QSwipeGesture>
#include <QTapGesture>
#include <QTimer>
#include <QSet>
#include <QPointF>
#include <QRectF>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QBasicTimer>

#define CELL_TIME   15
#define TAP_MOVE_DELAY 300

using namespace deepin_reader;

class SheetOperation;
class BrowserPage;
class DocSheet;
class TipsWidget;
class BrowserAnnotation;
class TextEditShadowWidget;
class RenderViewportThread;
class BrowserMagniFier;
class FindWidget;
class PageSearchThread;
class QScroller;
class BrowserWord;

/**
 * @brief 浏览文档内容区域,使用视图框架
 */
class SheetBrowser : public Dtk::Widget::DGraphicsView
{
    Q_OBJECT
public:
    friend class BrowserMagniFier;

    explicit SheetBrowser(DocSheet *parent = nullptr);

    ~SheetBrowser() override;

    /**
     * @brief 获取封面缩略图
     * @param filePath 文档所在路径
     * @return
     */
    static QImage firstThumbnail(const QString &filePath);

    /**
     * @brief init
     * 按历史记录初始化，只可调用一次
     * @param operation 操作记录
     * @param bookmarks 书签记录
     */
    void init(SheetOperation &operation, const QSet<int> &bookmarks);

    /**
     * @brief setMouseShape
     * 根据鼠标样式,设置文字是否可选
     * @param shape 鼠标样式
     */
    void setMouseShape(const Dr::MouseShape &shape);

    /**
     * @brief setBookMark
     * 操作文档书签
     * @param index
     * @param state
     */
    void setBookMark(int index, int state);

    /**
     * @brief setAnnotationInserting
     * 设置添加图标注释标志位
     * @param inserting true:添加,false:不添加
     */
    void setAnnotationInserting(bool inserting);

    /**
     * @brief deform
     * 渲染当前视图中的页
     * @param operation 渲染参数包括,大小,单双页,旋转,缩放比例
     */
    void deform(SheetOperation &operation);

    /**
     * @brief hasLoaded
     * 文档是否已加载过
     * @return
     */
    bool hasLoaded();

    /**
     * @brief allPages
     * 当前文档页码总数
     * @return
     */
    int allPages();

    /**
     * @brief currentPage
     * 当前页页码(从1开始)
     * @return
     */
    int currentPage();

    /**
     * @brief setCurrentPage
     * 页码跳转
     * @param page 跳转的页码(base 1)
     */
    void setCurrentPage(int page);

    /**
     * @brief getExistImage
     * 获取文档页图片
     * @param index 页码编号
     * @param image 页码图片
     * @param width 图片宽度
     * @param height 图片高度
     * @param mode 图片状态
     * @param bSrc
     * @return
     */
    bool getExistImage(int index, QImage &image, int width, int height);

    /**
     * @brief magnifierOpened
     * 是否开启了放大镜
     * @return true:开启,false:未开启
     */
    bool magnifierOpened();

    /**
     * @brief maxWidth
     * 最大一页的宽度
     * @return  宽度值
     */
    qreal maxWidth();

    /**
     * @brief maxHeight
     *最大一页的高度
     * @return 高度值
     */
    qreal maxHeight();

    /**
     * @brief 添加标签状态
     * @param index
     * @param state
     */
    void needBookmark(int index, bool state);

    /**
     * @brief selectedWordsText
     * 鼠标选择的文字
     * @return 文字内容
     */
    QString selectedWordsText();

    /**
     * @brief jumpToOutline
     * 点击目录列表中的目录,跳转到文档区域相应位置处
     * @param linkLeft 链接到左侧的距离
     * @param linkTop 链接到顶部的距离
     * @param index 哪一页
     */
    void jumpToOutline(const qreal  &left, const qreal &top, int page);

    /**
     * @brief jumpToHighLight
     * 点击注释列表中的注释,文档区跳到相应注释处
     * @param annotation 点击的那个注释
     * @param index 哪一页
     */
    void jumpToHighLight(deepin_reader::Annotation *annotation, const int index);

    /**
     * @brief annotations
     * 获取注释列表
     * @return 注释列表
     */
    QList< deepin_reader::Annotation * > annotations();

    /**
     * @brief addIconAnnotation
     * 添加图标注释
     * @param page 哪一页
     * @param clickPoint 要添加的注释图标的位置
     * @param contents 图标注释内容
     * @return 当前添加的注释
     */
    Annotation *addIconAnnotation(BrowserPage *page, const QPointF &, const QString &);

    /**
     * @brief addHighLightAnnotation
     * 添加高亮注释
     * @param contains 注释内容
     * @param color 高亮颜色
     * @param showPoint 选取文字的结束坐标
     * @return 新添加高亮注释的指针
     */
    Annotation *addHighLightAnnotation(const QString, const QColor, QPoint &);

    /**
     * @brief removeAnnotation
     * 删除单个注释
     * @param annotation 要删除注释的指针
     * @return 操作状态
     */
    bool removeAnnotation(deepin_reader::Annotation *annotation);

    /**
     * @brief removeAllAnnotation
     * 删除当前文档的所有注释
     * @return 是否操作成功
     */
    bool removeAllAnnotation();

    /**
     * @brief updateAnnotation
     * 更新高亮注释
     * @param annotation 高亮注释的指针
     * @param text 注释内容
     * @param color 高亮颜色
     * @return 是否更新成功
     */
    bool updateAnnotation(deepin_reader::Annotation *annotation, const QString &text, QColor color = QColor());

    /**
     * @brief showNoteEditWidget
     * 显示注释编辑框
     * @param annotation 注释
     * @param point 显示位置
     */
    void showNoteEditWidget(deepin_reader::Annotation *annotation, const QPoint &point);

    /**
     * @brief handlePrepareSearch
     * 搜索操作,弹出搜索框
     */
    void handlePrepareSearch();

    /**
     * @brief jumpToNextSearchResult
     * 跳到下一个搜索条目中
     */
    void jumpToNextSearchResult();

    /**
     * @brief jumpToPrevSearchResult
     * 跳到前一个搜索条目中
     */
    void jumpToPrevSearchResult();

    /**
     * @brief handleFindExit
     * 结束搜索操作,结束搜索任务,清空搜索列表,展示上一次缩略图列表
     */
    void handleSearchStop();

    /**
     * @brief handleSearchStart
     * 准备搜索
     */
    void handleSearchStart();

    /**
     * @brief handleSearchResult
     * 处理搜索结果
     * @param index
     * @param rects
     */
    void handleSearchResultComming(const deepin_reader::SearchResult &res);

    /**
     * @brief handleFindFinished
     * 设置搜索框状态,如果有搜索框是正常,反之设置成Alert状态
     * @param searchcnt 搜索条目数量
     */
    void handleFindFinished(int searchcnt);

    /**
     * @brief jump2Li
     * 判断鼠标点击的位置是否有链接或者目录,如果有响应跳转
     * @param point 鼠标点击的位置
     * @return 返回跳转状态
     */
    bool jump2Link(QPointF);

    /**
     * @brief showMenu
     * 根据菜单键弹出菜单
     */
    void showMenu();

    /**
     * @brief beginViewportChange
     * 延时进行视图更新 过滤高频率调用
     */
    void beginViewportChange();

    /**
     * @brief hideTipsWidget
     * 全屏时隐藏注释提示框,并设置鼠标为指针样式
     */
    void hideSubTipsWidget();

    /**
     * @brief pages
     * @return
     */
    QList<BrowserPage *> pages();

    /**
     * @brief getNoteEditWidget
     * 获取注释窗口对象
     */
    TextEditShadowWidget *getNoteEditWidget() const;

    /**
     * @brief setIsSearchResultNotEmpty
     * 设置m_isSearchResultNotEmpty值
     */
    void setIsSearchResultNotEmpty(bool isSearchResultNotEmpty);

signals:
    void sigPageChanged(int page);

    void sigNeedPageFirst();

    void sigNeedPagePrev();

    void sigNeedPageNext();

    void sigNeedPageLast();

    void sigNeedBookMark(int index, bool state);

    void sigOperaAnnotation(int type, int index, deepin_reader::Annotation *annotation);

    void sigAddHighLightAnnot(BrowserPage *, QString, QColor);

protected:
    enum GestureAction {
        GA_null,
        GA_tap,
        GA_touch,
        GA_click,
        GA_slide,
        GA_select,
        GA_pinch
    };

    /**
     * @brief showEvent
     * 第一次显示之后跳转到初始页码
     * @param event
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief resizeEvent
     * 当前文档视图大小变化事件
     * @param event
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief mousePressEvent
     * 鼠标点击操作,鼠标左键点击注释或者目录操作,鼠标右键菜单操作
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief mouseMoveEvent
     * 鼠标移动操作,移动注释图标,选取文字等操作
     * @param event
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief mouseReleaseEvent
     * 鼠标释放操作,主要是添加图标注释操作,还有选取文字动作等参数,以及展示注释内容
     * @param event
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief focusOutEvent
     * 失去焦点处理
     * @param event
     */
    void focusOutEvent(QFocusEvent *event) override;

    /**
     * @brief 计时器事件
     * @param event
     */
    void timerEvent(QTimerEvent *event) override;

    /**
     * @brief dragEnterEvent
     * 忽略拖拽事件
     * @param event
     */
    void dragEnterEvent(QDragEnterEvent *event) override;

    /**
     * @brief wheelEvent
     * 鼠标滚轮事件,主要是文档缩放(ctrl+wheel)
     * @param event
     */
    void wheelEvent(QWheelEvent *event) override;

    /**
     * @brief event
     * 综合事件处理中心,处理显示菜单和触摸屏手势
     * @param event 系统事件
     * @return
     */
    bool event(QEvent *event) override;

    /**
     * @brief gestureEvent
     * 处理触摸屏手势事件
     * @param event
     * @return
     */
    bool gestureEvent(QGestureEvent *event);

    /**
     * @brief pinchTriggered
     * 触摸屏双指缩放和旋转
     */
    void pinchTriggered(QPinchGesture *);

    /**
     * @brief 根据鼠标点击位置判断在哪一页
     * @param viewPoint 鼠标点击位置
     * @return 鼠标点击的页对象
     */
    BrowserPage *getBrowserPageForPoint(QPointF &viewPoint);

public slots:
    /**
     * @brief openMagnifier
     * 打开放大镜
     */
    void openMagnifier();

    /**
     * @brief closeMagnifier
     * 关闭放大镜
     */
    void closeMagnifier();

private slots:
    /**
     * @brief onInit
     * 初始化响应,跳转到初始化页
     */
    void onInit();

    /**
     * @brief onVerticalScrollBarValueChanged
     * 纵向滚动条数值改变槽函数,并通知左侧栏页变化
     */
    void onVerticalScrollBarValueChanged(int value);

    /**
     * @brief onHorizontalScrollBarValueChanged
     * 水平滚动条数值改变槽函数
     */
    void onHorizontalScrollBarValueChanged(int value);

    /**
     * @brief onViewportChanged
     * 当前视图区域改变,渲染文档
     */
    void onViewportChanged();

    /**
     * @brief onAddHighLightAnnot
     * 添加高亮注释
     * @param page 哪一页
     * @param text 注释内容
     * @param color 注释高亮颜色
     */
    void onAddHighLightAnnot(BrowserPage *page, QString text, QColor color);

    /**
     * @brief onRemoveAnnotation
     * 删除注释
     * @param annotation 要删除注释的指针
     * @param tips 是否显示提示框
     */
    void onRemoveAnnotation(deepin_reader::Annotation *annotation, bool tips);

    /**
     * @brief onUpdateAnnotation
     * 更新注释内容
     * @param annotation 哪个注释
     * @param text 注释内容
     */
    void onUpdateAnnotation(deepin_reader::Annotation *annotation, const QString &text);

    /**
     * @brief onSetDocSlideGesture
     * 设置文档触摸屏滑动事件
     */
    void onSetDocSlideGesture();

    /**
     * @brief onRemoveDocSlideGesture
     * 移除文档触摸屏滑动事件
     */
    void onRemoveDocSlideGesture();

    /**
     * @brief onRemoveIconAnnotSelect
     * 注释菜单隐藏取消图标注释选中
     */
    void onRemoveIconAnnotSelect();

private:
    /**
     * @brief calcIconAnnotRect
     * 计算注释图标范围
     * @param page 图标注释在哪一页
     * @param point 当前点击的坐标点
     * @param iconRect 注释图标的范围
     * @return  true:计算成功    false:计算失败
     */
    bool calcIconAnnotRect(BrowserPage *page, const QPointF &, QRectF &);

    /**
     * @brief translate2Local
     * 将非0度的坐标点转换成0度的坐标点
     * @return
     */
    QPointF translate2Local(QPointF);

    /**
     * @brief getClickAnnot
     * 获得当前鼠标点击的注释
     * @param page 哪一页
     * @param clickPoint 鼠标点击位置
     * @param drawRect 是否绘制选择框
     * @return 当前点击注释的指针
     */
    Annotation *getClickAnnot(BrowserPage *page, const QPointF, bool drawRect = false);

    /**
     * @brief jump2PagePos
     * 响应左侧注释列表点击事件,跳到对应文档目录处
     * @param jumpPage  哪一页
     * @param posLeft   目录到左侧距离
     * @param posTop    目录到顶部距离
     */
    void jump2PagePos(BrowserPage *page, const qreal posLeft, const qreal posTop);

    /**
     * @brief moveIconAnnot
     * 鼠标移动图标注释,设置当前图标注释的位置
     * @param page 图标注释所在页的指针
     * @param clickPoint 鼠标移动的位置
     */
    bool moveIconAnnot(BrowserPage *page, const QPointF &);

    /**
     * @brief currentIndexRange
     * 当前显示的页数范围
     * @param fromIndex 当前显示的起始页码索引
     * @param toIndex 当前显示的结束页码索引
     * @return
     */
    void currentIndexRange(int &fromIndex, int &toIndex);

    /**
     * @brief currentScrollValueForPage
     * 纵向滚动条值变化,得到滚动条当前位置所在文档页的编号
     * @return 文档页的编号(页码从1开始)
     */
    int currentScrollValueForPage();

    /**
     * @brief curpageChanged
     * 页码变化,通知左侧栏
     * @param curpage 当前页页码编号,从0开始
     */
    void curpageChanged(int curpage);

    /**
     * @brief isLink
     * 判断鼠标点击位置是否有链接或者目录
     * @param point 鼠标点击位置
     * @return true:有, false:没有
     */
    bool isLink(QPointF);

    /**
     * @brief getLinkAtPoint 获取当前位置的链接
     * @param point
     * @return
     */
    Link getLinkAtPoint(QPointF viewpoint);

    /**
     * @brief setIconAnnotSelect
     * 设置注释图标选择框
     * @param select true:是选择, false:取消选择
     */
    void setIconAnnotSelect(const bool);

    /**
     * @brief setDocTapGestrue
     * 根据手势点击位置设置文档滑动方式
     */
    bool setDocTapGestrue(QPoint);

    /**
     * @brief clearSelectIconAnnotAfterMenu
     * 图标注释消失之后清除图标注释的选中状态
     */
    void clearSelectIconAnnotAfterMenu();

    /**
     * @brief 显示放大镜位置图片
     * @param point
     */
    void showMagnigierImage(const QPoint &point);

    /**
     * @brief moveScrollBar 移动滚动条
     * @param point
     */
    void moveScrollBar(const QPoint &point);

    /**
     * @brief getAnnotPosInPage
     * 将传入的位置拉回到页面里，返回新的位置
     * @param pos 需要是page中的位置，全局的位置需要先使用page->mapFromScene
     * @return
     */
    QPointF getAnnotPosInPage(const QPointF &pos, BrowserPage *page);

private:
    DocSheet *m_sheet = nullptr;

    BrowserMagniFier *m_magnifierLabel = nullptr;
    TipsWidget *m_tipsWidget = nullptr;
    TextEditShadowWidget *m_noteEditWidget = nullptr;
    QPointer<FindWidget> m_findWidget = nullptr;

    BrowserPage *m_lastFindPage = nullptr;
    BrowserPage *m_lastSelectIconAnnotPage = nullptr;   // 最后选中图标注释所在页
    QTimer *m_viewportChangeTimer = nullptr;            //用于延时进行视图区域更新 防止高频率调用

    QString m_filePassword;
    QList<BrowserPage *> m_items;

    QPointF m_selectPressedPos;         // 鼠标按压的起始位置
    QPointF m_selectStartPos;           // 选取文字的开始位置
    QPointF m_selectEndPos;             // 选取文字的结束位置(鼠标释放的最后位置)
    bool m_bHandAndLink = false;               // 鼠标是否手形工具且在超链接处

    BrowserWord *m_selectWord = nullptr;
    BrowserWord *m_selectStartWord = nullptr;
    BrowserWord *m_selectEndWord = nullptr;

    double m_lastScaleFactor = 0;
    qreal m_maxWidth = 0;               //最大一页的宽度
    qreal m_maxHeight = 0;              //最大一页的高度
    bool m_changSearchFlag = false;
    bool m_hasLoaded = false;           //是否已经加载过每页的信息
    int m_initPage = 1;                 //用于刚显示跳转的页数

    //搜索
    int m_searchCurIndex = 0;           //当前搜索所在索引
    int m_searchPageTextIndex = 0;      //当前搜索索引的索引

    //文字选择
    int m_selectIndex = -1;             //选取文字开始的index
    bool m_isSearchResultNotEmpty = false; // true表示搜索到的结果不为空

    bool m_annotationInserting = false;     //正在插入注释状态
    bool m_selectIconAnnotation = false;    //当前是否有选中的图标注释
    QPointF m_iconAnnotationMovePos;        //当前选中的图标注释移动位置
    deepin_reader::Annotation *m_iconAnnot = nullptr; // 当前选中的图标注释

    int m_currentPage = 0;
    int m_lastrotation = 0;
    bool m_bNeedNotifyCurPageChanged = true;

    QBasicTimer m_repeatTimer;
    bool m_startPinch = false; //开始收缩
    bool m_canTouchScreen = false;
    QScroller *m_scroller = nullptr;
};

#endif // SheetBrowser_H
