// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BrowserPage_H
#define BrowserPage_H

#include "PDFModel.h"
#include "DocSheet.h"

#include <QGraphicsItem>
#include <QSet>
#include <QMutex>
#include <QPointF>
#include <QRectF>

using namespace deepin_reader;

namespace deepin_reader {
class Annotation;
class Page;
}

class SheetBrowser;
class BrowserWord;
class BrowserAnnotation;

/**
 * @brief The BrowserPage class
 * 用于视图框架中表示一页文档
 */
class BrowserPage : public QGraphicsItem
{
    friend class PageSearchThread;
    friend class PageRenderThread;
    friend class PageViewportThread;
public:
    explicit BrowserPage(SheetBrowser *parent, int index, DocSheet *sheet);

    ~BrowserPage() override;

    /**
     * @brief 文档页缩放后的原区域 不受旋转影响
     * @return
     */
    QRectF boundingRect() const override;

    /**
     * @brief 文档页实际区域
     * @return
     */
    QRectF rect();

    /**
     * @brief scaleFactor
     * 当前缩放因子
     * @return
     */
    qreal scaleFactor();

    /**
     * @brief 设置书签
     * @param hasBookmark 是否存在书签
     */
    void setBookmark(const bool &hasBookmark);

    /**
     * @brief 文档进行加载像显示内容
     * @param scaleFactor 缩放系数
     * @param rotation 旋转角度
     * @param renderLater 是否延迟加载
     * @param force 是否强制更新
     */
    void render(const double &scaleFactor, const Dr::Rotation &rotation, const bool &renderLater = false, const bool &force = false);

    /**
     * @brief 加载局部区域
     * @param scaleFactor 缩放系数
     * @param rect 需要被局部加载的文档区域
     */
    void renderRect(const QRectF &rect);

    /**
     * @brief 加载当前视图区域
     * @param scaleFactor 缩放因子
     */
    void renderViewPort();

    /**
     * @brief 更新书签状态
     */
    void updateBookmarkState();

    /**
     * @brief currentImage
     * 获取整个图片并转成对应宽高
     * @param width 宽
     * @param height 高
     * @param mode 缩放模式
     * @param bSrc 是否可以使用已存在图片缩放
     * @return
     */
    QImage getCurrentImage(int width, int height);

    /**
     * @brief getImagePoint
     * 得到鼠标点周围的片图(放大镜)
     * @param scaleFactor 缩放比例
     * @param point 鼠标点
     * @return鼠标点周围的片图(带缩放)
     */
    QImage getImagePoint(double scaleFactor, QPoint point);

    /**
     * @brief getCurImagePoint
     * 得到鼠标点周围的片图(放大镜)
     * @param point 鼠标点
     * @return 鼠标点周围的片图
     */
    QImage getCurImagePoint(QPointF point);

    /**
     * @brief itemIndex
     * 当前页的编号(从0开始)
     * @return
     */
    int itemIndex();

    /**
     * @brief selectedWords
     * 得到当前选择文字内容
     * @return  选择文字的内容
     */
    QString selectedWords();

    /**
     * @brief setWordSelectable
     * 设置文字是否可选择
     * @param selectable true:可选   false:不可选
     */
    void setWordSelectable(bool selectable);

    /**
     * @brief loadWords
     * 加载文字 如果正在加载则不进行操作 如果已经加载过则按当前缩放改变大小
     */
    void loadWords();

    /**
     * @brief scaleAnnots
     *  更改缩放如果存在注释
     */
    void scaleAnnots(bool force = false);

    /**
     * @brief scaleWords
     *  更改缩放如果存在文字
     */
    void scaleWords(bool force = false);

    /**
     * @brief clearPixmap
     * 删除缓存图片,当距离当前页较远的时候需要被清除以节省内存空间占用
     */
    void clearPixmap();

    /**
     * @brief clearWords
     *  清除文字,,当距离当前页较远的时候需要被清除以节省内存空间占用,如果有文字被选中则不做任何事
     */
    void clearWords();

    /**
     * @brief annotations
     * 当前页中所有注释
     * @return 注释列表
     */
    QList< deepin_reader::Annotation * > annotations();

    /**
     * @brief updateAnnotation
     * 更新注释内容,颜色
     * @param annotation 当前注释
     * @param text 注释内容
     * @param color 注释高亮颜色
     * @return true:更新成功  false:更新失败
     */
    bool updateAnnotation(deepin_reader::Annotation *annotation, const QString &, const QColor &);

    /**
     * @brief addHighlightAnnotation
     * 添加文本高亮注释接口
     * @param text  高亮注释内容
     * @param color 高亮注释颜色
     * @return      当前添加高亮注释
     */
    Annotation *addHighlightAnnotation(QString text, QColor color);

    /**
     * @brief hasAnnotation
     * 当前页注释中是否有这个注释
     * @param annotation 当前注释
     * @return true:有       false:没有
     */
    bool hasAnnotation(deepin_reader::Annotation *annotation);

    /**
     * @brief removeAnnotation
     * 删除某个注释
     * @param annota 具体哪个注释
     * @return true:删除成功, false:删除失败
     */
    bool removeAnnotation(deepin_reader::Annotation *annotation);

    /**
     * @brief addIconAnnotation
     * 添加图标注释
     * @param rect 图标rect
     * @param text 注释内容
     * @return 添加之后的注释,添加失败返回nullptr,反之不为nullptr
     */
    Annotation *addIconAnnotation(const QRectF &, const QString &);

    /**
     * @brief setSearchHighlightRectf
     * 设置当前搜索选择框,在搜索列表中
     * @param rectflst
     */
    void setSearchHighlightRectf(const QVector<PageSection> &sections);

    /**
     * @brief clearSearchHighlightRects
     *结束搜索,清除搜索选择框,清空搜索结果列表
     */
    void clearSearchHighlightRects();

    /**
     * @brief clearSelectSearchHighlightRects
     * 清除高亮选择框,在搜索结果中
     */
    void clearSelectSearchHighlightRects();

    /**
     * @brief searchHighlightRectSize
     * 当前页中搜索到目标的个数
     * @return 目标的个数
     */
    int searchHighlightRectSize();

    /**
     * @brief findSearchforIndex
     * 得到搜索元素的rect,根据编号
     * @param index 编号
     * @return 编号对应的rect
     */
    PageSection findSearchforIndex(int index);

    /**
     * @brief translateRect
     * 将非0度(旋转角度)rect转换成0度rect
     * @param rect 转换之前rect
     * @return 转换之后rect
     */
    QRectF translateRect(const QRectF &rect);

    /**
     * @brief getNorotateRecr
     * 得到没有旋转rect
     * @param rect
     * @return
     */
    QRectF getNorotateRect(const QRectF &rect);

    /**
     * @brief getBrowserAnnotation
     * 获得注释,根据坐标点
     * @param point PageItem坐标系统
     * @return 坐标点下的注释
     */
    BrowserAnnotation *getBrowserAnnotation(const QPointF &point);

    /**
     * @brief getBrowserWord
     * 根据坐标点获得当前的文字
     * @param point PageItem坐标系统
     * @return 坐标点下的文字
     */
    BrowserWord *getBrowserWord(const QPointF &point);

    /**
     * @brief setSelectIconRect
     * 设置图标注释选择框
     * @param draw true:绘制  false:取消绘制
     * @param iconAnnot 图标注释
     */
    void setSelectIconRect(const bool draw, Annotation *iconAnnot = nullptr);

    /**
     * @brief setDrawMoveIconRect
     * 绘制图标注释选择框
     * @param draw true:绘制  false:取消绘制
     */
    void setDrawMoveIconRect(const bool draw);

    /**
     * @brief iconMovePos
     * @return
     */
    QPointF iconMovePos();

    /**
     * @brief setIconMovePos
     * 图标注释选择框移动位置
     * @param movePoint 移动位置
     */
    void setIconMovePos(const QPointF);

    /**
     * @brief moveIconAnnotation
     * 移动注释图标位置
     * @return true:移动成功   false:移动失败
     */
    bool moveIconAnnotation(const QRectF &);

    /**
     * @brief removeAllAnnotation
     * 删除当前页中所有注释
     * @return ture:删除成功       false:删除失败
     */
    bool removeAllAnnotation();

    /**
     * @brief setPageBookMark
     * 在书签附近文字时,有时添加/删除书签无效,特意在brower中先处理
     * @param clickPoint 鼠标点击位置
     */
    void setPageBookMark(const QPointF);

    /**
     * @brief getTopLeftPos
     * 根据旋转角度和item坐标计算出item的左上角坐标
     * @return
     */
    QPointF getTopLeftPos();

private:
    /**
     * @brief handleRenderFinished
     * 渲染缩略图
     * @param scaleFactor 缩放系数
     * @param image 缩略图
     * @param rect 范围
     */
    void handleRenderFinished(const int &pixmapId, const QPixmap &pixmap, const QRect &slice = QRect());

    /**
     * brief handleWordLoaded
     * 加载文字
     * @param words 要加载的文字
     */
    void handleWordLoaded(const QList<Word> &words);

    /**
     * brief handleAnnotationLoaded
     * 加载注释
     * @param annots 要加载的注释
     */
    void handleAnnotationLoaded(const QList<Annotation *> &annots);

    /**
     * @brief bookmarkRect
     * 书签范围
     * @return
     */
    QRectF bookmarkRect();

    /**
     * @brief bookmarkMouseRect
     * 书签图标范围
     * @return
     */
    QRectF bookmarkMouseRect();

    /**
     * @brief 是否属于超大文档
     * @return
     */
    bool isBigDoc();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    bool sceneEvent(QEvent *event) override;

private:
    QMutex m_mutex;

    DocSheet *m_sheet = nullptr;

    SheetBrowser *m_parent = nullptr;

    double  m_scaleFactor = -1;                             //当前被设置的缩放
    int     m_index = 0;                                    //当前索引
    Dr::Rotation m_rotation = Dr::NumberOfRotations;        //当前被设置的旋转

    QPixmap m_pixmap;                                       //当前图片
    QPixmap m_renderPixmap;                                 //当前被绘制图片
    int     m_pixmapId          = 0;                        //当前图片的标识
    bool    m_pixmapIsLastest   = false;                    //当前图示是否最新
    bool    m_pixmapHasRendered = false;                    //当前图片是否已经加载
    double  m_renderPixmapScaleFactor = -1;                 //当前图片的缩放
    bool    m_viewportRendered  = false;                    //图片初始化加载视图窗口

    QList<BrowserWord *> m_words;                           //当前文字
    double m_wordScaleFactor = -1;                          //当前文字的缩放
    bool m_wordIsRendering = false;                         //当前文字是否正在加载
    bool m_wordHasRendered = false;                         //当前文字是否被加载
    bool m_wordNeeded      = false;                         //当前文字是否需要
    bool m_wordSelectable  = false;                         //当前文字是否可以选取

    QList<BrowserAnnotation *> m_annotationItems;           //一个deepin_reader::Annotation可能对应多个annotationItems
    QList<Annotation *> m_annotations;                      //当前注释
    double m_annotScaleFactor = -1;                         //当前注释的缩放
    bool m_annotatinIsRendering = false;                    //当前注释是否正在加载
    bool m_hasLoadedAnnotation = false;                     //是否已经加载注释
    bool m_drawMoveIconRect = false;                        //绘制移动图标注释边框
    BrowserAnnotation *m_lastClickIconAnnotationItem = nullptr;
    QPointF m_drawMoveIconPoint;                            //绘制移动图标注释点

    QVector<PageSection> m_searchLightrectLst;                   //搜索结果
    PageSection m_searchSelectLighRectf;

    bool m_bookmark = false;                                // 当前是否有书签
    int  m_bookmarkState = 0;                               // 当前书签状态 1为on 2为pressed 3为show

    QSizeF m_originSizeF;
};

#endif // BrowserPage_H
