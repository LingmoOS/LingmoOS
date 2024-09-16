// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewlistview.h"
#include "dfontinfomanager.h"
#include "dfontpreviewitemdelegate.h"
#include "globaldef.h"
#include "dfmxmlwrapper.h"
#include "views/dfontmgrmainwindow.h"
#include "performancemonitor.h"
#include "dcomworker.h"


#include <DMessageManager>

#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>

DWIDGET_USE_NAMESPACE

DFontPreviewListView::DFontPreviewListView(QWidget *parent)
    : QListView(parent)
    , m_bLoadDataFinish(false)
    , m_parentWidget(qobject_cast<DFontMgrMainWindow *>(parent))
    , m_fontPreviewItemModel(new QStandardItemModel(this))
    , m_dataThread(nullptr)
    , m_fontChangeTimer(new QTimer(this))
{
    qRegisterMetaType<DFontPreviewItemData>("DFontPreviewItemData &");
    qRegisterMetaType<QList<DFontPreviewItemData>>("QList<DFontPreviewItemData> &");
    qRegisterMetaType<QItemSelection>("QItemSelection");
    qRegisterMetaType<DFontSpinnerWidget::SpinnerStyles>("DFontSpinnerWidget::SpinnerStyles");
    qRegisterMetaType<qint64>("qint64 &");
    qRegisterMetaType<QStringList>("QStringList &");
    m_fontPreviewItemModel->setColumnCount(1);
    setFrameShape(QFrame::NoFrame);

    PerformanceMonitor::loadFontStart();
    m_dataThread = DFontPreviewListDataThread::instance(this);


    //启动检测加载状态的定时器
    m_fontLoadTimer = new QTimer(this);
    m_fontLoadTimer->start(500);

    setMouseTracking(true);
    setUpdatesEnabled(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    //准备加载数据，发送信号设置界面各控件状态
    initFontListData();
    initDelegate();
    initConnections();
    installEventFilter(this);

    m_curAppFont.setFamily(QString());
    m_curAppFont.setStyleName(QString());
}

DFontPreviewListView::~DFontPreviewListView()
{
    QFontDatabase::removeAllApplicationFonts();
}

/*************************************************************************
 <Function>      initFontListData
 <Description>   初始化时,设置界面各个控件的状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::initFontListData()
{
    Q_EMIT onLoadFontsStatus(0);
}

/*************************************************************************
 <Function>      isListDataLoadFinished
 <Description>   获取加载数据是否完成的标志位
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        m_bLoadDataFinish            Description:加载数据是否完成的标志位
 <Note>          null
*************************************************************************/
bool DFontPreviewListView::isListDataLoadFinished()
{
    return m_bLoadDataFinish;
}

/*************************************************************************
 <Function>      onFinishedDataLoad
 <Description>   数据加载完成响应函数
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onFinishedDataLoad()
{
    qDebug() << "onFinishedDataLoad thread id = " << QThread::currentThreadId();
    m_bLoadDataFinish = true;
}

/*************************************************************************
 <Function>      onMultiItemsAdded
 <Description>   listview中添加项响应函数
 <Author>        null
 <Input>
    <param1>     data              Description:需要添加项的数据列表
    <param2>     styles            Description:加载动画的样式
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onMultiItemsAdded(QList<DFontPreviewItemData> &data, DFontSpinnerWidget::SpinnerStyles styles)
{
    if (data.isEmpty())
        return;

    QMutexLocker locker(&m_mutex);
    int rows = m_fontPreviewItemModel->rowCount();
    qInfo() << __FUNCTION__ << data.size() << rows;

    int i = 0;
    bool res = m_fontPreviewItemModel->insertRows(rows, data.size());
    if (!res) {
        qDebug() << __FUNCTION__ << "insertRows fail";
        return;
    }

    qInfo() << __FUNCTION__ << "rows = " << m_fontPreviewItemModel->rowCount();
    QList<DFontInfo> fontList;
    for (DFontPreviewItemData &itemData : data) {
        if (itemData.appFontId < 0) {
            int appFontId = QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);

            itemData.appFontId = appFontId;

            //compitable with SP2 Update1
            QString familyName;
            if (!itemData.fontInfo.isSystemFont
                    && (itemData.fontInfo.sp3FamilyName.isEmpty() || itemData.fontInfo.sp3FamilyName.contains(QChar('?')))) {
                QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
                QString firstName;
                if (!fontFamilyList.isEmpty())
                    firstName = fontFamilyList.first();
                for (QString &family : fontFamilyList) {
                    if (family.contains(QChar('?')))
                        continue;
                    familyName = family;
                    break;
                }
                if (familyName.isEmpty())
                    familyName = firstName;

                if (familyName != itemData.fontInfo.sp3FamilyName) {
                    itemData.fontInfo.sp3FamilyName = familyName;
                    fontList << itemData.fontInfo;
                }
            }
            m_dataThread->updateFontId(itemData, appFontId);
        }

        FontData fdata = itemData.fontData;
        if (fdata.strFontName.isEmpty()) {
            qDebug() << __FUNCTION__ << " empty strfontname";
        }

        QModelIndex index = m_fontPreviewItemModel->index(rows + i,   0);
        res = m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
        if (!res)
            qDebug() << __FUNCTION__ << "setData fail";
        setFontData(index, itemData);
        i++;
        // repaint spinner
        if (styles != DFontSpinnerWidget::NoLabel)
            updateSpinner(styles);
    }

    DFontInfoManager::instance()->updateSP3FamilyName(fontList, true);

    if (styles == DFontSpinnerWidget::StartupLoad) {
        if (m_fontLoadTimer->isActive()) {
            m_fontLoadTimer->stop();
            loadLeftFonts();
        }
        Q_EMIT onLoadFontsStatus(1);
    }
}


/*************************************************************************
 <Function>      onStartupMultiItemAdded
 <Description>   启动过程中listview中添加项响应函数
 <Author>        null
 <Input>
    <param1>     data              Description:需要添加项的数据列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onStartupMultiItemAdded(QList<DFontPreviewItemData> &data)
{
    onRefreshListview(data);

    //检查是否有用户没通过应用自己添加的字体
    emit loadUserAddFont();
}

/*************************************************************************
 <Function>      onItemRemoved
 <Description>   移除某一项响应函数
 <Author>        null
 <Input>
    <param1>     itemData            Description:需要移除项的数据
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onItemRemoved(DFontPreviewItemData &itemData)
{
    if (m_fontPreviewProxyModel == nullptr)
        return;

    QFontDatabase::removeApplicationFont(itemData.appFontId);
    deleteFontModelIndex(itemData);

    updateSelection();
}

/*************************************************************************
 <Function>      initDelegate
 <Description>   初始化listview的代理
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::initDelegate()
{
    m_fontPreviewItemDelegate = new DFontPreviewItemDelegate(this);
    this->setItemDelegate(m_fontPreviewItemDelegate);

    m_fontPreviewProxyModel = new DFontPreviewProxyModel(this);
    m_fontPreviewProxyModel->setSourceModel(m_fontPreviewItemModel);
    m_fontPreviewProxyModel->setDynamicSortFilter(true);

    QListView::setModel(m_fontPreviewProxyModel);
}

/*************************************************************************
 <Function>      initConnections
 <Description>   初始化链接
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::initConnections()
{
    connect(this, &DFontPreviewListView::itemsSelected, this, &DFontPreviewListView::selectFonts);
    connect(this, &DFontPreviewListView::multiItemsAdded, this, &DFontPreviewListView::onMultiItemsAdded);
    connect(this, &DFontPreviewListView::refreshListview, this, &DFontPreviewListView::onRefreshListview);
    //  connect(this, &DFontPreviewListView::startupMultiItemsAdded, this, &DFontPreviewListView::onStartupMultiItemAdded);
    connect(this, &DFontPreviewListView::itemRemoved, this, &DFontPreviewListView::onItemRemoved);
    connect(this, &DFontPreviewListView::requestUpdateModel, this, &DFontPreviewListView::updateModel);

    //在拖动滚动条时对字体进行清理,之前在代理中进行清理,减少了清理的次数,避免卡顿
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, [ = ] {
        QFont::cleanup();
    });

    connect(m_signalManager, &SignalManager::cancelDel, this, &DFontPreviewListView::cancelDel);
    /*切换listview后，scrolltotop UT000539*/
    connect(m_signalManager, &SignalManager::changeView, this, &DFontPreviewListView::viewChanged);
    connect(m_signalManager, &SignalManager::clearRecoverList, this, [ = ] {
        m_recoverSelectStateSysfontList.clear();
        m_recoverSelectStateUserfontMultiMap.clear();
        setUserFontInUseSelected(false);
    });

    //打开应用后每隔一段时间去检测一次后台数据加载状态，加载完成后，将所有数据刷新出来
    connect(m_fontLoadTimer, &QTimer::timeout, this, &DFontPreviewListView::loadLeftFonts);
    connect(m_fontChangeTimer, &QTimer::timeout, this, &DFontPreviewListView::onUpdateCurrentFont);


}

/*************************************************************************
 <Function>      count
 <Description>   获取当前listview的行数
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        int             Description:当前listview的行数
 <Note>          null
*************************************************************************/
inline int DFontPreviewListView::count() const
{
    return model()->rowCount(rootIndex());
}

/*************************************************************************
 <Function>      cancelDel
 <Description>   取消删除后,重置之前记录的删除后的位置
 <Author>        UT000539
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::cancelDel()
{
    m_selectAfterDel = -1;
    setFontViewHasFocus(false);
}

/*************************************************************************
 <Function>      loadLeftFonts
 <Description>   加载启动过程中未加载的字体
 <Author>        UT000442
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::loadLeftFonts()
{
    qInfo() << QThread::currentThreadId() << "DFontPreviewListView" << __func__;
    if (isListDataLoadFinished()) {
        m_fontLoadTimer->stop();
//        m_parentWidget->showSpinner(DFontSpinnerWidget::Load);
        qDebug() << DFMDBManager::recordList.count() << endl;
        qDebug() << QThread::currentThreadId() << __func__ << "------------";

        m_dataLoadThread = new LoadFontDataThread(DFMDBManager::recordList, this);
        m_dataLoadThread->start();

        connect(m_dataLoadThread, &LoadFontDataThread::dataLoadFinish, this, &DFontPreviewListView::onStartupMultiItemAdded);
    }
}

void DFontPreviewListView::onRefreshListview(QList<DFontPreviewItemData> &data)
{
    QMutexLocker locker(&m_mutex);
    int rows = m_fontPreviewItemModel->rowCount();
    qInfo() << __FUNCTION__ << data.size() << rows;

    bool res = m_fontPreviewItemModel->insertRows(rows, data.size());
    if (!res) {
        qDebug() << __FUNCTION__ << "insertRows fail";
        return;
    }
    int i = 0;

    for (DFontPreviewItemData &itemData : data) {
        if (!itemData.fontData.isEnabled()) { // 字体被禁用时，让应用临时可用该字体进行预览
            QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);
        }

        FontData fdata = itemData.fontData;
        QModelIndex index = m_fontPreviewItemModel->index(rows + i,   0);
        res = m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
        if (itemData.fontInfo.fullname.compare("04b_09") == 0) {
            qDebug() << __FUNCTION__ << itemData.fontInfo.fullname;
        }
        qDebug() << __FUNCTION__ << itemData.fontInfo.fullname;
        if (!res)
            qDebug() << __FUNCTION__ << "setData fail";
        setFontData(index, itemData);
        i++;
    }
}

/*************************************************************************
 <Function>      viewChanged
 <Description>   切换界面后,滚动到最上方
 <Author>        UT000539
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::viewChanged()
{
    scrollToTop();
    m_currentSelectedRow = -1;
}

/*记录移除前位置*/
/*************************************************************************
 <Function>      markPositionBeforeRemoved
 <Description>   记录移除前位置
 <Author>        UT000539
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::markPositionBeforeRemoved()
{
    QModelIndexList deleteFontList = selectedIndexes();
    if (deleteFontList.count() > 0) {
        qSort(deleteFontList.begin(), deleteFontList.end(), qGreater<QModelIndex>());
        QVariant varModel = m_fontPreviewProxyModel->data(deleteFontList.last(), Qt::DisplayRole);
        //获取首个选中字体，判断是否为系统字体
        FontData fdata = varModel.value<FontData>();
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);
        //如果为系统字体，更新变量值m_selectAfterDel为-2的状态
        if (itemData.fontInfo.isSystemFont) {
            m_selectAfterDel = -2;
        } else {
            m_selectAfterDel = deleteFontList.last().row();
        }
    }
}

/*************************************************************************
 <Function>      refreshFocuses
 <Description>   设置focus状态、设置选中状态
 <Author>        UT000539
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::refreshFocuses()
{
    qDebug() << __FUNCTION__;
    if (isVisible())
        setFocus(Qt::MouseFocusReason);
}

/*************************************************************************
 <Function>      getOnePageCount
 <Description>   获取一页中列表项的个数
 <Author>        UT000539
 <Input>         null
 <Return>        int             Description:列表项的个数
 <Note>          null
*************************************************************************/
int DFontPreviewListView::getOnePageCount()
{
    const int defaultCount = 12;
    int height = viewport()->height();
    const QModelIndex idx = getFontPreviewProxyModel()->index(0, 0);
    if (!idx.isValid())
        return defaultCount;
    const QStyleOptionViewItem option;
    QSize size = m_fontPreviewItemDelegate->sizeHint(option, idx);
    int itemHeight = size.height();
    int  count = height / itemHeight;
    return count;
}

/*************************************************************************
 <Function>      updateSpinner
 <Description>   更新加载动画spinner
 <Author>        null
 <Input>
    <param1>     style            Description:spinner的种类
    <param2>     force            Description:是否强制刷新
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::updateSpinner(DFontSpinnerWidget::SpinnerStyles style, bool force)
{
    qint64 curtm = QDateTime::currentMSecsSinceEpoch();
    //超过500ms刷新
    if (curtm - m_curTm >= 350) {
        Q_EMIT requestShowSpinner(true, force, style);
        m_curTm = QDateTime::currentMSecsSinceEpoch();
    }
}

/*************************************************************************
 <Function>      updateModel
 <Description>   删除字体后更新整个model
 <Author>        null
 <Input>
    <param1>     showSpinner            Description:显示加载图标的样式
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::updateModel(int deleteCount, bool showSpinner)
{
    getAtListViewPosition();
    int rowCnt = m_fontPreviewItemModel->rowCount();
    m_fontPreviewItemModel->removeRows(0, rowCnt);
    for (int i = rowCnt; i >= 0; i--) {
        QStandardItem *item = m_fontPreviewItemModel->takeItem(i);
        if (item)
            delete item;
    }

    m_fontPreviewItemModel->clear();
    delete m_fontPreviewItemModel;
    delete m_fontPreviewProxyModel;

    m_fontPreviewItemModel = new QStandardItemModel(this);
    m_fontPreviewItemModel->setColumnCount(1);
    m_fontPreviewProxyModel = new DFontPreviewProxyModel(this);
    m_fontPreviewProxyModel->setSourceModel(m_fontPreviewItemModel);
    setModel(m_fontPreviewProxyModel);

    m_fontPreviewProxyModel->setFilterKeyColumn(0);

    if (m_parentWidget)
        m_fontPreviewProxyModel->setFilterGroup(m_parentWidget->currentFontGroup());
    QList<DFontPreviewItemData> modelist = m_dataThread->getFontModelList();

    DFontSpinnerWidget::SpinnerStyles spinnerstyle = (showSpinner) ? DFontSpinnerWidget::Delete : DFontSpinnerWidget::NoLabel;
    onMultiItemsAdded(modelist, spinnerstyle);

    Q_EMIT requestShowSpinner(false, true, DFontSpinnerWidget::Delete);

    //设置预览大小
    emit m_signalManager->fontSizeRequestToSlider();

    //设置字体删除后的选中
    selectItemAfterRemoved(m_bListviewAtButtom, m_bListviewAtTop, false, false);

    //删除之后设置焦点
    qDebug() << m_FontViewHasFocus << endl;
    if (m_FontViewHasFocus) {
        refreshFocuses();
        setFontViewHasFocus(false);
    }
    Q_EMIT rowCountChanged();
    Q_EMIT deleteFinished();
    syncTabStatus();
    m_recoveryTabFocusState = false;

    PerformanceMonitor::deleteFontFinish(deleteCount);
}

/*************************************************************************
 <Function>      getCollectionIconRect
 <Description>   获取收藏图标尺寸
 <Author>        null
 <Input>
    <param1>     rect             Description:listview中一项的范围
 <Return>        QRect            Description:收藏图标尺寸
 <Note>          null
*************************************************************************/
QRect DFontPreviewListView::getCollectionIconRect(const QRect &rect)
{
    int collectIconSize = 22 + 10;
    return QRect(rect.right() - 10 - 33, rect.top() + 10 - 5, collectIconSize, collectIconSize);
}

/*************************************************************************
 <Function>      getCheckboxRect
 <Description>   获取复选框尺寸
 <Author>        null
 <Input>
    <param1>     rect            Description:listview中一项的范围
 <Return>        QRect            Description:复选框尺寸
 <Note>          null
*************************************************************************/
QRect DFontPreviewListView::getCheckboxRect(const QRect &rect)
{
    int checkBoxSize = 20 + 10;
    return QRect(rect.left() + 20, rect.top() + 10 - 5, checkBoxSize, checkBoxSize);
}

/*************************************************************************
 <Function>      onUpdateCurrentFont
 <Description>   更新当前字体
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onUpdateCurrentFont()
{
    if (!m_fontChanged) {
        m_fontChangeTimer->stop();
        return;
    }

    QStringList curFont = DFontInfoManager::instance()->getCurrentFontFamily();
    QString filePath = DFontInfoManager::instance()->getFontPath();
    qDebug() << __FUNCTION__ << "begin " << curFont << m_fontChanged;

    if (curFont.isEmpty() || curFont.size() < 3) {
        qDebug() << __FUNCTION__ << curFont << " is invalid";
        m_fontChanged = false;
        m_fontChangeTimer->stop();
        return;
    }

    if ((m_currentFont.size() == curFont.size()) && (m_currentFont.at(1) == curFont.at(1)) && (m_currentFont.at(2) == curFont.at(2))) {
        qDebug() << __FUNCTION__ << " will check 2 times : " << m_tryCnt;
        m_tryCnt++;
        if (m_tryCnt > 2) {
            m_tryCnt = 0;
            m_fontChanged = false;
            m_fontChangeTimer->stop();
        }
        return;
    }

    for (DFontPreviewItemData &itemData : m_dataThread->getFontModelList()) {
        if (itemData.fontInfo.filePath != filePath)
            continue;

        if (itemData.fontInfo.isSystemFont && curFont.at(2) != itemData.fontInfo.styleName)
            continue;

        bool found = false;
        if (itemData.fontInfo.familyName == curFont.at(1)) {
            found = true;
        } else {
            QStringList families = DFontInfoManager::instance()->getFontFamilyStyle(itemData.fontInfo.filePath);
            if (families.contains(curFont.at(1))
                    || (filePath == itemData.fontInfo.filePath && !itemData.fontInfo.isSystemFont))
                found = true;
        }

        if (found) {
            m_currentFont = curFont;
            m_curFontData = itemData.fontData;
            qDebug() << __FUNCTION__ << " found " << curFont << itemData.fontData.strFontName;
            m_fontChanged = false;
            m_fontChangeTimer->stop();
            break;
        }
    }
    qDebug() << __FUNCTION__ << "end" << m_currentFont;
}

/*************************************************************************
 <Function>      onFontChanged
 <Description>   应用字体变化时触发函数
 <Author>        null
 <Input>
    <param1>     font            Description:当前使用字体
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onFontChanged(const QFont &font)
{
    if (font.weight() == m_curAppFont.weight() && font.style() == m_curAppFont.style()
            && font.stretch() == m_curAppFont.stretch() && font.styleHint() == m_curAppFont.styleHint()
            && font.styleStrategy() == m_curAppFont.styleStrategy() && font.fixedPitch() == m_curAppFont.fixedPitch()
            && font.family() == m_curAppFont.family() && font.styleName() == m_curAppFont.styleName()
            && font.hintingPreference() == m_curAppFont.hintingPreference()) {
        qDebug() << __FUNCTION__ << "same fontChanged , ignore it " << font.family();
        return;
    }

    m_fontChangeTimer->stop();
    qDebug() << __FUNCTION__ << "Font changed " << m_currentFont << font.family() << font.style() << m_currentFont;
    m_fontChanged = true;
    onUpdateCurrentFont();
    if (m_fontChanged) {
        m_fontChangeTimer->setInterval(300);
        m_fontChangeTimer->start();
    }
    m_curAppFont = font;
    qDebug() << __FUNCTION__ << " end";
}

void DFontPreviewListView::setFontData(const QModelIndex &index, const DFontPreviewItemData &itemData)
{
    QString fontPreview = itemData.fontInfo.defaultPreview;
    if (fontPreview.isEmpty()) {
        if (itemData.fontInfo.previewLang == FONT_LANG_CHINESE) {
            fontPreview = FTM_DEFAULT_PREVIEW_CN_TEXT;
        } else if (itemData.fontInfo.previewLang & FONT_LANG_ENGLISH) {
            fontPreview = FTM_DEFAULT_PREVIEW_EN_TEXT;
        } else {
            fontPreview = FTM_DEFAULT_PREVIEW_DIGIT_TEXT;
        }
    }
    FontDelegateData data(itemData.fontInfo.sp3FamilyName.isEmpty()
                          ? itemData.fontInfo.familyName : itemData.fontInfo.sp3FamilyName,
                          itemData.fontInfo.styleName,
                          fontPreview);

    bool res = m_fontPreviewItemModel->setData(index, QVariant::fromValue(data),
                                               DFontPreviewItemDelegate::FontFamilyStylePreviewRole);
    if (!res) {
        qDebug() << __FUNCTION__ << " fail ";
    }
}

/*************************************************************************
 <Function>      selectItemAfterRemoved
 <Description>   设置item移除后的选中
 <Author>        UT000539
 <Input>
    <param1>     isAtBottom            Description:是否在列表底部
    <param2>     isAtTop               Description:是否在列表顶部
    <param2>     isCollectionPage      Description:是否是收藏页取消收藏操作
    <param2>     hasDisableFailedFont  Description:是否有不能禁用的系统字体
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::selectItemAfterRemoved(bool isAtBottom, bool isAtTop, bool isCollectionPage, bool hasDisableFailedFont)
{
    int param = getOnePageCount();
    if (m_selectAfterDel != -1) {
        if ((m_selectAfterDel != -2 || isCollectionPage)
                && !m_userFontInUseSelected
                && 0 == m_recoverSelectStateSysfontList.count()
                && 0 == m_recoverSelectStateUserfontMultiMap.count()) {
            int nextIndexRow = -1;
            //删除最后一个
            if (m_selectAfterDel == this->count()) {
                if (count() > 0) {
                    //上移选中
                    nextIndexRow = count() - 1;
                }
                scrollToBottom();
            } else {
                //超过一页时
                if (this->count() > param) {
                    //删除第一页的字体
                    if (m_selectAfterDel <= param) {
                        if (isAtTop)
                            scrollToTop();
                        else {
                            scrollTo(m_fontPreviewProxyModel->index(m_selectAfterDel, 0));
                        }
                        nextIndexRow = m_selectAfterDel;
                    }
                    //删除最后一页的字体
                    else if (m_selectAfterDel >= this->count() - param) {
                        if (isAtBottom) {
                            nextIndexRow = m_selectAfterDel - 1;
                            if (m_selectAfterDel == this->count() - param) {
                                scrollTo(m_fontPreviewProxyModel->index(m_selectAfterDel - 1, 0));
                            } else {
                                scrollToBottom();
                            }
                        } else {
                            nextIndexRow = m_selectAfterDel;
                            scrollTo(m_fontPreviewProxyModel->index(m_selectAfterDel, 0));
                        }
                    }
                    //删除中间位置的字体
                    else {
                        nextIndexRow = m_selectAfterDel;
                    }
                }
                //只有一页时
                else {
                    if (m_selectAfterDel <= param) {
                        if (m_fontPreviewProxyModel->index(m_selectAfterDel, 0).isValid()) {
                            nextIndexRow = m_selectAfterDel;
                        } else {
                            nextIndexRow = m_selectAfterDel - 1;
                        }
                        scrollToTop();
                    }
                }
            }
            //更新选中位置
            setCurrentSelected(nextIndexRow);
            //不能禁用的系统字体，保留选中状态
            if (!hasDisableFailedFont) {
                setCurrentIndex(m_fontPreviewProxyModel->index(nextIndexRow, 0));
            }
        }

        // 跳过未删除的用户字体(不包括当前使用的字体)，恢复选中状态
        QMultiMap<QString, DFontPreviewItemData>::iterator it = m_recoverSelectStateUserfontMultiMap.begin();
        while (it != m_recoverSelectStateUserfontMultiMap.end()) {
            for (int i = 0; i < count(); i++) {
                QModelIndex modelIndex = m_fontPreviewProxyModel->index(i, 0);
                QVariant varModel = m_fontPreviewProxyModel->data(modelIndex, Qt::DisplayRole);
                FontData fdata = varModel.value<FontData>();
                if (it.value().fontData == fdata) {
                    selectionModel()->select(m_fontPreviewProxyModel->index(i, 0), QItemSelectionModel::Select);
                    break;
                }
            }
            ++it;
        }
        m_recoverSelectStateUserfontMultiMap.clear();

        //不能删除的系统列表，恢复选中状态
        if (m_recoverSelectStateSysfontList.count() > 0) {
            if (!isCollectionPage) {
                for (auto &idx : m_recoverSelectStateSysfontList) {
                    selectionModel()->select(m_fontPreviewProxyModel->index(idx, 0), QItemSelectionModel::Select);
                }
                //如果选中项包含系统字体，则删除后滚动到恢复选中的系统字体
                scrollTo(selectedIndexes().last());
            }
            m_recoverSelectStateSysfontList.clear();
        }
        //操作前已选中的当前正在使用的用户字体，恢复选中状态
        if (m_userFontInUseSelected) {
            for (int i = count() - 1; i >= 0; i--) {
                FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(m_fontPreviewProxyModel->index(i, 0)));
                if (fdata == m_curFontData) {
                    selectionModel()->select(m_fontPreviewProxyModel->index(i, 0), QItemSelectionModel::Select);
                    break;
                }
            }
            setUserFontInUseSelected(false);
        }
    }
}

/*************************************************************************
 <Function>      deleteFontModelIndex
 <Description>   根据文件路径删除字体列表中项
 <Author>        null
 <Input>
    <param1>     itemData            Description:需要删除的字体
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::deleteFontModelIndex(DFontPreviewItemData &itemData)
{
//    qDebug() << __FUNCTION__ << "m_fontPreviewProxyModel rowcount = " << m_fontPreviewProxyModel->rowCount();
    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    QStringList str;

    for (int i = 0; i < m_fontPreviewProxyModel->sourceModel()->rowCount(); i++) {
        QModelIndex modelIndex = m_fontPreviewProxyModel->sourceModel()->index(i, 0);
        QVariant varModel = m_fontPreviewProxyModel->sourceModel()->data(modelIndex, Qt::DisplayRole);

        FontData fdata = varModel.value<FontData>();
        if (itemData.fontInfo.filePath.isEmpty())
            continue;

        if (itemData.fontData == fdata) {
            //记录待删除字体当前的位置索引
            m_selectAfterDel = m_fontPreviewProxyModel->mapFromSource(modelIndex).row();
            m_fontPreviewProxyModel->sourceModel()->removeRow(i);
            break;
        }
    }
}

/*************************************************************************
 <Function>      selectFonts
 <Description>   根据提供的路径选中哦个listview中的项
 <Author>        null
 <Input>
    <param1>     fileList            Description:需要选中字体的路径
 <Return>        null                Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::selectFonts(const QStringList &fileList)
{
    if (fileList.isEmpty())
        return;

    QItemSelection selection;
    DFontPreviewProxyModel *filterModel = getFontPreviewProxyModel();
    filterModel->setFilterGroup(DSplitListWidget::UserFont);
    qDebug() << __FUNCTION__ << " fileList size " << fileList.size() << ", row count " << filterModel->rowCount();
    for (int i = 0; i < filterModel->rowCount(); ++i) {
        QModelIndex index = filterModel->index(i, 0);
        FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(index));
        QMutexLocker locker(&m_mutex);
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);
        locker.unlock();
        if (itemData.fontInfo.filePath.isEmpty()) {
            qDebug() << __FUNCTION__ << fdata.strFontName;
            continue;
        }

        if (fileList.contains(itemData.fontInfo.filePath)) {
            QModelIndex left = m_fontPreviewProxyModel->index(index.row(), 0);
            QModelIndex right = m_fontPreviewProxyModel->index(index.row(), m_fontPreviewProxyModel->columnCount() - 1);
            QItemSelection sel(left, right);
            selection.merge(sel, QItemSelectionModel::Select);
        }
    }

    qDebug() << " selection size " << selection.size();

    QItemSelectionModel *selection_model = selectionModel();
    if (selection.size() > 0)  {
        selection_model->reset();
        selection_model->select(selection, QItemSelectionModel::Select);
    }

    Q_EMIT requestInstFontsUiAdded();
    //同步tab聚焦状态
    syncTabStatus();
}

/*************************************************************************
 <Function>      selectionChanged
 <Description>   选中切换后触发函数
 <Author>        null
 <Input>
    <param1>     selected              Description:下个选中项
    <param2>     deselected            Description:取消选中项
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QListView::selectionChanged(selected, deselected);
}

/*************************************************************************
 <Function>      mouseMoveEvent
 <Description>   鼠标移动事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::mouseMoveEvent(QMouseEvent *event)
{
    if (nullptr == m_fontPreviewProxyModel) {
        return;
    }

    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);
    if (modelIndex.isValid()) {
        QRect rect = visualRect(modelIndex);
        QRect collectIconRect = getCollectionIconRect(rect);

        FontData itemData = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndex));

        if (m_hoverModelIndex.row() != modelIndex.row())
            clearHoverState();
        clearPressState(ClearType::PreviousClear, modelIndex.row());
        if (collectIconRect.contains(clickPoint)) {

            if (m_isMousePressNow) {
                if (itemData.getHoverState() != IconPress) {
                    itemData.setHoverState(IconPress);
                    m_previousPressPos = modelIndex.row();
                    m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
                }
            } else {
                if (itemData.getHoverState() != IconHover) {
                    itemData.setHoverState(IconHover);
                    m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
                }
                m_hoverModelIndex = modelIndex;
            }
        } else {
            if (itemData.getHoverState() != IconNormal) {
                itemData.setHoverState(IconNormal);
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
            }
        }
    }
    return  QListView::mouseMoveEvent(event);
}

/*************************************************************************
 <Function>      mousePressEvent
 <Description>   鼠标点击事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << __FUNCTION__ << event->type() << event->button();
    //检查当前是否有选中，恢复起始位
    checkIfHasSelection();
    QListView::mousePressEvent(event);
    //应该设置焦点，否则鼠标在其他区域release会导致缺失焦点。

    setFocus(Qt::MouseFocusReason);
    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);

    bool isShiftMd = (event->modifiers() == Qt::ShiftModifier) ? true : false;
    bool isCtrlMd = (event->modifiers() == Qt::ControlModifier) ? true : false;

    if (modelIndex.isValid()) {
        if (event->button() == Qt::LeftButton) {
            onMouseLeftBtnPressed(modelIndex, clickPoint, isShiftMd, isCtrlMd);
        } else if (event->button() == Qt::RightButton) {
            onMouseRightBtnPressed(modelIndex, isShiftMd);
        } else if (event->button() == Qt::MidButton) {
            if (!isShiftMd && !isCtrlMd) {
                clearSelection();
                setCurrentIndex(modelIndex);
                setCurrentSelected(modelIndex.row());
            } else if (selectedIndexes().count() == 0) {
                setIsTabFocus(false);
            } else if (isShiftMd) {
                updateShiftSelect(modelIndex);
            } else if (isCtrlMd) {
                setCurrentSelected(modelIndex.row());
            }
        }
    } else {
        if (!isShiftMd && !isCtrlMd) {
            clearSelection();
            setIsTabFocus(false);
        }
    }
}

/*************************************************************************
 <Function>      onMouseLeftBtnPressed
 <Description>   鼠标左键press事件处理函数
 <Author>
 <Input>
    <param1>     modelIndex      Description:当前press位置的modelIndex
    <param1>     clickPoint      Description:当前press位置点--用于收藏按钮状态判断
    <param2>     isShiftMdf      Description:是否shift组合键被按下
    <param2>     isCtrlMdf       Description:是否ctrl组合键被按下
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onMouseLeftBtnPressed(const QModelIndex &modelIndex, const QPoint &clickPoint, bool isShiftMdf, bool isCtrlMdf)
{
    m_isMousePressNow = true;
    if (isShiftMdf) {
        updateShiftSelect(modelIndex);
    } else if (isCtrlMdf && selectedIndexes().count() == 0) {
        setIsTabFocus(false);
    } else {
        //左键单击
        m_currentSelectedRow = modelIndex.row();
        m_curRect = visualRect(modelIndex);

        QRect collectIconRect = getCollectionIconRect(m_curRect);

        FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndex));
        if (collectIconRect.contains(clickPoint)) {
            if (fdata.getHoverState() != IconPress) {
                fdata.setHoverState(IconPress);
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
            }
            m_pressModelIndex = modelIndex;
        } else if (fdata.getHoverState() != IconNormal) {
            fdata.setHoverState(IconNormal);
            m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
        }
    }
}

/*************************************************************************
 <Function>      onMouseRightBtnPressed
 <Description>   鼠标右键press事件处理函数
 <Author>
 <Input>
    <param1>     modelIndex      Description:当前press位置的modelIndex
    <param2>     isShiftMdf      Description:是否shift组合键被按下
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onMouseRightBtnPressed(const QModelIndex &modelIndex, bool isShiftMdf)
{
    if (isShiftMdf) {
        clearSelection();
        if (m_currentSelectedRow <= modelIndex.row()) {
            for (auto it = m_currentSelectedRow; it <= modelIndex.row(); it++) {
                selectionModel()->select(m_fontPreviewProxyModel->index(it, 0), QItemSelectionModel::Select);
            }
        } else {
            for (auto it = modelIndex.row(); it <= m_currentSelectedRow; it++) {
                selectionModel()->select(m_fontPreviewProxyModel->index(it, 0), QItemSelectionModel::Select);
            }
        }
    } else {
        setCurrentSelected(modelIndex.row());
    }
    //右键单击
    if (!selectedIndexes().contains(modelIndex) && !isShiftMdf) {
        this->setCurrentIndex(modelIndex);
    }
    //恢复normal状态
    FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndex));
    if (fdata.getHoverState() != IconNormal) {
        fdata.setHoverState(IconNormal);
        m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
    }
    //记录焦点状态
    syncRecoveryTabStatus();
    qDebug() << hasFocus() << endl;
    setFontViewHasFocus(hasFocus());
    //弹出右键菜单
    onListViewShowContextMenu();
    //菜单关闭之后
    emit m_signalManager->menuHidden();
    refreshFocuses();
}

/*************************************************************************
 <Function>      mouseReleaseEvent
 <Description>   鼠标释放事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::MidButton == event->button()) {
        return;
    }
    QListView::mouseReleaseEvent(event);

    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);
    m_currModelIndex = modelIndex;

    if (event->button() == Qt::LeftButton) {
        onMouseLeftBtnReleased(modelIndex, clickPoint);
    }
}

/*************************************************************************
 <Function>      onMouseLeftBtnReleased
 <Description>   鼠标左键release事件处理函数
 <Author>
 <Input>
    <param1>     modelIndex      Description:当前press位置的modelIndex
    <param1>     clickPoint      Description:当前press位置点--用于收藏按钮状态判断
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onMouseLeftBtnReleased(const QModelIndex &modelIndex, const QPoint &clickPoint)
{
    QModelIndexList indexList;
    FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndex));

    QRect rect = visualRect(modelIndex);
    QRect checkboxRealRect = getCheckboxRect(rect);
    QRect collectIconRect = getCollectionIconRect(rect);

    m_isMousePressNow = false;
    clearPressState(ClearType::MoveClear);
    if (checkboxRealRect.contains(clickPoint)) {
        //触发启用/禁用字体
        int sysFontCnt = (fdata.isEnabled() && fdata.isSystemFont) ? 1 : 0;
        // 选中字体与当前使用的字体为同一字体文件
        // 对于ttc字体集，即使不是同一个字体名称，它们也是同一个字体文件，
        int curFontCnt = ((fdata == m_curFontData) ||
                          (m_dataThread->getFontData(fdata).fontInfo.filePath
                           == m_dataThread->getFontData(m_curFontData).fontInfo.filePath)) ? 1 : 0;
        if (sysFontCnt == 0 && curFontCnt == 0)
            indexList << modelIndex;
        onEnableBtnClicked(indexList, sysFontCnt, curFontCnt, !fdata.isEnabled(),
                           m_parentWidget->currentFontGroup() == DSplitListWidget::ActiveFont);
    } else if (collectIconRect.contains(clickPoint)) {
        //恢复normal状态
        if (fdata.getHoverState() != IconNormal) {
            fdata.setHoverState(IconNormal);
            m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
        }
        //触发收藏/取消收藏
        indexList << modelIndex;
        onCollectBtnClicked(indexList, !fdata.isCollected(), (m_parentWidget->currentFontGroup() == DSplitListWidget::CollectFont));
    }
    QModelIndex modelIndexNew = modelIndex;
    //我的收藏界面,index点击之后会涉及到index的变化,所以需要重新获取一遍
    if (m_parentWidget->currentFontGroup() == DSplitListWidget::CollectFont) {
        qDebug() << __FUNCTION__ << " collect font grp";
        modelIndexNew = currModelIndex();
        rect = visualRect(modelIndexNew);
        checkboxRealRect = getCheckboxRect(rect);
        collectIconRect = getCollectionIconRect(rect);
    }

    fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndexNew));

    if (collectIconRect.contains(clickPoint)) {
        if (fdata.getHoverState() != IconHover) {
            fdata.setHoverState(IconHover);
            m_fontPreviewProxyModel->setData(modelIndexNew, QVariant::fromValue(fdata), Qt::DisplayRole);
        }
        m_hoverModelIndex = modelIndexNew;
    } else if (fdata.getHoverState() != IconNormal) {
        fdata.setHoverState(IconNormal);
        m_fontPreviewProxyModel->setData(modelIndexNew, QVariant::fromValue(fdata), Qt::DisplayRole);
    }
}

/*************************************************************************
 <Function>      getTimer
 <Description>   获取字体加载数据定时器
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QTimer *DFontPreviewListView::getFontLoadTimer() const
{
    return m_fontLoadTimer;
}

/*************************************************************************
 <Function>      mouseDoubleClickEvent
 <Description>   鼠标双击事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QListView::mouseDoubleClickEvent(event);
}

/*************************************************************************
 <Function>      ifHasSelection
 <Description>   检查当前是否无选中，恢复起始位
 <Author>        UT000539
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::checkIfHasSelection()
{
    if (selectedIndexes().count() == 0)
        m_currentSelectedRow = -1;
}

/*************************************************************************
 <Function>      setSelection
 <Description>   listview设置选中的函数,主要在这里获取当前选中的字体index
 <Author>        null
 <Input>
    <param1>     rect            Description:需要选中的区域范围
    <param2>     command            Description:SelectionFlag
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::setSelection(const QRect &rect,
                                        QItemSelectionModel::SelectionFlags command)
{
    QPoint clickPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(clickPoint);
    m_currModelIndex = modelIndex;

    QListView::setSelection(rect, command);
}

/*************************************************************************
 <Function>      setModel
 <Description>   为listview设置模型
 <Author>        null
 <Input>
    <param1>     model            Description:需要设置的模型
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::setModel(QAbstractItemModel *model)
{
//    m_fontPreviewItemModel = qobject_cast<QStandardItemModel *>(model);
    QListView::setModel(model);
}

/*************************************************************************
 <Function>      rowsAboutToBeRemoved
 <Description>   listview一行将要移出时触发函数
 <Author>        null
 <Input>
    <param1>     parent            Description:对应被删除的夫索引
    <param2>     start            Description:第一个眼删除的行号
    <param3>     end            Description:最后一个要删除的行号
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if (selectionModel()->selectedIndexes().count() > 0) {
        //bug 37050 ut000442 当start与end位置相同时表示这一行是在我的收藏和已激活这个界面分别进行取消搜藏和禁用操作
        //时，listview一行被移出，这时候需要进行下面的操作，要不然界面会向下滚动。其他操作不需要这个操作，会导致bug现象。
        if (start == end) {
            selectionModel()->setCurrentIndex(parent, QItemSelectionModel::NoUpdate);
        }
    }

    QListView::rowsAboutToBeRemoved(parent, start, end);
}

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   键盘点击事件
 <Author>        null
 <Input>
    <param1>     event            Description:键盘点击事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::keyPressEvent(QKeyEvent *event)
{
    //检查当前是否有选中，恢复起始位
    checkIfHasSelection();
    //fix bug 50743,有关滚动条位置状态更新的函数被修改，导致上下键不能循环选中。
    //在顶部或底部的状态于此处更新，用作上下键或其他按键功能的判断依据
    getAtListViewPosition();

    if (event->key() == Qt::Key_End) {
        if (event->modifiers() == Qt::NoModifier) {
            setCurrentIndex(m_fontPreviewProxyModel->index(count() - 1, 0));
            setCurrentSelected(count() - 1);
        } else if (event->modifiers() == Qt::ShiftModifier) {
            clearSelection();
            for (int i = m_currentSelectedRow; i < count(); i++) {
                selectionModel()->select(m_fontPreviewProxyModel->index(i, 0), QItemSelectionModel::Select);
            }
            if (-1 == m_currentSelectedRow)
                setCurrentSelected(0);
        }
        scrollToBottom();
    } else if (event->key() == Qt::Key_Home) {
        if (event->modifiers() == Qt::NoModifier || selectedIndexes().count() == 0) {
            setCurrentIndex(m_fontPreviewProxyModel->index(0, 0));
            setCurrentSelected(0);
        } else if (event->modifiers() == Qt::ShiftModifier) {
            clearSelection();
            for (int i = m_currentSelectedRow; i >= 0; i--) {
                selectionModel()->select(m_fontPreviewProxyModel->index(i, 0), QItemSelectionModel::Select);
            }
        }
        scrollToTop();
    } else {
        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
            QModelIndexList list = selectedIndexes();
            qSort(list.begin(), list.end(), qGreater<QModelIndex>());
            //判断当前shift+上下键选中
            if (QApplication::keyboardModifiers() == Qt::ShiftModifier && list.count() > 0) {
                if (event->key() == Qt::Key_Up) {
                    keyPressEventFilter(list, true, false, true);
                    return;
                } else {
                    keyPressEventFilter(list, false, true, true);
                    return;
                }
            } else if (QApplication::keyboardModifiers() == Qt::CTRL && list.count() > 0) {
                return;
            }
            //判断当前选中item是否为首个或末尾，首个按上键且在可见时切换至末尾选中，末尾按下键且可见时切换至首个选中 UT000539
            if (event->key() == Qt::Key_Up) {
                keyPressEventFilter(list, true, false, false);
                return;
            } else if (event->key() == Qt::Key_Down) {
                keyPressEventFilter(list, false, true, false);
                return;
            }
        }
        //左键切换焦点至菜单项
        else if (event->key() == Qt::Key_Left) {
            emit m_signalManager->requestSetLeftSiderBarFocus();
            return;
        }
        //事件为空格按键，不做响应，直接退出，如果以后要实现空格功能，在这里添加即可
        else if (event->key() == Qt::Key_Space)
            return;
        QListView::keyPressEvent(event);
    }
}

/*************************************************************************
 <Function>      keyPressEventFilter
 <Description>   根据按键设置选中
 <Author>        null
 <Input>
    <param1>     list              Description:The selected QModelIndexList
    <param2>     isUp              Description:Key_Up pressed
    <param3>     isDown            Description:Key_Down pressed
    <param4>     isShiftModifier   Description:Key_Shift pressed
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::keyPressEventFilter(const QModelIndexList &list, bool isUp, bool isDown, bool isShiftModifier)
{
    //SP3--空白页面上下键选中判断
    if (list.count() < 1) {
        QModelIndex idx;
        if (isUp)
            idx = m_fontPreviewProxyModel->index(this->count() - 1, 0);
        else if (isDown) {
            idx = m_fontPreviewProxyModel->index(0, 0);
        }
        setCurrentIndex(idx);
        setCurrentSelected(idx.row());
        return;
    }
    //上键
    if (isUp) {
        //相反方向如果有选中，则清空并选中
        for (auto idx : list) {
            if (idx.row() > m_currentSelectedRow) {
                clearSelection();
                setCurrentIndex(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                scrollTo(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                return;
            }
        }
        //shift
        if (isShiftModifier) {
            if (list.last().row() > 0) {
                QModelIndex nextModelIndex = m_fontPreviewProxyModel->index(list.last().row() - 1, 0);
                selectionModel()->select(nextModelIndex, QItemSelectionModel::Select);
                scrollTo(nextModelIndex);
            }
        } else if (list.last().row() == 0 && list.count() == 1) {
            if (m_bListviewAtTop) {
                m_currentSelectedRow = count() - 1;
                setCurrentIndex(m_fontPreviewProxyModel->index(count() - 1, 0));
            } else {
                scrollToTop();
            }
        } else {
            setCurrentIndex(m_fontPreviewProxyModel->index(m_currentSelectedRow - 1, 0));
            m_currentSelectedRow --;
        }
    }
    //下键
    else if (isDown) {
        //相反方向如果有选中，则清空并选中
        for (auto idx : list) {
            if (idx.row() < m_currentSelectedRow) {
                clearSelection();
                setCurrentIndex(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                scrollTo(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                return;
            }
        }
        //shift
        if (isShiftModifier) {
            if (list.first().row() < this->count()) {
                QModelIndex nextModelIndex = m_fontPreviewProxyModel->index(list.first().row() + 1, 0);
                selectionModel()->select(nextModelIndex, QItemSelectionModel::Select);
                scrollTo(nextModelIndex);
            }
        } else if (list.last().row() == this->count() - 1 && list.count() == 1) {
            if (m_bListviewAtButtom) {
                QModelIndex modelIndex = m_fontPreviewProxyModel->index(0, 0);
                m_currentSelectedRow = 0;
                setCurrentIndex(modelIndex);
            } else {
                scrollToBottom();
            }
        } else {
            QModelIndex modelIndex = m_fontPreviewProxyModel->index(m_currentSelectedRow + 1, 0);
            setCurrentIndex(modelIndex);
            m_currentSelectedRow ++;
        }
    }
}

/*************************************************************************
 <Function>      eventFilter
 <Description>   事件过滤器,用于处理焦点移动事件
 <Author>        null
 <Input>
    <param1>     obj              Description:UNUSED
    <param2>     event            Description:触发的事件
 <Return>        null            Description:null

 <Function>      enableFont
 <Description>   启用字体 <Note>          null
*************************************************************************/
bool DFontPreviewListView::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

    if (event->type() == QEvent::FocusOut) {
        m_IsTabFocus = false;
    }

    if (event->type() == QEvent::FocusIn) {
        QFocusEvent *fxEvent = dynamic_cast<QFocusEvent *>(event);
        if (fxEvent->reason() == Qt::TabFocusReason && m_isGetFocusFromSlider == true) {
            emit m_signalManager->requestSetTabFocusToAddBtn();
        }
    }

    return false;
}

/*************************************************************************
 <Function>      enableFont
 <Description>   加入启用字体列表
 <Author>        null
 <Input>
    <param1>     filePath            Description:启用字体路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::enableFont(const QString &filePath)
{
    if (!m_enableFontList.contains(filePath))
        m_enableFontList << filePath;
}

/*************************************************************************
 <Function>      disableFont
 <Description>   加入禁用字体列表
 <Author>        null
 <Input>
    <param1>     filePath            Description:禁用字体路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::disableFont(const QString &filePath)
{
    if (!m_disableFontList.contains(filePath))
        m_disableFontList << filePath;
}

/*************************************************************************
 <Function>      enableFonts
 <Description>   批量启用字体
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::enableFonts()
{
    if (m_enableFontList.isEmpty())
        return;
    qDebug() << __FUNCTION__ << m_enableFontList.size();

    QString fontConfigPath = DFMXmlWrapper::m_fontConfigFilePath;
    bool isCreateSuccess = DFMXmlWrapper::createFontConfigFile(fontConfigPath);

    if (!isCreateSuccess) {
        return;
    }

    DFMXmlWrapper::deleteNodeWithTextList(fontConfigPath, "pattern", m_enableFontList);
    m_enableFontList.clear();
}

/*************************************************************************
 <Function>      enableFonts
 <Description>   批量禁用字体
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::disableFonts()
{
    if (m_disableFontList.isEmpty())
        return;
    qDebug() << __FUNCTION__ << m_disableFontList.size();

    QString fontConfigPath = DFMXmlWrapper::m_fontConfigFilePath;
    bool isCreateSuccess = DFMXmlWrapper::createFontConfigFile(fontConfigPath);

    if (!isCreateSuccess) {
        return;
    }

    DFMXmlWrapper::addPatternNodesWithTextList(fontConfigPath, "rejectfont", m_disableFontList);
    m_disableFontList.clear();
}

/*************************************************************************
 <Function>      updateShiftSelect
 <Description>   更新shift选中的字体.
 <Author>        null
 <Input>
    <param1>     modelIndex            Description:需要更新的index
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::updateShiftSelect(const QModelIndex &modelIndex)
{
    if (!modelIndex.isValid())
        return;

    int begin = m_currentSelectedRow;
    int end = m_currentSelectedRow;

    if (-1 != m_currentSelectedRow) {
        if (m_currentSelectedRow < modelIndex.row()) {
            end = modelIndex.row();
        } else if (m_currentSelectedRow > modelIndex.row()) {
            begin = modelIndex.row();
        }
    } else {
        begin = 0;
        end = modelIndex.row();
    }

    if (begin < 0)
        return;

    selectionModel()->clear();
    for (int i = begin; i <= end; i++) {
        selectionModel()->select(m_fontPreviewProxyModel->index(i, 0), QItemSelectionModel::Select);
    }
}

/*************************************************************************
 <Function>      onRightMenuShortCutActivated
 <Description>   Alt+M右键菜单--弹出
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
/*SP3--Alt+M右键菜单--弹出*/
void DFontPreviewListView::onRightMenuShortCutActivated()
{
    if (selectedIndexes().count() == 0) {
        return;
    }

    m_isMousePressNow = false;
    syncRecoveryTabStatus();
    setFontViewHasFocus(hasFocus());

    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    QModelIndexList indexes = selectedIndexes();
    //排序选中项
    qSort(indexes.begin(), indexes.end());
    //菜单弹出位置
    QPoint showMenuPosition;
    //记录鼠标位置下的QModelIndex
    QModelIndex n_currentIdx = indexAt(mapFromGlobal(QCursor::pos()));
    qDebug() << n_currentIdx.row();
    if (selectedIndexes().count() == 1) {
        QRect curRect = visualRect(selectedIndexes().first());
        //判断选中是否可见
        if (!viewport()->visibleRegion().contains(curRect.center())) {
            scrollTo(selectedIndexes().first());
            curRect = visualRect(selectedIndexes().first());
        }
        showMenuPosition = curRect.center();
        showMenuPosition = QPoint(showMenuPosition.x() + mw->pos().x() + POSITION_PARAM_X, showMenuPosition.y() + mw->pos().y() + POSITION_PARAM_Y);
    } else if (selectedIndexes().count() > 1) {
        bool n_needScroll = true;
        for (auto idx : indexes) {
            QRect curRect = visualRect(idx);
            //判断选中是否可见,在第一个可见位置弹出
            if (viewport()->visibleRegion().contains(curRect.center())) {
                setCurrentSelected(idx.row());
                showMenuPosition = QPoint((curRect.center()).x() + mw->pos().x() + POSITION_PARAM_X, (curRect.center()).y() + mw->pos().y() + POSITION_PARAM_Y);
                n_needScroll = false;
                break;
            }
        }
        if (n_needScroll) {
            scrollTo(indexes.first());
            setCurrentSelected(indexes.first().row());
            QPoint n_firstPosCenter = (visualRect(indexes.first())).center();
            showMenuPosition = QPoint(n_firstPosCenter.x() + mw->pos().x() + POSITION_PARAM_X, n_firstPosCenter.y() + mw->pos().y() + POSITION_PARAM_Y);
        }
    }
    //恢复normal状态
    FontData itemData = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(n_currentIdx));
    itemData.getHoverState();
    if (itemData.getHoverState() != IconNormal) {
        itemData.setHoverState(IconNormal);
        m_fontPreviewProxyModel->setData(n_currentIdx, QVariant::fromValue(itemData), Qt::DisplayRole);
    }
    if (!m_rightMenu->isVisible()) {
        m_rightMenu->exec(showMenuPosition);
        return;
    }
}

/*************************************************************************
 <Function>      checkHoverState
 <Description>   检查鼠标是否处于hover状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::checkHoverState()
{
    //记录鼠标位置下的QModelIndex
    QModelIndex n_currentIdx = indexAt(mapFromGlobal(QCursor::pos()));
    QRect collectIconRect = getCollectionIconRect(visualRect(n_currentIdx));
    FontData itemData = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(n_currentIdx));
    if (collectIconRect.contains(mapFromGlobal(QCursor::pos()))) {
        if (itemData.getHoverState() != IconHover) {
            itemData.setHoverState(IconHover);
            m_fontPreviewProxyModel->setData(n_currentIdx, QVariant::fromValue(itemData), Qt::DisplayRole);
        }
        m_hoverModelIndex = n_currentIdx;
    }
}

/*************************************************************************
 <Function>      onEnableBtnClicked
 <Description>   listview中启用禁用响应函数
 <Author>        null
 <Input>
    <param1>     itemIndexes            Description:选中项的索引
    <param2>     systemCnt              Description:是否为系统字体
    <param3>     curCnt                 Description:是否为系统当前使用的字体
    <param4>     setValue               Description:需要设置的值
    <param5>     isFromActiveFont       Description:是否在已激活界面触发
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onEnableBtnClicked(QModelIndexList &itemIndexes, int systemCnt,
                                              int curCnt, bool setValue, bool isFromActiveFont)
{
    PerformanceMonitor::activeFontStart();

    if (itemIndexes.isEmpty())
        return;

    qDebug() << __FUNCTION__ << " before " << currModelIndex().row() << currentIndex().row();
    bool needShowTips = false;
    int count = 0;
    QMutexLocker locker(&m_mutex);
    QString fontName;

    if (isFromActiveFont)
        qSort(itemIndexes.begin(), itemIndexes.end(), qGreater<QModelIndex>());

    //记录禁用前选中位置
    getAtListViewPosition();
    m_selectAfterDel = itemIndexes.last().row();

    bool isConfirm = false; // 是否确认选择
    bool isAapplyToAll = false; // 选择应用于全部ttc字体集
    QSet<QString> ttcSet; //记录ttc字体文件
    QSet<QString> confirmSet; //记录确认选择的字体
    for (QModelIndex &index : itemIndexes) {
        FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(index));
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);

        // ttc字体集询问是否应用全部
        if (itemData.fontInfo.filePath.endsWith(".ttc", Qt::CaseInsensitive)) {
            if (!ttcSet.contains(itemData.fontInfo.filePath)) { //不包含
                if (!isAapplyToAll) {
                    Q_EMIT signalHandleDisableTTC(itemData.fontInfo.filePath, setValue, isConfirm, isAapplyToAll);
                }
                ttcSet.insert(itemData.fontInfo.filePath);
                if (!isConfirm) { // 取消对该ttc字体禁用/启用操作
                    continue;
                } else {
                    confirmSet.insert(itemData.fontInfo.filePath);
                }
            }
        } else {
            confirmSet.insert(itemData.fontInfo.filePath);
        }
    }

    DFontPreviewProxyModel *filterModel = getFontPreviewProxyModel();
    int oldFilterGroup = filterModel->getFilterGroup(); // 保存当前字体组
    filterModel->setFilterGroup(DSplitListWidget::UserFont); // 只有用户字体才能被禁用
    for (int i = 0; i < filterModel->rowCount(); ++i) {
        QModelIndex index = filterModel->index(i, 0);
        FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(index));
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);
        if (confirmSet.contains(itemData.fontInfo.filePath)) {
            int idx = m_dataThread->m_fontModelList.indexOf(itemData);
            if (setValue) {
                enableFont(itemData.fontInfo.filePath);
            } else {
                if (systemCnt > 0 || curCnt > 0) {
                    needShowTips = true;
                }

                if (itemIndexes.size() == 1) {
                    fontName = itemData.fontData.strFontName;
                }
                disableFont(itemData.fontInfo.filePath);
                count++;
            }
            itemData.fontData.setEnabled(setValue);
            fdata.setEnabled(setValue);
            // 更新状态
            m_dataThread->updateItemStatus(idx, itemData);
            DFMDBManager::instance()->updateFontInfo(itemData, "isEnabled");
            m_fontPreviewProxyModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
        }
    }
    filterModel->setFilterGroup(oldFilterGroup); // 恢复原字体组


    DFMDBManager::instance()->commitUpdateFontInfo();

    if (setValue) {
        enableFonts();
        qDebug() << __FUNCTION__ << " after " << currModelIndex().row() << currentIndex().row();
        PerformanceMonitor::activeFontFinish(false, itemIndexes.count());
        return;
    } else {
        disableFonts();
    }

    if (isFromActiveFont == true) {
        //设置移除后的选中
        selectItemAfterRemoved(m_bListviewAtButtom, m_bListviewAtTop, false, needShowTips);
    }

    QString message;
    if (needShowTips) {
        //不可禁用字体
        if (curCnt > 0 && systemCnt == 0) {
            message = QApplication::translate("MessageManager", "%1 is in use, so you cannot disable it").arg(m_curFontData.strFontName);
        } else if (curCnt > 0 && systemCnt > 0) {
            message = QApplication::translate("MessageManager", "You cannot disable system fonts and the fonts in use");
        } else {
            message = QApplication::translate("MessageManager", "You cannot disable system fonts");
        }
    } else {
        if (count == 1) {
            message = QString("%1 %2").arg(fontName).arg(QApplication::translate("MessageManager", "deactivated"));
        } else if (count > 1) {
            message = QApplication::translate("MessageManager", "The fonts have been deactivated");
        }
    }
    //禁用字体大于零
    if (count > 0)
        DMessageManager::instance()->sendMessage(this->m_parentWidget, QIcon("://ok.svg"), message);

    qDebug() << __FUNCTION__ << " after " << currModelIndex().row() << currentIndex().row();
    Q_EMIT rowCountChanged();

    PerformanceMonitor::activeFontFinish(true, itemIndexes.count());
}

/*************************************************************************
 <Function>      onCollectBtnClicked
 <Description>   listview收藏界面点击后触发函数
 <Author>        null
 <Input>
    <param1>     index               Description:选中项的indexlist
    <param2>     setValue            Description:需要设置的值
    <param3>     isFromCollectFont   Description:是否来自于我的收藏界面
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onCollectBtnClicked(QModelIndexList &indexList, bool setValue, bool isFromCollectFont)
{
    PerformanceMonitor::favoriteFontStart();

    if (indexList.isEmpty())
        return;

    qDebug() << __FUNCTION__ << " before " << currModelIndex().row() << currentIndex().row();

    QMutexLocker locker(&m_mutex);
    if (isFromCollectFont)
        qSort(indexList.begin(), indexList.end(), qGreater<QModelIndex>());
    m_selectAfterDel = indexList.last().row();

    getAtListViewPosition();

    for (QModelIndex &index : indexList) {
        FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(index));
        if (fdata.strFontName.isEmpty() && fdata.fontState == 0)
            continue;

        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);

        itemData.fontData.setCollected(setValue);
        fdata.setCollected(setValue);
        DFMDBManager::instance()->updateFontInfo(itemData, "isCollected");
        int idx = m_dataThread->m_fontModelList.indexOf(itemData);
        //更新状态
        m_dataThread->updateItemStatus(idx, itemData);
        m_fontPreviewProxyModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    }
    if (isFromCollectFont == true) {
        //设置收藏页面移除后的选中
        selectItemAfterRemoved(m_bListviewAtButtom, m_bListviewAtTop, true, false);
    }

    DFMDBManager::instance()->commitUpdateFontInfo();

    Q_EMIT rowCountChanged();
    PerformanceMonitor::favoriteFontFinish(setValue, indexList.count());

    qDebug() << __FUNCTION__ << " after " << currModelIndex().row() << currentIndex().row();
}

/*************************************************************************
 <Function>      onListViewShowContextMenu
 <Description>   显示右键菜单
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onListViewShowContextMenu()
{
    QAction *action = m_rightMenu->exec(QCursor::pos());
    m_isMousePressNow = false;
    qDebug() << __FUNCTION__ << action;
}

/*************************************************************************
 <Function>      setRightContextMenu
 <Description>   设置右键菜单
 <Author>        null
 <Input>
    <param1>     rightMenu       Description:需要设置的右键菜单
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::setRightContextMenu(QMenu *rightMenu)
{
    m_rightMenu = rightMenu;
}

/*************************************************************************
 <Function>      currModelIndex
 <Description>   获取当前项的Index
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        QModelIndex     Description:当前项的Index
 <Note>          null
*************************************************************************/
QModelIndex DFontPreviewListView::currModelIndex()
{
    int min = -1;
    QModelIndex minIndex;
    for (QModelIndex &index : selectedIndexes()) {
        if (min < 0 || min > index.row()) {
            min = index.row();
            minIndex = index;
        }
    }

    if (minIndex.isValid())
        m_currModelIndex = minIndex;

    return m_currModelIndex;
}

/*************************************************************************
 <Function>      currModelData
 <Description>   返回当前项的itemdata
 <Author>        null
 <Input>
    <param1>     null                            Description:null
 <Return>        DFontPreviewItemData            Description:当前项的itemdata
 <Note>          null
*************************************************************************/
DFontPreviewItemData DFontPreviewListView::currModelData()
{
    QVariant varModel = m_fontPreviewProxyModel->data(currModelIndex(), Qt::DisplayRole);
    FontData fdata = varModel.value<FontData>();
    DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);

    return itemData;
}

/*************************************************************************
 <Function>      clearPressState
 <Description>   清空收藏图标的press状态
 <Author>        null
 <Input>
    <param1>     type            Description:用于清空收藏图标press状态的类型
 <Return>        currentRow      Description:当前收藏图标在press状态的行
 <Note>          null
*************************************************************************/
void DFontPreviewListView::clearPressState(ClearType type, int currentRow)
{
    QModelIndex modelIndexBefore = m_fontPreviewProxyModel->index(m_previousPressPos, 0);
    FontData itemDataBefore =
        qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndexBefore));

    FontData pressData =
        qvariant_cast<FontData>(m_fontPreviewProxyModel->data(m_pressModelIndex));

    switch (type) {
    case ClearType::MoveClear:
        if (!m_pressModelIndex.isValid())
            return;
        if (pressData.getHoverState() == IconNormal)
            return;
        pressData.setHoverState(IconNormal);
        m_fontPreviewProxyModel->setData(m_pressModelIndex, QVariant::fromValue(pressData), Qt::DisplayRole);
        m_pressModelIndex = QModelIndex();
        break;
    case DFontPreviewListView::ClearType::PreviousClear:
        if (-1 != m_previousPressPos && m_previousPressPos != currentRow) {
            itemDataBefore.setHoverState(IconNormal);
            m_fontPreviewProxyModel->setData(modelIndexBefore, QVariant::fromValue(itemDataBefore), Qt::DisplayRole);
        }
        break;
    default:
        break;
    }
}

/*************************************************************************
 <Function>      clearHoverState
 <Description>   清空hover状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::clearHoverState()
{
    if (!m_hoverModelIndex.isValid())
        return;

    FontData itemData =
        qvariant_cast<FontData>(m_fontPreviewProxyModel->data(m_hoverModelIndex));
    if (itemData.getHoverState() != IconNormal) {
        qDebug() << " restore hover item " << itemData.strFontName;
        itemData.setHoverState(IconNormal);
        m_fontPreviewProxyModel->setData(m_hoverModelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
    }
    m_hoverModelIndex = QModelIndex();
}

/*************************************************************************
 <Function>      updateChangedFile
 <Description>   文件改动时触发的函数
 <Author>        null
 <Input>
    <param1>     path            Description:改动文件的路径

 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::updateChangedFile(const QStringList &pathlist)
{
    qDebug() << __FUNCTION__ << pathlist << " begin ";
    QMutexLocker locker(&m_mutex);

    foreach (auto it, pathlist) {
        changeFontFile(it);
    }


    Q_EMIT rowCountChanged();
    qDebug() << __FUNCTION__ << pathlist << " end ";
}

/*************************************************************************
 <Function>      updateChangedFile
 <Description>   目录改动时触发的函数
 <Author>        null
 <Input>
    <param1>     null

 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::updateChangedDir()
{
    //no different between "share" or "fonts" dir
    //qDebug() << __FUNCTION__ << path << m_dataThread->m_fontModelList.size() <<" begin ";
    QList<DFontPreviewItemData>::iterator iter = m_dataThread->m_fontModelList.begin();
    bool removed = false;
    while (iter != m_dataThread->m_fontModelList.end()) {
        DFontPreviewItemData itemData(*iter);
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if (!filePath.isEmpty() && !filePathInfo.exists()) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont((itemData).fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            m_dataThread->removePathWatcher(filePathInfo.filePath());
            iter = m_dataThread->m_fontModelList.erase(iter);
            if (!removed)
                removed = true;
            Q_EMIT itemRemoved(itemData);
        } else {
            ++iter;
        }
    }

    if (!removed)
        return;

    DFMDBManager::instance()->commitDeleteFontInfo();
    enableFonts();

//    Q_EMIT requestUpdateModel(false);
    Q_EMIT rowCountChanged();
    //qDebug() << __FUNCTION__ << "update end";
}

/*************************************************************************
 <Function>      deleteFontFiles
 <Description>   删除字体
 <Author>        null
 <Input>
    <param1>     files            Description:需要删除的字体
    <param2>     force            Description:是否为强制删除
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::deleteFontFiles(QStringList &files, bool force)
{
    if (force) {
        for (const QString &path : files) {
            bool del = QFile::remove(path);
            qDebug() << __FUNCTION__ << " force delete file " << path << del;
        }
    }

    deleteCurFonts(files, force);
}

/*************************************************************************
 <Function>      deleteCurFonts
 <Description>   从字体库删除字体
 <Author>
 <Input>
    <param1>     files           Description:待删除字体列表
    <param2>     force           Description:删除类型
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::deleteCurFonts(QStringList &files, bool force)
{
    qDebug() << __FUNCTION__ << " before delete " << m_dataThread->getFontModelList().size() << m_fontPreviewProxyModel->rowCount();

    int deleteCount = files.count();

    if (!force)
        updateSpinner(DFontSpinnerWidget::Delete, false);

    QList<DFontPreviewItemData>::iterator iter = m_dataThread->m_fontModelList.begin();
    while (iter != m_dataThread->m_fontModelList.end()) {
        DFontPreviewItemData itemData(*iter);
        QString filePath = itemData.fontInfo.filePath;
        //如果字体文件已经不存在，则从t_manager表中删除
        if (files.contains(filePath)) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            QFontDatabase::removeApplicationFont(itemData.appFontId);
            m_dataThread->removePathWatcher(filePath);
            iter = m_dataThread->m_fontModelList.erase(iter);
            m_recoverSelectStateUserfontMultiMap.remove(filePath);
        } else {
            ++iter;
        }
    }

    DFMDBManager::instance()->commitDeleteFontInfo();

    enableFonts();

    Q_EMIT requestUpdateModel(deleteCount, !force);
    m_dataThread->onAutoDirWatchers();
}

/*************************************************************************
 <Function>      changeFontFile
 <Description>   文件改动时触发的函数,对相应文件进行处理
 <Author>        null
 <Input>
    <param1>     null            Description:null
    <param2>     null            Description:null
    <param3>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::changeFontFile(const QString &path, bool force)
{
    QFileInfo fi(path);
    bool isDir = fi.isDir();
    if (force) {
        bool del = QFile::remove(path);
        qDebug() << __FUNCTION__ << " force delete file " << path << del;
    }

    qDebug() << __FUNCTION__ << path;
    QList<DFontPreviewItemData>::iterator iter = m_dataThread->m_fontModelList.begin();
    while (iter != m_dataThread->m_fontModelList.end()) {
        DFontPreviewItemData itemData(*iter);
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if ((!isDir && (filePath == path) && (!filePathInfo.exists())) || (isDir && filePath.startsWith(path) && !filePathInfo.exists())) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            m_dataThread->removePathWatcher(filePath);
            iter = m_dataThread->m_fontModelList.erase(iter);
            Q_EMIT itemRemoved(itemData);
            if (!isDir && !itemData.fontInfo.filePath.endsWith(".ttc", Qt::CaseInsensitive)) {
                break;
            }
        } else {
            ++iter;
        }
    }
    DFMDBManager::instance()->commitDeleteFontInfo();
    enableFonts();
    //    DFontInfoManager::instance()->removeFontInfo();

    if (isDir) {
        if (!QFileInfo(QDir::homePath() + "/.local/share/fonts").exists()) {
            m_dataThread->removePathWatcher(QDir::homePath() + "/.local/share/fonts");
        } else if (!QFileInfo(QDir::homePath() + "/.local/share/").exists()) {
            m_dataThread->removePathWatcher(QDir::homePath() + "/.local/share/");
        }
    }
}

/*************************************************************************
 <Function>      selectedFonts
 <Description>   选中字体,并通过传入参数获得各种字体的数目等信息.
 <Author>        null
 <Input>
    <param1> deleteCnt           Description: count of deletable fonts (minus system fonts and current font)
    <param1> systemCnt           Description: count of system font
    <param1> curFontCnt          Description: count of current font (0/1)
    <param1> disableCnt          Description: count of can be disabled/enabled fonts
    <param1> delFontList         Description: delete font path list (for delete fonts)
    <param1> allIndexList        Description: all font index list (includes system fonts and current font) (for collect fonts)
    <param1> disableIndexList    Description: can be disabled/enabled font index list (for enable/disable fonts)
    <param1> allMinusSysFontList Description: all font path list exclude system fonts (for export fonts)
    <param1> curData             Description: current index data (required for disable/enable/collect fonts)
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::selectedFonts(const DFontPreviewItemData &curData,
                                         int *deleteCnt, int *disableSysCnt, int *systemCnt, qint8 *curFontCnt,
                                         int *disableCnt, QStringList *delFontList, QModelIndexList *allIndexList,
                                         QModelIndexList *disableIndexList, QStringList *allMinusSysFontList)
{
    if (deleteCnt != nullptr)
        *deleteCnt = 0;
    if (disableSysCnt != nullptr)
        *disableSysCnt = 0;
    if (systemCnt != nullptr)
        *systemCnt = 0;
    if (curFontCnt != nullptr)
        *curFontCnt = 0;
    if (disableCnt != nullptr)
        *disableCnt = 0;
    if (delFontList != nullptr)
        delFontList->clear();
    if (allIndexList != nullptr)
        allIndexList->clear();
    if (disableIndexList != nullptr)
        disableIndexList->clear();
    if (allMinusSysFontList != nullptr)
        allMinusSysFontList->clear();
    m_recoverSelectStateSysfontList.clear();
    m_recoverSelectStateUserfontMultiMap.clear();

    QModelIndexList list = selectedIndexes();

    bool curEnabled = false;
    bool curCollected = false;
    bool calDisable = ((disableIndexList != nullptr) || (disableCnt != nullptr));
    bool calCollect = (allIndexList != nullptr);

    if (calDisable) {
        curEnabled = curData.fontData.isEnabled();
    }
    if (calCollect) {
        curCollected = curData.fontData.isCollected();
    }

    for (QModelIndex &index : list) {
        QVariant varModel = m_fontPreviewProxyModel->data(index, Qt::DisplayRole);
        FontData fdata = varModel.value<FontData>();
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);
        if (itemData.fontInfo.filePath.isEmpty())
            continue;

        if (itemData.fontInfo.isSystemFont) {
            //处理启用禁用
            if (calDisable) {
                if (!curEnabled) { //禁用状态
                    //系统字体可以启用
                    if (curEnabled == itemData.fontData.isEnabled()) {
                        if (disableCnt)
                            *disableCnt += 1;
                        if (disableIndexList)
                            *disableIndexList << index;
                    }
                    //启用状态, 系统字体不可禁用
                } else {
                    if (disableSysCnt != nullptr)
                        *disableSysCnt += 1;
                }
            }
            //系统字体不可删除,记录idx便于恢复选中状态
            m_recoverSelectStateSysfontList.append(index.row());
            if (systemCnt != nullptr)
                *systemCnt += 1;
            if (calCollect && (curCollected == itemData.fontData.isCollected()))
                *allIndexList << index;
        } else if ((itemData.fontData == m_curFontData) ||
                   (itemData.fontInfo.filePath == m_dataThread->getFontData(m_curFontData).fontInfo.filePath)) {
            // 选中字体与当前使用的字体为同一字体文件
            // ttc字体集，即使不是同一个字体，也是同一个字体文件
            qDebug() << __FUNCTION__ << " current font " << itemData.fontData.strFontName;
            if (curFontCnt)
                *curFontCnt += 1;
            appendFilePath(allMinusSysFontList, itemData.fontInfo.filePath);
            if ((calCollect) && (curCollected == itemData.fontData.isCollected()))
                *allIndexList << index;
            //不可删除字体包括当前使用中用户字体
            setUserFontInUseSelected(true);
        } else {
            if (deleteCnt)
                *deleteCnt += 1;

            appendFilePath(delFontList, itemData.fontInfo.filePath);
            m_recoverSelectStateUserfontMultiMap.insert(itemData.fontInfo.filePath, itemData);
            appendFilePath(allMinusSysFontList, itemData.fontInfo.filePath);

            if (calDisable && (curEnabled == itemData.fontData.isEnabled())) {
                if (disableCnt)
                    *disableCnt += 1;
                if (disableIndexList)
                    *disableIndexList << index;
            }

            if ((calCollect) && (curCollected == itemData.fontData.isCollected())) {
                *allIndexList << index;
            }
        }
    }

    if (delFontList && deleteCnt)
        qDebug() << __FUNCTION__ << delFontList->size() << *deleteCnt;
}

/*************************************************************************
 <Function>      scrollWithTheSelected
 <Description>   切换至listview，已有选中且不可见，则滚动到第一并记录位置
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::scrollWithTheSelected()
{
    if (0 == selectedIndexes().count()) {
        QModelIndex modelIndex = getFontPreviewProxyModel()->index(0, 0);
        if (modelIndex.isValid()) {
            setCurrentIndex(modelIndex);
            setCurrentSelected(0);
        }
        return;
    }
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    qSort(indexes.begin(), indexes.end(), qGreater<QModelIndex>());
    bool needScroll = true;
    for (auto &idx : indexes) {
        QRect rect = visualRect(idx);
        if (this->viewport()->visibleRegion().contains(rect.center())) {
            //如果在刷新之前没有选中位置，则更新为首个选中项。
            m_currentSelectedRow = (m_currentSelectedRow = -1) ? indexes.last().row() : m_currentSelectedRow;
            needScroll = false;
            break;
        }
    }
    if (needScroll) {
        scrollTo(indexes.last());
        setCurrentSelected(indexes.last().row());
        indexes.clear();
    }
}

/*************************************************************************
 <Function>      updateSelection
 <Description>   更新选中状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::updateSelection()
{
    //如果删除的字体为当前选中的字体，则更新选中
    if (m_currentSelectedRow == m_selectAfterDel) {
        getAtListViewPosition();
        //执行删除后的选中逻辑
        selectItemAfterRemoved(m_bListviewAtButtom, m_bListviewAtTop, false, false);
    }
}
