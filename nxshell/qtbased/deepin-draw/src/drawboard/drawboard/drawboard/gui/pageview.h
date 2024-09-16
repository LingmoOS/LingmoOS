// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSVIEW_H
#define CGRAPHICSVIEW_H

#include "csizehandlerect.h"
#include "../../utils/globaldefine.h"
#include "progresslayout.h"

#include <QGraphicsView>
#include <QGestureEvent>
#include <QFileDialog>

#ifdef USE_DTK
DWIDGET_USE_NAMESPACE
#endif

class QUndoStack;
class QUndoCommand;
class CDDFManager;

class CMenu;
class PageContext;
class PageScene;
class Page;
class UndoStack;

/**
 * @brief The CGraphicsView class 图元显示VIEW 类
 *
 */
class DRAWLIB_EXPORT PageView : public QGraphicsView
{
    Q_OBJECT
public:
    /**
     * @brief CGraphicsView 构造函数
     * @param parent
     */
    explicit PageView(Page *parentPage = nullptr);
    ~PageView();

    Page *page() const;

    /**
     * @brief 缩放中心的枚举值，
     * ESceneCenter   表示以画布中心进行缩放
     * EMousePos      表示以鼠标的位置进行缩放
     * ECustomViewPos 表示自定义缩放位置（当前指viewport上的坐标位置）
     */
    enum EScaleCenter { EViewCenter,
                        ESceneCenter,
                        EMousePos,
                        ECustomViewPos
                      };

    /**
     * @brief zoomOut 放大
     */
    void zoomOut(EScaleCenter center = EViewCenter, const QPoint &viewPos = QPoint());

    /**
     * @brief zoomIn 缩小
     */
    void zoomIn(EScaleCenter center = EViewCenter, const QPoint &viewPos = QPoint());

    /**
     * @brief scale 缩放接口
     * @param scale 缩放比例
     */
    void scale(qreal scale, EScaleCenter center = EViewCenter, const QPoint &viewPos = QPoint());

    /**
     * @brief scale 获取缩放接口
     * @return scale 缩放比例
     */
    qreal getScale();

    /**
     * @brief  scaleWithCenter 以某一个中心进行缩放
     * @return factor 缩放因子
     */
    void scaleWithCenter(qreal factor, const QPoint viewPos = QPoint());

    UndoStack *stack()const;

    /**
     * @brief isModified　是否被修改过
     */
    bool isModified() const;

    /**
     * @brief pageScene　绘制的场景指针
     */
    PageScene *pageScene() const;

    /**
     * @brief setCacheEnable　设置是否启动缓冲绘制()
     * @param enable true:会生成一个位图进行绘制加速 false:取消缓冲
     * @param fruzzCurFrame true:生成的位图是当前场景帧 false:全透明的位图
     */
    void setCacheEnable(bool enable, bool fruzzCurFrame = true);

    /**
     * @brief isCacheEnabled　是否当前是启动了缓冲加速的
     */
    bool isCacheEnabled();

    /**
     * @brief cachedPixmap　当前的缓冲图
     */
    QPixmap &cachedPixmap();

    /**
     * @brief activeProxWidget　当前活跃着的代理图元widget对象
     */
    QWidget *activeProxWidget();

    /**
     * @brief activeProxItem　当前活跃着的代理图元
     */
    QGraphicsProxyWidget *activeProxItem();

    /**
     * @brief activeProxDrawItem　当前活跃着的代理图元的父亲业务图元
     */
    PageItem *activeProxDrawItem();

    QMenu *menu()const;

    void alignmentSelectItmes(Qt::AlignmentFlag align);
    void itemsEqulSpaceAlignment(bool bHor);
protected:

    /**
     * @brief showEvent 重载实现一些显示之前的初始化
     */
    void showEvent(QShowEvent *event)override;

    /**
     * @brief wheelEvent 鼠标滚轮事件响应函数
     * @param event 鼠标滚轮事件
     */
    void wheelEvent(QWheelEvent *event) override;

    /**
     * @brief contextMenuEvent 右键菜单响应事件函数
     * @param event 右键菜单
     */
    void contextMenuEvent(QContextMenuEvent *event) override;

    /**
     * @brief resizeEvent 窗口大小更改响应事件函数
     * @param event
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief paintEvent 重绘事件
     * @param event
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief drawItems 绘制所有图元接口
     * @param painter 绘制的画板指针
     * @param numItems 绘制数
     * @param items 图元
     * @param options 参数
     */
    void drawItems(QPainter *painter, int numItems,
                   QGraphicsItem *items[],
                   const QStyleOptionGraphicsItem options[]) override;

    /**
     * @brief leaveEvent 鼠标离开事件
     * @param event
     */
    void leaveEvent(QEvent *event) override;

    /**
     * @brief dropEvent 拖曳加载文件
     * @param e
     */
    void dropEvent(QDropEvent *e) override;

    /**
     * @brief dragEnterEvent 拖文件进入画板
     * @param event
     */
    void dragEnterEvent(QDragEnterEvent *event) override;

    /**
     * @brief dragMoveEvent 拖拽移动事件
     * @param event
     */
    void dragMoveEvent(QDragMoveEvent *event) override;

    /**
     * @brief enterEvent 拖拽事件
     * @param event
     */
    void enterEvent(QEvent *event) override;

protected:
    void keyPressEvent(QKeyEvent *event)override;
    void keyReleaseEvent(QKeyEvent *event)override;
    bool eventFilter(QObject *o, QEvent *e) override;
    bool viewportEvent(QEvent *event)override;

signals:
    /**
     * @brief signalSetScale 设置缩放信号
     * @param scale 缩放比例
     */
    void signalSetScale(const qreal scale);

    void selectionChanged(const QList<PageItem * > &children);

private:
    PRIVATECLASS(PageView)
    friend class PageScene;
};

#endif // CGRAPHICSVIEW_H
