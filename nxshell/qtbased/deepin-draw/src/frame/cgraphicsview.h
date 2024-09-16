// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSVIEW_H
#define CGRAPHICSVIEW_H

#include "csizehandlerect.h"
#include "drawshape/globaldefine.h"

#include "widgets/cmenu.h"
#include "progresslayout.h"

#include <DGraphicsView>
#include <QGestureEvent>
#include <QFileDialog>

DWIDGET_USE_NAMESPACE

class QUndoStack;
class QUndoCommand;
class CDDFManager;
class CGraphicsItem;
class CGraphicsRectItem;
class CGraphicsPolygonItem;
class CGraphicsPolygonalStarItem;
class CGraphicsPenItem;
class CMenu;
class CGraphicsLineItem;
//class CGraphicsMasicoItem;
class PageContext;
class PageScene;
class Page;

/**
 * @brief The CGraphicsView class 图元显示VIEW 类
 *
 */
class PageView : public DGraphicsView
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

    /**
     * @brief pushUndoStack　入撤销栈
     */
    void pushUndoStack(QUndoCommand *cmd);

    /**
     * @brief pushUndoCount 入栈记录
     */
    void pushActionCount();

    /**
     * @brief isModified　是否被修改过
     */
    bool isModified() const;

    /**
     * @brief isModifyStashEmpty　是否修改栈为空(不同于isModified,isModifyStashEmpty更严格的表示必须修改栈也是空)
     */
    //bool isModifyStashEmpty();

    /**
     * @brief isKeySpacePressed　是否当前页下空格键处于被点下的状态
     */
    bool isKeySpacePressed();

    /**
     * @brief drawScene　绘制的场景指针
     */
    PageScene *drawScene() const;

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
    CGraphicsItem *activeProxDrawItem();

    /**
     * @brief captureFocus　捕获焦点
     */
    void captureFocus(bool force = true);

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

protected:
    void showMenu(DMenu *pMenu);
    bool gestureEvent(QGestureEvent *event);
protected:
    void pinchTriggered(QPinchGesture *);

    QPoint _pressBeginPos;
    QPoint _recordMovePos;
    bool   _spaceKeyPressed = false;
    QCursor _tempCursor;
signals:
    /**
     * @brief signalSetScale 设置缩放信号
     * @param scale 缩放比例
     */
    void signalSetScale(const qreal scale);

    /**
     * @brief signalTransmitContinueDoOtherThing 传递继续做事的信号
     */
    void signalTransmitContinueDoOtherThing();

    /**
     * @brief singalTransmitEndLoadDDF 传输停止加载DDF信号
     */
    void singalTransmitEndLoadDDF();


public slots:

    /**
     * @brief slotStartLoadDDF 开始加载DDF信号
     * @param rect
     */
    //void slotStartLoadDDF(QRectF rect);

    /**
     * @brief slotQuitCutMode 退出裁剪模式
     */
    //void slotQuitCutMode();

    /**
     * @brief clearScene 清除场景
     */
    //void clearScene();

    /**
     * @bref: updateSelectedItemsAlignment 更新选中图元的对齐方式
     * @param: Qt::AlignmentFlag align 对齐方式
     */
    void updateSelectedItemsAlignment(Qt::AlignmentFlag align);

    /**
      * @bref: alignmentMovPos 对齐图元移动的距离
      * @param: Qt::AlignmentFlag align 对齐方式
      */
    QPointF alignmentMovPos(const QRectF &fatherRect, const QRectF &childRect,
                            Qt::AlignmentFlag align);

public slots:

    /**
     * @brief slotOnCut 剪切图元
     */
    void slotOnCut();

    /**
     * @brief slotOnCopy 复制图元
     */
    void slotOnCopy();

    /**
     * @brief slotOnPaste 粘贴图元
     */
    void slotOnPaste(bool textItemInCenter = true);

    /**
     * @brief slotOnSelectAll 全选图元
     */
    void slotOnSelectAll();

    /**
     * @brief slotOnDelete 删除图元
     */
    void slotOnDelete();

    /**
     * @brief slotOneLayerUp 图元向上一层
     */
    void slotOneLayerUp();

    /**
     * @brief slotOneLayerDown 图元向下一层
     */
    void slotOneLayerDown();

    /**
     * @brief slotBringToFront 置顶图元
     */
    void slotBringToFront();

    /**
     * @brief slotSendTobackAct 置底图元
     */
    void slotSendTobackAct();

    /**
     * @brief slotOnTextCut 剪切文字
     */
    void slotOnTextCut();

    /**
     * @brief slotOnTextCopy 复制文字
     */
    void slotOnTextCopy();

    /**
     * @brief slotOnTextPaste 粘贴文字
     */
    void slotOnTextPaste();

    /**
     * @brief slotOnTextSelectAll 全选文字
     */
    void slotOnTextSelectAll();

    /**
     * @brief slotSetTextAlignment 设置文字对齐方式
     */
    void slotSetTextAlignment(const Qt::Alignment &align);

    /**
     * @brief slotOnTextUndo  文字撤消
     */
    void slotOnTextUndo();

    /**
     * @brief slotOnTextRedo 文字重做
     */
    void slotOnTextRedo();

    /**
     * @brief slotOnTextDelete 文字删除
     */
    void slotOnTextDelete();

    /**
     * @brief slotViewZoomIn 缩小
     */
    void slotViewZoomIn();

    /**
     * @brief slotViewZoomOut 放大
     */
    void slotViewZoomOut();

    /**
     * @brief slotViewOrignal 缩放到原始大小
     */
    void slotViewOrignal();

private:
    qreal m_scale; //记录当前缩放

    CMenu *m_contextMenu;//右键菜单
    QAction *m_cutAct;              //剪切
    QAction *m_copyAct;             //拷贝
    QAction *m_pasteAct;            //粘贴
    QAction *m_pasteActShortCut;    //快捷键粘贴
    QAction *m_selectAllAct;        //全选
    QAction *m_deleteAct;           //删除
    QAction *m_undoAct;             //撤销
    QAction *m_redoAct;             //重做

    QAction *m_group;               //组合
    QAction *m_unGroup;             //取消组合

    DMenu *m_layerMenu;             //图层菜单
    QAction *m_oneLayerUpAct;       //向上一层
    QAction *m_oneLayerDownAct;     //向下一层
    QAction *m_bringToFrontAct;     //置于最顶层
    QAction *m_sendTobackAct;       //置于最底层

    DMenu *m_alignMenu;
    QAction *m_itemsLeftAlign;      //左对齐
    QAction *m_itemsHCenterAlign;   //水平居中对齐
    QAction *m_itemsRightAlign;     //右对齐
    QAction *m_itemsTopAlign;       //顶对齐
    QAction *m_itemsVCenterAlign;   //垂直居中对齐
    QAction *m_itemsBottomAlign;    //底对齐
    QAction *m_itemsHEqulSpaceAlign;//水平等间距对齐
    QAction *m_itemsVEqulSpaceAlign;//垂直等间距对齐

    QAction *m_viewZoomInAction;  // 缩小快捷键
    QAction *m_viewZoomOutAction; // 放大快捷键 ctrl + +
    QAction *m_viewZoomOutAction1; // 放大快捷键 ctrl + =
    QAction *m_viewOriginalAction;


    //QAction *m_cutScence;          //裁剪

    //文字图元右键菜单
    DMenu *m_textMenu;                      //文字菜单
    QAction *m_textCutAction;            //文字剪切
    QAction *m_textCopyAction;          //文字复制
    QAction *m_textPasteAction;         //文字粘贴
    QAction *m_textSelectAllAction;  //文字全选
    QAction *m_textDeleteAction;      //文字删除
    QAction *m_textUndoAct;              //文字撤销
    QAction *m_textRedoAct;              //文字重做
    QAction *m_textLeftAlignAct;      //文字左对齐
    QAction *m_textRightAlignAct;   //文字右对齐
    QAction *m_textCenterAlignAct; //文字水平垂直居中对齐(目前Qt只支持水平方向的对齐)


    QUndoStack *m_pUndoStack;
    bool m_visible;

    bool m_isShowContext;
    bool m_isStopContinuousDrawing;

    QList<QGraphicsItem *> m_loadFromDDF; //从DDF中加载的图圆


    bool    _cacheEnable = false;
    QPixmap _cachePixmap;

    QPointF letfMenuPopPos; // 右键菜单弹出位置

    int userActionCount = 0; // 操作计数

    //进度提升
    //ProgressLayout  *m_progressLayout = nullptr;

private:
    /**
     * @brief initContextMenu 初始化系统右键菜单
     */
    void initContextMenu();

    /**
     * @brief initContextMenuConnection 初始化菜单槽函数
     */
    void initContextMenuConnection();

    /**
     * @brief initTextContextMenu 初始化文字菜单
     */
    void initTextContextMenu();

    /**
     * @brief initTextContextMenuConnection 初始化文字菜单槽函数
     */
    void initTextContextMenuConnection();

    /**
     * @brief initConnection 初始化槽函数
     */
    void initConnection();

    /**
     * @brief setCcdpMenuActionStatus　设置右键菜单剪切复制删除的选项状态
     */
    void setCcdpMenuActionStatus(bool enable);

    /**
     * @brief setClipboardStatus　判断剪切板数据是否为文字
     */
    void setClipboardStatus();

    /**
     * @brief setLayerMenuActionStatus　设置右键菜单图层选项状态
     */
    void setLayerMenuActionStatus(bool layervistual = true);

    /**
     * @brief setAlignMenuActionStatus　根据选择的图元个数显示能进行的对齐操作
     */
    void setAlignMenuActionStatus(bool actionvistual = true);

    /**
     * @brief setTextAlignMenuActionStatus　设置文本图元对齐显示状态
     */
    void setTextAlignMenuActionStatus(CGraphicsItem *tmpitem);

    /**
     * @brief getProgressLayout　粘贴板复制数据提示
     */
    //ProgressLayout *getProgressLayout(bool firstShow = true);

    friend class PageScene;
};

#endif // CGRAPHICSVIEW_H
