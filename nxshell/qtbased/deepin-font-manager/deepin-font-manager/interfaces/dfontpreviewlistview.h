// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFONTPREVIEWLISTVIEW_H
#define DFONTPREVIEWLISTVIEW_H

#include "dfontpreviewitemdef.h"
#include "dfontpreviewitemdelegate.h"
#include "dfontpreviewlistdatathread.h"
#include "dfontpreviewproxymodel.h"
#include "dfmdbmanager.h"
#include "signalmanager.h"
#include "../views/dfontspinnerwidget.h"
#include "loadfontdatathread.h"

#include <QScrollBar>
#include <QListView>
#include <QPointer>

class DFontMgrMainWindow;

/*************************************************************************
 <Class>         DFontPreviewListView
 <Description>   字体预览列表listview,负责显示每个字体的状态,字体基本信息以及预览效果
 <Author>
 <Note>          null
*************************************************************************/
class DFontPreviewListView : public QListView
{
    Q_OBJECT

public:
    /*************************************************************************
     <Enum>          ClearType
     <Description>   设置对于收藏图标press状态的清空操作类型
     <Author>        UT000539
     <Value>
        <Value1>     MoveClear          Description:清空press状态
        <Value2>     PreviousClear      Description:清空前一个press的状态
     <Note>          null
    *************************************************************************/
    enum ClearType {
        MoveClear,
        PreviousClear
    };

    explicit DFontPreviewListView(QWidget *parent = nullptr);
    ~DFontPreviewListView() override;
    //初始化时,设置界面各个控件的状态
    void initFontListData();
    //初始化listview的代理
    void initDelegate();
    //获取加载数据是否完成的标志位
    bool isListDataLoadFinished();
    //鼠标点击事件
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //鼠标移动事件
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //鼠标双击事件
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //listview设置选中的函数,主要在这里获取当前选中的字体index
    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;
    //为listview设置模型
    void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;
    //listview一行将要移出时触发函数
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) Q_DECL_OVERRIDE;
    //键盘点击事件
    void keyPressEvent(QKeyEvent *event)Q_DECL_OVERRIDE;
    //事件过滤器,用于处理焦点移动事件
    bool eventFilter(QObject *obj, QEvent *event)override;
    //设置右键菜单
    void setRightContextMenu(QMenu *rightMenu);
    //获取当前项的Index
    QModelIndex currModelIndex();
    //返回当前项的itemdata
    DFontPreviewItemData currModelData();
    //获取listview的model
    DFontPreviewProxyModel *getFontPreviewProxyModel()
    {
        return m_fontPreviewProxyModel;
    }
    //清空收藏图标的press状态
    void clearPressState(ClearType type, int currentRow = -2);
    //清空hover状态
    void clearHoverState();
    //文件改动时触发的函数
    void updateChangedFile(const QStringList &pathlist);
    //目录改动时触发的函数
    void updateChangedDir();
    //删除字体
    void deleteFontFiles(QStringList &files, bool force = false);
    //从字体库删除字体
    void deleteCurFonts(QStringList &files, bool force = false);
    //文件改动时触发的函数,对相应文件进行处理
    void changeFontFile(const QString &path, bool force = false);
    //选中字体,并通过传入参数获得各种字体的数目等信息.
    void selectedFonts(const DFontPreviewItemData &curData, int *deleteCnt = nullptr, int *disableSysCnt = nullptr,
                       int *systemCnt = nullptr, qint8 *curFontCnt = nullptr, int *disableCnt = nullptr,
                       QStringList *delFontList = nullptr, QModelIndexList *allIndexList = nullptr,
                       QModelIndexList *disableIndexList = nullptr, QStringList *allMinusSysFontList = nullptr);
    //选中字体调用的字体路径添加函数,用作内联，方便调用
    inline void appendFilePath(QStringList *allFontList, const QString &filePath)
    {
        if ((allFontList != nullptr) && (!allFontList->contains(filePath))) {
            *allFontList << filePath;
        } else if (allFontList != nullptr && allFontList->contains(filePath)) {
            qDebug() << filePath << __FUNCTION__ << " duplicate end";
        }
    }
    //根据文件路径删除字体列表中项
    void deleteFontModelIndex(DFontPreviewItemData &itemData);
    //获取线程锁
    QMutex *getMutex()
    {
        return &m_mutex;
    }
    QMenu *m_rightMenu {nullptr};
    //加入启用字体列表
    void enableFont(const QString &filePath);
    //加入禁用字体列表
    void disableFont(const QString &filePath);
    //批量启用字体
    void enableFonts();
    //批量禁用字体
    void disableFonts();
    //SP3--切换至listview，已有选中且不可见，则滚动到第一并记录位置
    void scrollWithTheSelected();
    //更新shift选中的字体
    void updateShiftSelect(const QModelIndex &modelIndex);
    //判断listview是否在底部和在顶部
    void getAtListViewPosition()
    {
        m_bListviewAtButtom = (verticalScrollBar()->value() >= this->verticalScrollBar()->maximum());
        m_bListviewAtTop = (verticalScrollBar()->value() <= this->verticalScrollBar()->minimum());
    }
    //记录当前选中行的行数
    void setCurrentSelected(int indexRow)
    {
        m_currentSelectedRow = indexRow;
    }

    //记录移除前位置
    void markPositionBeforeRemoved();
    //设置focus状态、设置选中状态
    void refreshFocuses();
    //更新加载动画spinner
    void updateSpinner(DFontSpinnerWidget::SpinnerStyles style, bool force = true);
    //返回当前在用字体的名称
    inline QString getCurFontStrName()
    {
        return QString("%1").arg(m_curFontData.strFontName);
    }
    //返回当前DFontData信息
    inline FontData getCurFontData()
    {
        return m_curFontData;
    }

    //获取是否为tab focus
    bool getIsTabFocus() const
    {
        return m_IsTabFocus;
    }

    //设置是否为tabfocus的标志位
    void setIsTabFocus(bool IsTabFocus)
    {
        m_IsTabFocus = IsTabFocus;
    }
    //Alt+M右键菜单--弹出
    void onRightMenuShortCutActivated();//SP3--Alt+M右键菜单
    //检查鼠标是否处于hover状态
    void checkHoverState();
    //记录操作前的tabfocus状态,用于进行操作后还原
    void setRecoveryTabFocusState(bool recoveryTabFocusState)
    {
        m_recoveryTabFocusState = recoveryTabFocusState;
    }

    //是否由左侧菜单而来的焦点
    void setIsLostFocusOfLeftKey(bool isLostFocusOfLeftKey);
    //是否由slider获取的焦点
    void setIsGetFocusFromSlider(bool isGetFocusFromSlider)
    {
        m_isGetFocusFromSlider = isGetFocusFromSlider;
    }

    //根据是否需要恢复tab聚焦状态进行恢复
    void syncTabStatus(bool restoreFlag = true)
    {
        setIsTabFocus(m_recoveryTabFocusState);
        if (restoreFlag)
            m_recoveryTabFocusState = false;
    }

    //是否需要恢复tab聚焦状态
    bool getRecoveryTabFocusState() const
    {
        return m_recoveryTabFocusState;
    }

    //记录当前是否为tab聚焦状态
    void syncRecoveryTabStatus()
    {
        setRecoveryTabFocusState(m_IsTabFocus);
    }

    void setFontViewHasFocus(bool FontViewHasFocus)
    {
        m_FontViewHasFocus = FontViewHasFocus;
    }

    //获取操作前字体列表焦点状态
    inline bool getFontViewHasFocus() const
    {
        return m_FontViewHasFocus;
    }
    //更新是否选中,正在使用的用户字体标志位状态
    void setUserFontInUseSelected(bool curFontSelected)
    {
        m_userFontInUseSelected = curFontSelected;
    }

    //获取加载数据计时器
    QTimer *getFontLoadTimer() const;

protected:
    //选中切换后触发函数
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

private:
    //初始化链接
    void initConnections();
    //获取当前listview的行数
    int count() const;
    int m_previousPressPos = -1;
    //获取收藏图标尺寸
    inline QRect getCollectionIconRect(const QRect &rect);
    //获取复选框尺寸
    inline QRect getCheckboxRect(const QRect &rect);
    //设置Font data
    void setFontData(const QModelIndex &index, const DFontPreviewItemData &itemData);
    //设置item移除后的选中
    void selectItemAfterRemoved(bool isAtBottom, bool isAtTop, bool isCollectionPage, bool hasDisableFailedFont); //设置item移除后的选中
    //更新选中状态
    void updateSelection();
    //获取一页中列表项的个数
    int getOnePageCount();
    //根据按键设置选中
    void keyPressEventFilter(const QModelIndexList &list, bool isUp, bool isDown, bool isShiftModifier);
    //检查当前是否无选中
    void checkIfHasSelection();
    //鼠标左键press事件处理函数
    void onMouseLeftBtnPressed(const QModelIndex &modelIndex, const QPoint &clickPoint, bool isShiftMdf, bool isCtrlMdf);
    //鼠标右键press事件处理函数
    void onMouseRightBtnPressed(const QModelIndex &modelIndex, bool isShiftMdf);
    void onMouseLeftBtnReleased(const QModelIndex &modelIndex, const QPoint &clickPoint);

private:
    bool m_bLoadDataFinish = false;
    volatile bool m_fontChanged = false;
    bool m_bListviewAtButtom = false;
    bool m_bListviewAtTop = false;
    bool m_isLostFocusOfLeftKey{false};

    bool m_isGetFocusFromSlider{false};
    bool m_IsTabFocus = false;
    bool m_isMousePressNow {false};
    //记录删除前字体列表焦点状态539
    bool m_FontViewHasFocus{false};
//    QPoint lastTouchBeginPos;
//    QPointer<QTimer> touchCheckTimer;

    DFontMgrMainWindow *m_parentWidget;
    QStandardItemModel *m_fontPreviewItemModel {nullptr};

    DFontPreviewItemDelegate *m_fontPreviewItemDelegate {nullptr};
    SignalManager *m_signalManager = SignalManager::instance();

    DFontPreviewProxyModel *m_fontPreviewProxyModel {nullptr};

    DFontPreviewListDataThread *m_dataThread;
    LoadFontDataThread *m_dataLoadThread = nullptr;
    QTimer *m_fontChangeTimer;

    // 打开应用加载数据计时器
    QTimer *m_fontLoadTimer;
    QFont m_curAppFont;
    QModelIndex m_currModelIndex;
    QModelIndex m_pressModelIndex;
    QModelIndex m_hoverModelIndex;
    QMutex m_mutex;
    QStringList m_enableFontList;
    QStringList m_disableFontList;
    QStringList m_currentFont;
    FontData m_curFontData;

    QList<int> m_recoverSelectStateSysfontList; //需要恢复选中状态系统字体idx列表
    QMultiMap<QString, DFontPreviewItemData> m_recoverSelectStateUserfontMultiMap; //需要恢复选中状态用户字体MultiMap
    QRect m_curRect;

    bool m_recoveryTabFocusState = false;//是否需要恢复tab状态
    bool m_userFontInUseSelected{false}; //选中的用户字体字体是否正在被使用
    int m_currentSelectedRow = -1;
    int m_selectAfterDel = -1; //删除后的选中位置，如果为系统字体，更新变量值m_selectAfterDel为-2的状态
    int m_tryCnt = 0;
    qint64 m_curTm {0};


signals:
    //字体列表加载状态
    void onLoadFontsStatus(int type);
    //批量添加item请求
    void multiItemsAdded(QList<DFontPreviewItemData> &data, DFontSpinnerWidget::SpinnerStyles styles);
    //启动时批量添加item请求
    // void startupMultiItemsAdded(QList<DFontPreviewItemData> &data);
    //请求移除某一项的响应函数
    void itemRemoved(DFontPreviewItemData &data);
    //请求安装后的选中响应函数
    void itemsSelected(const QStringList &files, bool isFirstInstall = false);
    void requestInstFontsUiAdded();
    //请求字体列表数目改变函数
    void rowCountChanged();
    //更新删除状态标志位
    void deleteFinished();
    //请求刷新QStandardItemModel信息
    void requestUpdateModel(int deleteCount, bool showSpinner);
    //请求刷新加载图标
    void requestShowSpinner(bool bShow, bool force, DFontSpinnerWidget::SpinnerStyles style);
    //加载用户自己添加的字体
    void loadUserAddFont();
    void refreshListview(QList<DFontPreviewItemData> &data);
    //禁用/启用ttc里的某个字体
    void signalHandleDisableTTC(const QString &filePath, bool &isEnable, bool &isConfirm, bool &isAapplyToAll);

public slots:
    //listview中启用禁用响应函数
    void onEnableBtnClicked(QModelIndexList &itemIndexes, int systemCnt, int curCnt, bool setValue, bool isFromActiveFont = false);
    //listview收藏界面点击后触发函数
    void onCollectBtnClicked(QModelIndexList &indexList, bool setValue, bool isFromCollectFont = false);
    //显示右键菜单
    void onListViewShowContextMenu();
    //数据加载完成响应函数
    void onFinishedDataLoad();
    //根据提供的路径选中哦个listview中的项
    void selectFonts(const QStringList &fileList);
    //listview中添加项响应函数
    void onMultiItemsAdded(QList<DFontPreviewItemData> &data, DFontSpinnerWidget::SpinnerStyles styles);

    //启动时向listview添加项
    void onStartupMultiItemAdded(QList<DFontPreviewItemData> &data);

    //更新当前字体
    void onUpdateCurrentFont();
    //应用字体变化时触发函数
    void onFontChanged(const QFont &font);
    //移除某一项响应函数
    void onItemRemoved(DFontPreviewItemData &itemData);
    //删除字体后更新整个model
    void updateModel(int deleteCount, bool showSpinner = true);
    //切换界面后,滚动到最上方
    void viewChanged();
    //取消删除后,重置之前记录的删除后的位置
    void cancelDel();
    //加载启动过程中未加载的字体
    void loadLeftFonts();
    //
    void onRefreshListview(QList<DFontPreviewItemData> &data);
};

#endif  // DFONTPREVIEWLISTVIEW_H
