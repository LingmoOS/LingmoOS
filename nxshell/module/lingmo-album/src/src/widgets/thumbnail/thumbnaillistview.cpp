// SPDX-FileCopyrightText: 2020 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnaillistview.h"
//#include "application.h"

#include <DApplication>

#include <QDebug>
#include <QDrag>
#include <QFileInfo>
#include <QImageReader>
#include <QMimeData>
#include <QScrollBar>
#include <QMutex>
#include <QScroller>
#include <QPropertyAnimation>
#include <QDesktopServices>
#include <QProcess>
#include <QElapsedTimer>

//#include "controller/signalmanager.h"
//#include "controller/wallpapersetter.h"
//#include "widgets/dialogs/imgdeletedialog.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"
#include "globalstatus.h"
#include "albumControl.h"
//#include "imageengine/imageengineapi.h"
//#include "imageengine/imageenginethread.h"
//#include "ac-desktop-define.h"
//#include "allpicview/allpicview.h"
#include "imageengine/imagedataservice.h"
//#include "batchoperatewidget.h"

namespace {
const int ITEM_SPACING = 4;
const int BASE_HEIGHT = 100;
const int MINIMUN_WIDTH = 442;
const int STATUSBAR_HEIGHT = 27;
const int RIGHT_MARGIN = 10;

// const QString IMAGE_DEFAULTTYPE = "All pics";
const QString IMAGE_DEFAULTTYPE = "All Photos";
const QString SHORTCUTVIEW_GROUP = "SHORTCUTVIEW";

using namespace Libutils::common;

QString ss(const QString &text)
{
    QString str/* = dApp->setter->value(SHORTCUTVIEW_GROUP, text).toString()*/;
    str.replace(" ", "");

    return str;
}
}  // namespace

ThumbnailListView::ThumbnailListView(ThumbnailDelegate::DelegateType type, int UID, const QString &imgtype, QWidget *parent)
    :  DListView(parent), m_delegatetype(type), m_allfileslist(), updateEnableSelectionByMouseTimer(nullptr)
{
    m_model = new QStandardItemModel(this);
    m_imageType = imgtype;
    m_currentUID = UID;
    m_iBaseHeight = BASE_HEIGHT;
    m_albumMenu = nullptr;
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setSpacing(ITEM_SPACING);
    setDragEnabled(false);
    //按照像素滚动，步进20
    setVerticalScrollMode(QListView::ScrollPerPixel);
    verticalScrollBar()->setSingleStep(20);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setMinimumWidth(MINIMUN_WIDTH);
    m_delegate = new ThumbnailDelegate(type, this); //绑在this上面，用于自动销毁
    m_delegate->m_imageTypeStr = m_imageType;
    setItemDelegate(m_delegate);
    setModel(m_model);
    m_pMenu = new DMenu();
    setViewportMargins(0, 0, 0, 0);
    initMenuAction();
    initConnections();
    installEventFilter(this);
    verticalScrollBar()->installEventFilter(this);

    updateEnableSelectionByMouseTimer = new QTimer(this);
    updateEnableSelectionByMouseTimer->setInterval(500);
    connect(updateEnableSelectionByMouseTimer, &QTimer::timeout, [ = ]() {
        if (touchStatus == 0) { //时间到了还在等待模式,则进入框选模式
            touchStatus = 2;
        }
        updateEnableSelectionByMouseTimer->stop();
    });

    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setInterval(100);
    m_scrollTimer->setSingleShot(true);
    connect(m_scrollTimer, &QTimer::timeout, this, &ThumbnailListView::onScrollTimerOut);

    m_importTimer = new QTimer(this);
    connect(m_importTimer, &QTimer::timeout, this, [this]() {
        this->update();
        m_importActiveCount--;
        if (m_importActiveCount <= 0) {
            m_importTimer->stop();
        }
    });

    //if (m_imageType == COMMON_STR_TRASH) {
    //    connect(dApp->signalM, &SignalManager::sigRestoreFailed, this, &ThumbnailListView::selectPhotos);
    //}
    connect(ImageDataService::instance(), &ImageDataService::sigeUpdateListview, this, &ThumbnailListView::onUpdateListview, Qt::ConnectionType::QueuedConnection);
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &ThumbnailListView::onScrollbarValueChanged);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ThumbnailListView::onSelectionChanged);

    connect(GlobalStatus::instance(), &GlobalStatus::sigKeyPressFromQml, this, &ThumbnailListView::onKeyPressFromQml);

    viewport()->setAttribute(Qt::WA_Hover);
}

ThumbnailListView::~ThumbnailListView()
{
    if (m_loadTimer) {
        if (m_loadTimer->isActive())
            m_loadTimer->stop();
        m_loadTimer->deleteLater();
    }
}

static QString myMimeType()
{
    return QStringLiteral("TestListView/text-icon-icon_hover");
}

void ThumbnailListView::mousePressEvent(QMouseEvent *event)
{
    QTime curTime = QTime::currentTime();

    //点击时将焦点设置到当前
    setFocus();
    //复位激活click
    activeClick = true;
    // 当事件source为MouseEventSynthesizedByQt，认为此事件为TouchBegin转换而来
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        lastTouchBeginPos = event->pos();

        // 清空触屏滚动操作，因为在鼠标按下时还不知道即将进行的是触屏滚动还是文件框选
        if (QScroller::hasScroller(this)) {
            // 不可使用 ungrab，会导致应用崩溃，或许是Qt的bug
            QScroller::scroller(this)->deleteLater();
        }

        updateEnableSelectionByMouseTimer->start();
    }

    if ((QApplication::keyboardModifiers() == Qt::ShiftModifier && event->button() == Qt::LeftButton)) {
        // 最近删除界面单指循环选中、取消选中
        if (selectionModel()->selectedIndexes().contains(this->indexAt(event->pos()))
                && selectionModel()->selectedIndexes().size() == 1 && m_imageType == COMMON_STR_TRASH) {
            if (event->button() != Qt::MouseButton::RightButton) { // 优化最近删除界面,长按时不清除选中
                clearSelection();
            }
        }
#ifdef tablet_PC
        else if (m_isSelectAllBtn && (m_imageType == COMMON_STR_VIEW_TIMELINE || m_imageType == COMMON_STR_RECENT_IMPORTED)) {
            QModelIndexList list = selectionModel()->selectedIndexes();
            DListView::mousePressEvent(event);
            for (auto item : list) {
                selectionModel()->select(item, QItemSelectionModel::Select);
            }
            if (list.contains(this->indexAt(event->pos())) && event->button() != Qt::RightButton) {
                selectionModel()->select(this->indexAt(event->pos()), QItemSelectionModel::Deselect);
            } else {
                selectionModel()->select(this->indexAt(event->pos()), QItemSelectionModel::Select);
            }
        }
#endif
    }
    QModelIndex index = this->indexAt(event->pos());
    DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();
    if ((m_imageType != COMMON_STR_VIEW_TIMELINE)
            && (m_delegatetype != ThumbnailDelegate::AllPicViewType)
            && (m_imageType != COMMON_STR_TRASH)
            && (m_imageType != ALBUM_PATHTYPE_BY_PHONE)) {
        if (info.itemType == ItemTypeImportTimeLineTitle) {
            //已导入时间线标题不可拖动
            setDragEnabled(false);
        } else if (dragDropMode() != NoDragDrop) {
            setDragDropMode(DragDrop);
        }
    } else {
        setDragEnabled(false);
    }
    m_pressed = event->pos();

    if (event->button() == Qt::LeftButton) {
        if (m_lastPressTime.msecsTo(curTime) < 200)
            emit doubleClicked(index);
        m_lastPressTime = curTime;
    }

    DListView::mousePressEvent(event);
}

void ThumbnailListView::mouseMoveEvent(QMouseEvent *event)
{
    QRectF rect(QPointF((lastTouchBeginPos.x() - 30), (lastTouchBeginPos.y() - 30)),
                QPointF((lastTouchBeginPos.x() + 30), (lastTouchBeginPos.y() + 30)));
    if (rect.contains(event->pos())) {
        return;
    }

    if (touchStatus == 0) {
        touchStatus = 1;
        activeClick = false;
    }

    if (touchStatus == 1) {
        if (event->source() == Qt::MouseEventSynthesizedByQt) {
        }
    }

    emit sigMouseMove();
    DListView::mouseMoveEvent(event);
}

void ThumbnailListView::startDrag(Qt::DropActions supportedActions)
{
    return;
    Q_UNUSED(supportedActions);
    QString text = "xxxxxxxxxxxxxx";
    QIcon icon = QIcon(":/resources/images/other/lingmo-album.svg");
    QIcon icon_hover = QIcon(":/resources/images/other/lingmo-album.svg");
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << text << icon << icon_hover;
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(myMimeType(), itemData);
    QDrag *pDrag = new QDrag(this);
    QPixmap p = QPixmap(":/resources/images/other/lingmo-album.svg");
    pDrag->setMimeData(mimeData);
    pDrag->setPixmap(p);
    pDrag->exec(Qt::MoveAction);
}

void ThumbnailListView::showEvent(QShowEvent *event)
{
    qDebug() << __FUNCTION__ << "---";
    Q_UNUSED(event);
    int i_totalwidth = width() - RIGHT_MARGIN;
    //计算一行的个数
    m_rowSizeHint = i_totalwidth / (m_iBaseHeight + ITEM_SPACING);
    m_onePicWidth = (i_totalwidth - ITEM_SPACING * (m_rowSizeHint - 1)) / m_rowSizeHint;//一张图的宽度
}

void ThumbnailListView::hideEvent(QHideEvent *event)
{
    // 控件隐藏时，若有焦点，清空焦点，可以避免焦点自动跳转到下一控件
    QWidget* fw = QApplication::focusWidget();
    if (fw == this)
        this->clearFocus();

    DListView::hideEvent(event);
}

void ThumbnailListView::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() < 0) {
        //向下滑时才可执行
        m_animationEnable = false;
    } else {
        m_animationEnable = true;
    }

    if (DApplication::keyboardModifiers() == Qt::ControlModifier) {
        GlobalStatus::instance()->sigZoomInOutFromQWidget(event->angleDelta().y());
    }

    DListView::wheelEvent(event);
}

QRect ThumbnailListView::visualRect(const QModelIndex &index) const
{
    //获取当前视图中的item项,找到视图中的index,注意：获取到的index可能不是最大值,但已经足够了
    return QListView::visualRect(index);
}

void ThumbnailListView::sltReloadAfterFilterEnd(const QVector<DBImgInfo> &allImageDataVector)
{
    if (m_delegatetype != ThumbnailDelegate::AllPicViewType) {
        return;
    }
    for (int i = (m_model->rowCount() - 1); i >= 0; i--) {
        QModelIndex index = m_model->index(i, 0);
        DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if ((data.itemType == ItemTypePic || data.itemType == ItemTypeVideo) && !QFile::exists(data.filePath)) {
            m_model->removeRow(index.row());
        }
    }
    //m_model->rowCount()减一是为了减去顶部空白栏占用的一个ModelIndex
    for (int j = (m_model->rowCount() - 1); j < allImageDataVector.size(); j++) {
        insertThumbnail(allImageDataVector.at(j));
    }

    // 添加底栏空白区域
    insertBlankOrTitleItem(ItemTypeBlank, "", "", STATUSBAR_HEIGHT);

    //加载完后通知所有图片页刷新状态栏显示数量
    emit sigUpdatePicNum();
}

void ThumbnailListView::onUpdateListview()
{
    if (this->isVisible()) {
        this->update();
    }
}

void ThumbnailListView::mouseReleaseEvent(QMouseEvent *event)
{
    //触屏状态复位
    touchStatus = 0;
    updateEnableSelectionByMouseTimer->stop();
    m_animationEnable = false;

    if (COMMON_STR_RECENT_IMPORTED  == m_imageType) {
        if (QApplication::keyboardModifiers() == Qt::NoModifier) {
            emit sigMouseRelease();
        }
    } else {
        emit sigMouseRelease();
    }

    DListView::mouseReleaseEvent(event);

    //每次选中需要更新右键菜单，以便快捷键响应，多选不响应，单选响应
    //BUG#101481 调整为在list view响应事件后刷新菜单
    updateMenuContents();

    //updatetimeLimeBtnText();
}

void ThumbnailListView::keyPressEvent(QKeyEvent *event)
{
    DListView::keyPressEvent(event);
    m_dragItemPath = selectedPaths();
    int id = -1;
    if (event->key() == Qt::Key_F11) {
        id = Types::IdFullScreen;
    } else if (event->key() == Qt::Key_F5) {
        id = Types::IdStartSlideShow;
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        id = Types::IdView;
    } else if (event->key() == Qt::Key_Period) {
        if (AlbumControl::instance()->canFavorite(m_dragItemPath))
            id = Types::IdAddToFavorites;
        else
            id = Types::IdRemoveFromFavorites;
    }else if (event->modifiers() == Qt::ControlModifier) {
        if (event->key() == Qt::Key_E) {
            id =Types::IdExport;
        } else if (event->key() == Qt::Key_I) {
            if (m_dragItemPath.isEmpty())
                return;
            if (Libutils::image::isVideo(LibUnionImage_NameSpace::localPath(m_dragItemPath.first())))
                id = Types::IdVideoInfo;
            else
                id = Types::IdImageInfo;
        } else if (event->key() == Qt::Key_F9) {
            id = Types::IdSetAsWallpaper;
        }
    } else if (event->modifiers() == Qt::AltModifier) {
        if (event->key() == Qt::Key_D)
            id = Types::IdDisplayInFileManager;
    }

    if (id != -1)
        GlobalStatus::instance()->sigMenuItemClickedFromQWidget(id);
}

void ThumbnailListView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(myMimeType())) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            if (!Libutils::base::checkMimeUrls(event->mimeData()->urls())) {
                return;
            }
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void ThumbnailListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(myMimeType())) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            if (!Libutils::base::checkMimeUrls(event->mimeData()->urls())) {
                return;
            }
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void ThumbnailListView::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_dragItemPath = selectedPaths();
    DListView::dragLeaveEvent(event);
}

void ThumbnailListView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("TestListView/text-icon-icon_hover"))
        return;
    DListView::dropEvent(event);
}

void ThumbnailListView::initConnections()
{
    connect(ImageDataService::instance(), &ImageDataService::sigeUpdateListview, this, &ThumbnailListView::onUpdateListview, Qt::ConnectionType::QueuedConnection);
    //有图片删除后，刷新列表
    //connect(dApp->signalM, &SignalManager::imagesRemovedPar, this, &ThumbnailListView::updateThumbnailViewAfterDelete);
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &ThumbnailListView::onScrollbarValueChanged);
    connect(this, &QListView::customContextMenuRequested, this, &ThumbnailListView::onShowMenu);
    connect(m_pMenu, &DMenu::triggered, this, &ThumbnailListView::onMenuItemClicked);
    connect(this, &ThumbnailListView::doubleClicked, this, &ThumbnailListView::onDoubleClicked);
    connect(this, &ThumbnailListView::clicked, this, &ThumbnailListView::onClicked);
    //connect(GlobalStatus::instance(), &GlobalStatus::thumbnailSizeLevelChanged, this, &ThumbnailListView::onThumbnailSizeLevelChanged);
    connect(GlobalStatus::instance(), &GlobalStatus::cellBaseWidthChanged, this, &ThumbnailListView::onCellBaseWidthChanged);
    connect(m_delegate, &ThumbnailDelegate::sigCancelFavorite, this, &ThumbnailListView::onCancelFavorite);

    //connect(ImageEngineApi::instance(), &ImageEngineApi::sigOneImgReady, this, &ThumbnailListView::slotOneImgReady);
    //connect(ImageEngineApi::instance(), &ImageEngineApi::sigReloadAfterFilterEnd, this, &ThumbnailListView::sltReloadAfterFilterEnd);
}

void ThumbnailListView::insertThumbnail(const DBImgInfo &dBImgInfo)
{
    DBImgInfo info = dBImgInfo;

    QStandardItem *item = new QStandardItem;
    QVariant infoVariant;
    int height = info.imgHeight;
    if (info.itemType == ItemType::ItemTypeBlank
            || info.itemType == ItemType::ItemTypeTimeLineTitle
            || info.itemType == ItemType::ItemTypeImportTimeLineTitle) {
        info.imgWidth = this->width() - 5;
    }
    infoVariant.setValue(info);
    item->setData(infoVariant, Qt::DisplayRole);
    item->setData(QVariant(QSize(info.imgWidth, height)),
                  Qt::SizeHintRole);
    m_model->appendRow(item);
    if (info.itemType == ItemTypeTimeLineTitle) {
        item->setSelectable(false);
        QModelIndex index = m_model->indexFromItem(item);
        TimeLineDateWidget *pCurrentDateWidget = new TimeLineDateWidget(item, info.date, info.num);
        connect(pCurrentDateWidget, &TimeLineDateWidget::sigIsSelectCurrentDatePic, this, &ThumbnailListView::slotSelectCurrentDatePic);
        connect(this, &ThumbnailListView::sigShowCheckBox, pCurrentDateWidget, &TimeLineDateWidget::onShowCheckBox);
        this->setIndexWidget(index, pCurrentDateWidget);
    } else if (info.itemType == ItemTypeImportTimeLineTitle) {
        item->setSelectable(false);
        QModelIndex index = m_model->indexFromItem(item);
        importTimeLineDateWidget *pCurrentDateWidget = new importTimeLineDateWidget(item, info.date, info.num);
        connect(pCurrentDateWidget, &importTimeLineDateWidget::sigIsSelectCurrentDatePic, this, &ThumbnailListView::slotSelectCurrentDatePic);
        connect(this, &ThumbnailListView::sigShowCheckBox, pCurrentDateWidget, &importTimeLineDateWidget::onShowCheckBox);
        this->setIndexWidget(index, pCurrentDateWidget);
    }
}

void ThumbnailListView::insertThumbnails(const DBImgInfoList &infos)
{
    std::vector<std::pair<QModelIndex, DWidget *>> timelines;

    for (const auto &dBImgInfo : infos) {
        DBImgInfo info = dBImgInfo;

        QStandardItem *item = new QStandardItem;
        QVariant infoVariant;
        int height = info.imgHeight;
        if (info.itemType == ItemType::ItemTypeBlank
                || info.itemType == ItemType::ItemTypeTimeLineTitle
                || info.itemType == ItemType::ItemTypeImportTimeLineTitle) {
            info.imgWidth = this->width() - 5;
        }
        infoVariant.setValue(info);
        item->setData(infoVariant, Qt::DisplayRole);
        item->setData(QVariant(QSize(info.imgWidth, height)),
                      Qt::SizeHintRole);
        m_model->appendRow(item);
        if (info.itemType == ItemTypeTimeLineTitle) {
            item->setSelectable(false);
            QModelIndex index = m_model->indexFromItem(item);
            TimeLineDateWidget *pCurrentDateWidget = new TimeLineDateWidget(item, info.date, info.num);
            connect(pCurrentDateWidget, &TimeLineDateWidget::sigIsSelectCurrentDatePic, this, &ThumbnailListView::slotSelectCurrentDatePic);
            connect(this, &ThumbnailListView::sigShowCheckBox, pCurrentDateWidget, &TimeLineDateWidget::onShowCheckBox);
            timelines.push_back(std::make_pair(index, pCurrentDateWidget));
        } else if (info.itemType == ItemTypeImportTimeLineTitle) {
            item->setSelectable(false);
            QModelIndex index = m_model->indexFromItem(item);
            importTimeLineDateWidget *pCurrentDateWidget = new importTimeLineDateWidget(item, info.date, info.num);
            connect(pCurrentDateWidget, &importTimeLineDateWidget::sigIsSelectCurrentDatePic, this, &ThumbnailListView::slotSelectCurrentDatePic);
            connect(this, &ThumbnailListView::sigShowCheckBox, pCurrentDateWidget, &importTimeLineDateWidget::onShowCheckBox);
            timelines.push_back(std::make_pair(index, pCurrentDateWidget));
        }
    }

    //最后再来统一插入控件，速度要快个几十倍
    for (const auto &eachLine : timelines) {
        this->setIndexWidget(eachLine.first, eachLine.second);
    }
}

void ThumbnailListView::stopLoadAndClear(bool bClearModel)
{
    if (bClearModel)
        m_model->clear();   //清除模型中的数据
    m_allfileslist.clear();
}
//根据显示类型，返回不同列表，如所有照片返回所有，时间线返回当前时间下内容
QStringList ThumbnailListView::getFileList(int row, ItemType type)
{
    m_allfileslist.clear();
    if (m_delegatetype == ThumbnailDelegate::AllPicViewType
            || m_delegatetype == ThumbnailDelegate::AlbumViewCustomType
            || m_delegatetype == ThumbnailDelegate::AlbumViewFavoriteType
            || m_delegatetype == ThumbnailDelegate::SearchViewType
            || m_delegatetype == ThumbnailDelegate::AlbumViewPhoneType) {
        //遍历所有数据
        for (int i = 0; i < m_model->rowCount(); i++) {
            QModelIndex index = m_model->index(i, 0);
            DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
            if (type == ItemType::ItemTypeNull) {
                if (data.itemType == ItemType::ItemTypePic || data.itemType == ItemType::ItemTypeVideo) {
                    m_allfileslist.append(data.filePath);
                }
            } else {
                if (data.itemType == type) {
                    m_allfileslist.append(data.filePath);
                }
            }

        }
    } else if (m_delegatetype == ThumbnailDelegate::TimeLineViewType
               || m_delegatetype == ThumbnailDelegate::AlbumViewImportTimeLineViewType) {
        //找到当前项前面最近的一个标题
        QModelIndex titleIndex;
        for (int i = row; i >= 0; i--) {
            QModelIndex itemIndex = m_model->index(i, 0);
            DBImgInfo data = itemIndex.data(Qt::DisplayRole).value<DBImgInfo>();
            if (data.itemType == ItemTypeTimeLineTitle
                    || data.itemType == ItemTypeImportTimeLineTitle
                    || data.itemType == ItemTypeBlank) {
                titleIndex = itemIndex;
                break;
            }
        }
        int index = titleIndex.row() + 1;
        //根据找到的标题，遍历当前时间下所有数据
        for (; index < m_model->rowCount(); index++) {
            QModelIndex itemIndex = m_model->index(index, 0);
            DBImgInfo data = itemIndex.data(Qt::DisplayRole).value<DBImgInfo>();
            if (data.itemType == ItemTypeTimeLineTitle
                    || data.itemType == ItemTypeImportTimeLineTitle) {
                break;
            } else {
                if (type == ItemType::ItemTypeNull) {
                    if (!data.filePath.isEmpty()) {
                        m_allfileslist.append(data.filePath);
                    }
                } else {
                    if (data.itemType == type) {
                        if (!data.filePath.isEmpty()) {
                            m_allfileslist.append(data.filePath);
                        }
                    }
                }
            }
        }
    }
    return m_allfileslist;
}

QList<DBImgInfo> ThumbnailListView::getAllFileInfo(int row)
{
    QList<DBImgInfo> DBImgInfos;
    if (m_delegatetype == ThumbnailDelegate::AllPicViewType
            || m_delegatetype == ThumbnailDelegate::AlbumViewCustomType
            || m_delegatetype == ThumbnailDelegate::AlbumViewFavoriteType
            || m_delegatetype == ThumbnailDelegate::SearchViewType) {
        for (int i = 0; i < m_model->rowCount(); i++) {
            QModelIndex index = m_model->index(i, 0);
            DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
            if (data.itemType == ItemType::ItemTypePic) {
                DBImgInfos.append(data);
            }
        }
    } else if (m_delegatetype == ThumbnailDelegate::TimeLineViewType
               || m_delegatetype == ThumbnailDelegate::AlbumViewImportTimeLineViewType) {
        //找到当前项前面最近的一个标题
        QModelIndex titleIndex;
        for (int i = row; i >= 0; i--) {
            QModelIndex itemIndex = m_model->index(i, 0);
            DBImgInfo data = itemIndex.data(Qt::DisplayRole).value<DBImgInfo>();
            if (data.itemType == ItemTypeTimeLineTitle
                    || data.itemType == ItemTypeImportTimeLineTitle
                    || data.itemType == ItemTypeBlank) {
                titleIndex = itemIndex;
                break;
            }
        }
        int index = titleIndex.row() + 1;
        //根据找到的标题，遍历当前时间下所有数据
        for (; index < m_model->rowCount(); index++) {
            QModelIndex itemIndex = m_model->index(index, 0);
            DBImgInfo data = itemIndex.data(Qt::DisplayRole).value<DBImgInfo>();
            if (data.itemType == ItemTypeTimeLineTitle
                    || data.itemType == ItemTypeImportTimeLineTitle) {
                break;
            } else {
                DBImgInfos.append(data);
            }
        }
    }
    return DBImgInfos;
}

int ThumbnailListView::getRow(const QString &path)
{
    int row = -1;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if (data.itemType == ItemType::ItemTypePic) {
            if (data.filePath == path) {
                row = i;
            }
        }
    }
    return row;
}

void ThumbnailListView::onShowMenu(const QPoint &pos)
{
    //外接设备显示照片时，禁用鼠标右键菜单
    QModelIndex index = this->indexAt(pos);
    if (!index.isValid() || ALBUM_PATHTYPE_BY_PHONE == m_imageType) {
        return;
    }
    //标题项和空白项不显示右键菜单
    DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();
    if (info.itemType == ItemTypeBlank
            || info.itemType == ItemTypeTimeLineTitle
            || info.itemType == ItemTypeImportTimeLineTitle) {
        return;
    }
#ifdef tablet_PC
    //区分最近删除界面和其他界面
    if (this->m_imageType == COMMON_STR_TRASH) {
        return;
    } else {
        emit sigMouseRelease();
        updateMenuContents();
        m_pMenu->popup(QCursor::pos());
    }
#else
    emit sigMouseRelease();
    updateMenuContents();
    m_pMenu->popup(QCursor::pos());
#endif
}

void ThumbnailListView::updateMenuContents()
{
    QStringList paths = selectedPaths();
    if (paths.empty()) {
        return;
    }
#ifndef tablet_PC
    if (m_imageType.compare(COMMON_STR_TRASH) == 0) {
        if (1 == paths.length()) {
            DBImgInfo data = selectedIndexes().at(0).data(Qt::DisplayRole).value<DBImgInfo>();
            if (data.itemType == ItemTypePic) {
                m_MenuActionMap.value(tr("Photo info"))->setVisible(true);
                m_MenuActionMap.value(tr("Video info"))->setVisible(false);
            } else {
                m_MenuActionMap.value(tr("Photo info"))->setVisible(false);
                m_MenuActionMap.value(tr("Video info"))->setVisible(true);
            }
        } else {
            m_MenuActionMap.value(tr("Photo info"))->setVisible(false);
            m_MenuActionMap.value(tr("Video info"))->setVisible(false);
        }
        return;
    }
#endif
    for (QAction *action : m_MenuActionMap.values()) {
        action->setVisible(true);
        action->setEnabled(true);
    }
    //源文件不存在时的菜单
    if (1 == paths.length()) {
        QFileInfo info(paths[0]);
        if (!info.exists() && COMMON_STR_TRASH != m_imageType) {
#ifndef tablet_PC
            m_MenuActionMap.value(tr("View"))->setEnabled(true);
            m_MenuActionMap.value(tr("Fullscreen"))->setEnabled(false);
            m_MenuActionMap.value(tr("Slide show"))->setEnabled(false);
            m_MenuActionMap.value(tr("Export"))->setEnabled(false);
            if (m_albumMenu)
                m_albumMenu->deleteLater();
            m_albumMenu = createAlbumMenu();
            if (m_albumMenu) {
                QAction *action = m_MenuActionMap.value(tr("Export"));
                action->setEnabled(false);
                m_albumMenu->setEnabled(false);
                m_pMenu->insertMenu(action, m_albumMenu);
            }
            m_MenuActionMap.value(tr("Copy"))->setEnabled(false);
#endif
            if (QFileInfo(info.dir(), info.dir().path()).isWritable()) {
                m_MenuActionMap.value(tr("Delete"))->setEnabled(true);
            } else {
                m_MenuActionMap.value(tr("Delete"))->setEnabled(false);
            }

            m_MenuActionMap.value(tr("Remove from album"))->setVisible(false);
#ifndef tablet_PC
            m_MenuActionMap.value(tr("Print"))->setVisible(false);
#endif
            if (DBManager::instance()->isImgExistInAlbum(DBManager::SpUID::u_Favorite, paths[0])) {
                m_MenuActionMap.value(tr("Favorite"))->setVisible(false);
                m_MenuActionMap.value(tr("Unfavorite"))->setEnabled(false);
            } else {
                m_MenuActionMap.value(tr("Unfavorite"))->setVisible(false);
                m_MenuActionMap.value(tr("Favorite"))->setEnabled(false);
            }
#ifndef tablet_PC
            m_MenuActionMap.value(tr("Rotate clockwise"))->setEnabled(false);
            m_MenuActionMap.value(tr("Rotate counterclockwise"))->setEnabled(false);
            m_MenuActionMap.value(tr("Display in file manager"))->setEnabled(false);
            m_MenuActionMap.value(tr("Photo info"))->setEnabled(false);
            m_MenuActionMap.value(tr("Video info"))->setVisible(false);
            m_MenuActionMap.value(tr("Set as wallpaper"))->setEnabled(false);
#endif
            return;
        }
#ifndef tablet_PC
        if (!info.permission(QFile::ReadOwner)) {
            m_MenuActionMap.value(tr("Set as wallpaper"))->setEnabled(false);
            m_MenuActionMap.value(tr("Set as wallpaper"))->setVisible(false);
        }
#endif
    }

#ifndef tablet_PC
    if (1 != paths.length()) {
        m_MenuActionMap.value(tr("View"))->setVisible(false);
        m_MenuActionMap.value(tr("Fullscreen"))->setVisible(false);
    }
#endif

    if (COMMON_STR_TRASH == m_imageType) {
        m_MenuActionMap.value(tr("Delete"))->setVisible(false);
    } else {
        if (!isSelectedCanUseDelete()) {
            m_MenuActionMap.value(tr("Delete"))->setVisible(false);
        } else {
            m_MenuActionMap.value(tr("Delete"))->setVisible(true);
        }

        if (m_albumMenu)
            m_albumMenu->deleteLater();
        m_albumMenu = createAlbumMenu();
        if (m_albumMenu) {
#ifndef tablet_PC
            QAction *action = m_MenuActionMap.value(tr("Export"));
#else
            QAction *action = m_MenuActionMap.value(tr("Delete"));
#endif
            m_pMenu->insertMenu(action, m_albumMenu);
        }
    }

    //非最近删除的单选
    if (1 == paths.length() && COMMON_STR_TRASH != m_imageType) {
        if (DBManager::instance()->isImgExistInAlbum(DBManager::SpUID::u_Favorite, paths[0])) {
            m_MenuActionMap.value(tr("Favorite"))->setVisible(false);
        } else {
            m_MenuActionMap.value(tr("Unfavorite"))->setVisible(false);
        }
        m_pMenu->addSeparator();
    } else {
        /*if (m_batchOperateWidget && !m_batchOperateWidget->isAllSelectedCollected()) {
            m_MenuActionMap.value(tr("Unfavorite"))->setVisible(false);
            m_MenuActionMap.value(tr("Favorite"))->setVisible(true);
        } else */if (!m_batchOperateWidget && !DBManager::instance()->isAllImgExistInAlbum(DBManager::SpUID::u_Favorite, paths, AlbumDBType::Favourite)) {
            m_MenuActionMap.value(tr("Unfavorite"))->setVisible(false);
            m_MenuActionMap.value(tr("Favorite"))->setVisible(true);
        } else {
            m_MenuActionMap.value(tr("Favorite"))->setVisible(false);
            m_MenuActionMap.value(tr("Unfavorite"))->setVisible(true);
        }
    }
    //非自定义相册、自动导入路径，隐藏从相册中移除菜单
    if (m_delegatetype != ThumbnailDelegate::AlbumViewCustomType || DBManager::instance()->getAlbumDBTypeFromUID(m_currentUID) == AutoImport) {
        m_MenuActionMap.value(tr("Remove from album"))->setVisible(false);
    }
#ifndef tablet_PC
    bool bflag_imageSupportSave = false;      //图片是否可以保存标志
    if (1 == paths.length()) { //单张照片
        if (LibUnionImage_NameSpace::isImageSupportRotate(paths[0]))
            bflag_imageSupportSave = true;
    }
    int flag_isRW = 0;
    if (1 == paths.length()) {
        if (QFileInfo(paths[0]).isReadable() && QFileInfo(paths[0]).isWritable()) {
            flag_isRW = 1;
        }
    }
    if (bflag_imageSupportSave && flag_isRW == 1) {
        m_MenuActionMap.value(tr("Rotate clockwise"))->setEnabled(true);
        m_MenuActionMap.value(tr("Rotate counterclockwise"))->setEnabled(true);
    } else {
        m_MenuActionMap.value(tr("Rotate clockwise"))->setVisible(false);
        m_MenuActionMap.value(tr("Rotate counterclockwise"))->setVisible(false);
    }
    if (1 == paths.length()) {
        DBImgInfo data = selectedIndexes().at(0).data(Qt::DisplayRole).value<DBImgInfo>();
        if (data.itemType == ItemTypePic) {
            m_MenuActionMap.value(tr("Photo info"))->setVisible(true);
            m_MenuActionMap.value(tr("Video info"))->setVisible(false);
            m_MenuActionMap.value(tr("Fullscreen"))->setVisible(true);
        } else {
            m_MenuActionMap.value(tr("Video info"))->setVisible(true);
            m_MenuActionMap.value(tr("Print"))->setVisible(false);
            m_MenuActionMap.value(tr("Slide show"))->setVisible(false);
            m_MenuActionMap.value(tr("Export"))->setVisible(false);
            m_MenuActionMap.value(tr("Photo info"))->setVisible(false);
            m_MenuActionMap.value(tr("Fullscreen"))->setVisible(false);
            m_MenuActionMap.value(tr("Set as wallpaper"))->setVisible(false);
        }
    } else {
        m_MenuActionMap.value(tr("Display in file manager"))->setVisible(false);
        m_MenuActionMap.value(tr("Photo info"))->setVisible(false);
        m_MenuActionMap.value(tr("Video info"))->setVisible(false);
        for (int i = 0; i < paths.length(); i++) {
            QString path = paths.at(i);
            if (Libutils::image::isVideo(path)) {
                m_MenuActionMap.value(tr("Print"))->setVisible(false);
                m_MenuActionMap.value(tr("Slide show"))->setVisible(false);
                m_MenuActionMap.value(tr("Export"))->setVisible(false);
                break;
            }
        }
        //有图片则可以显示幻灯片
        for (int i = 0; i < paths.length(); i++) {
            QString path = paths.at(i);
            if (!Libutils::image::isVideo(path)) {
                m_MenuActionMap.value(tr("Slide show"))->setVisible(true);
                break;
            }
        }
        //多选状态下应当支持批量旋转
        if (!isAllSelectedSupportRotate()) {
            m_MenuActionMap.value(tr("Rotate clockwise"))->setVisible(false);
            m_MenuActionMap.value(tr("Rotate counterclockwise"))->setVisible(false);
        } else {
            m_MenuActionMap.value(tr("Rotate clockwise"))->setVisible(true);
            m_MenuActionMap.value(tr("Rotate counterclockwise"))->setVisible(true);
        }
    }

    if ((1 == paths.length() || QFileInfo(paths[0]).suffix().contains("gif"))) {
        DBImgInfo data = selectedIndexes().at(0).data(Qt::DisplayRole).value<DBImgInfo>();
        if (data.itemType == ItemTypePic && QFileInfo(paths[0]).isReadable() && Libutils::base::isSupportWallpaper(paths[0])) {
            m_MenuActionMap.value(tr("Set as wallpaper"))->setVisible(true);
        } else {
            m_MenuActionMap.value(tr("Set as wallpaper"))->setVisible(false);
        }
    } else {
        m_MenuActionMap.value(tr("Set as wallpaper"))->setVisible(false);
    }
#endif
}

void ThumbnailListView::appendAction(int id, const QString &text, const QString &shortcut)
{
    QAction *ac = new QAction(this);
    addAction(ac);
    ac->setText(text);
    ac->setProperty("MenuID", id);
    //如果是查看照片，需要响应Enter键，而Enter键有两个Key-Enter和Return
    if (text.compare(tr("View")) == 0) {
        QList<QKeySequence> shortcuts;
        shortcuts.append(QKeySequence(ENTER_SHORTCUT));
        shortcuts.append(QKeySequence(RETURN_SHORTCUT));
        ac->setShortcuts(shortcuts);
    } else {
        ac->setShortcut(QKeySequence(shortcut));
    }
    ac->setShortcutContext(Qt::WidgetShortcut);
    m_MenuActionMap.insert(text, ac);
    m_pMenu->addAction(ac);
}

void ThumbnailListView::initMenuAction()
{
    m_pMenu->clear();
    if (m_imageType.compare(COMMON_STR_TRASH) == 0) {
        appendAction(Types::IdMoveToTrash, tr("Delete"), ss(""));
        appendAction(Types::IdTrashRecovery, tr("Restore"), ss(BUTTON_RECOVERY));
        appendAction(Types::IdImageInfo, tr("Photo info"), ss(ImageInfo_CONTEXT_MENU));
        appendAction(Types::IdVideoInfo, tr("Video info"), ss(VideoInfo_CONTEXT_MENU));
        return;
    }

    m_MenuActionMap.clear();
#ifndef tablet_PC
    appendAction(Types::IdView, tr("View"), ss(VIEW_CONTEXT_MENU));
    appendAction(Types::IdFullScreen, tr("Fullscreen"), ss(FULLSCREEN_CONTEXT_MENU));
    appendAction(Types::IdPrint, tr("Print"), ss(PRINT_CONTEXT_MENU));
    appendAction(Types::IdStartSlideShow, tr("Slide show"), ss(SLIDESHOW_CONTEXT_MENU));
    m_pMenu->addSeparator();
    appendAction(Types::IdExport, tr("Export"), ss(EXPORT_CONTEXT_MENU));
    appendAction(Types::IdCopyToClipboard, tr("Copy"), ss(COPYTOCLIPBOARD_CONTEXT_MENU));
#endif
    appendAction(Types::IdMoveToTrash, tr("Delete"), ss(""));
    appendAction(Types::IdRemoveFromAlbum, tr("Remove from album"), ss(""));

    m_pMenu->addSeparator();
    appendAction(Types::IdAddToFavorites, tr("Favorite"), "");
    appendAction(Types::IdRemoveFromFavorites, tr("Unfavorite"), "");
#ifndef tablet_PC
    m_pMenu->addSeparator();
    appendAction(Types::IdRotateClockwise, tr("Rotate clockwise"), ss(ROTATECLOCKWISE_CONTEXT_MENU));
    appendAction(Types::IdRotateCounterclockwise, tr("Rotate counterclockwise"),
                 ss(ROTATECOUNTERCLOCKWISE_CONTEXT_MENU));
    m_pMenu->addSeparator();
    appendAction(Types::IdSetAsWallpaper, tr("Set as wallpaper"), ss(SETASWALLPAPER_CONTEXT_MENU));
    appendAction(Types::IdDisplayInFileManager, tr("Display in file manager"), ss(DISPLAYINFILEMANAGER_CONTEXT_MENU));
    appendAction(Types::IdImageInfo, tr("Photo info"), ss(ImageInfo_CONTEXT_MENU));
    appendAction(Types::IdVideoInfo, tr("Video info"), ss(VideoInfo_CONTEXT_MENU));
#endif
}

DMenu *ThumbnailListView::createAlbumMenu()
{
    DMenu *am = new DMenu(tr("Add to album"));
    auto albums = DBManager::instance()->getAllAlbumNames();
    QAction *ac1 = new QAction(am);
    ac1->setProperty("MenuID", Types::IdAddToAlbum);
    ac1->setText(tr("New album"));
    ac1->setData(-1); //设置UID为-1表示为新建相册选项
    am->addAction(ac1);
    am->addSeparator();
    QModelIndexList indexList = selectionModel()->selectedIndexes();
    QList<int> albumNames;
    //选中的全部路径
    QStringList paths = this->selectedPaths();

    //注意：album.first是UID，album.second是album name
    for (const auto &album : albums) {
        if (DBManager::instance()->isAllImgExistInAlbum(album.first, paths, AlbumDBType::Custom)) {
            albumNames.append(album.first);
        }
    }

    for (const auto &album : albums) {
        QAction *ac = new QAction(am);
        ac->setProperty("MenuID", Types::IdAddToAlbum);
        ac->setText(
            fontMetrics().elidedText(QString(album.second).replace("&", "&&"), Qt::ElideMiddle, 200));
        ac->setData(album.first);
        am->addAction(ac);
        if (albumNames.contains(album.first)) {
            ac->setEnabled(false);
        }
    }
    return am;
}

void ThumbnailListView::onMenuItemClicked(QAction *action)
{
    QStringList paths = selectedPaths();
    menuItemDeal(paths, action);
}

//获取选中路径
QStringList ThumbnailListView::selectedPaths()
{
    QStringList paths;
    for (QModelIndex index : selectionModel()->selectedIndexes()) {
        if (isRowHidden(index.row())) {
            continue;
        }
        DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if ((info.itemType != ItemTypeBlank
                || info.itemType != ItemTypeTimeLineTitle
                || info.itemType != ItemTypeImportTimeLineTitle)
                && !info.filePath.isEmpty()) {
            paths << info.filePath;
        }
    }
    return paths;
}

QVariantList ThumbnailListView::selectedUrls()
{
    QVariantList urls;
    for (QModelIndex index : selectionModel()->selectedIndexes()) {
        if (isRowHidden(index.row())) {
            continue;
        }
        DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if ((info.itemType != ItemTypeBlank
             || info.itemType != ItemTypeTimeLineTitle
             || info.itemType != ItemTypeImportTimeLineTitle)
            && !info.filePath.isEmpty()) {
            urls << QUrl::fromLocalFile(info.filePath);
        }
    }
    return urls;
}

QVariantList ThumbnailListView::allUrls()
{
    QVariantList urls;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        if (isRowHidden(index.row())) {
            continue;
        }
        DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if ((info.itemType != ItemTypeBlank
             || info.itemType != ItemTypeTimeLineTitle
             || info.itemType != ItemTypeImportTimeLineTitle)
            && !info.filePath.isEmpty()) {
            urls << QUrl::fromLocalFile(info.filePath);
        }
    }
    return urls;
}

void ThumbnailListView::menuItemDeal(QStringList paths, QAction *action)
{
    paths.removeAll(QString(""));
    if (paths.isEmpty()) {
        return;
    }

    const QString path = paths.first();
    const int id = action->property("MenuID").toInt();
    int UID = action->data().toInt();
    if (id == Types::IdAddToAlbum) {
        if (UID == -1)
            GlobalStatus::instance()->sigMenuItemClickedFromQWidget(Types::IdNewAlbum);
        else
            GlobalStatus::instance()->sigMenuItemClickedFromQWidget(id, UID);
    } else
        GlobalStatus::instance()->sigMenuItemClickedFromQWidget(id);
}

void ThumbnailListView::onThumbnailSizeLevelChanged()
{
//    if (!this->isVisible())
//        return;
    int value = GlobalStatus::instance()->thumbnailSizeLevel();
    switch (value) {
    case 0:
        m_iBaseHeight = 80;
        break;
    case 1:
        m_iBaseHeight = 90;
        break;
    case 2:
        m_iBaseHeight = 100;
        break;
    case 3:
        m_iBaseHeight = 110;
        break;
    case 4:
        m_iBaseHeight = 120;
        break;
    case 5:
        m_iBaseHeight = 130;
        break;
    case 6:
        m_iBaseHeight = 140;
        break;
    case 7:
        m_iBaseHeight = 150;
        break;
    case 8:
        m_iBaseHeight = 160;
        break;
    case 9:
        m_iBaseHeight = 170;
        break;
    default:
        m_iBaseHeight = 80;
        break;
    }
    resizeEventF();
}

void ThumbnailListView::onCellBaseWidthChanged()
{
    m_iBaseHeight = GlobalStatus::instance()->cellBaseWidth();
    resizeEventF();
}

void ThumbnailListView::onSelectionChanged()
{
    updatetimeLimeBtnText();
    GlobalStatus::instance()->setSelectedPaths(selectedUrls());
}

void ThumbnailListView::onCancelFavorite(const QModelIndex &index)
{
    QStringList str;
    DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();
    str << info.filePath;
    //通知其它界面更新取消收藏
    DBManager::instance()->removeFromAlbum(DBManager::SpUID::u_Favorite, str, AlbumDBType::Favourite);
}

void ThumbnailListView::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    //改变第一个空白项和标题项的宽度
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if (data.itemType == ItemType::ItemTypeBlank
                || data.itemType == ItemType::ItemTypeTimeLineTitle
                || data.itemType == ItemType::ItemTypeImportTimeLineTitle) {
            data.imgWidth = this->width() - 5;
            QVariant meta;
            meta.setValue(data);
            m_model->setData(index, meta, Qt::DisplayRole);
            m_model->setData(index, QVariant(QSize(data.imgWidth, data.imgWidth)), Qt::SizeHintRole);
        }
    }

    resizeEventF();

    this->verticalScrollBar()->setFixedHeight(this->height());
//    if (m_model->rowCount() > 0) {
//        for (int i = 0; i < m_model->rowCount(); i++) {
//            QModelIndex index = m_model->index(i, 0);
//            QRect rect = this->visualRect(index);
//            qDebug() << "------" << __FUNCTION__ << "---row = " << i << "---rect = " << rect;
//        }
//        qDebug() << "------" << __FUNCTION__ << "--------rect = " << this->rect();
//    }
}

bool ThumbnailListView::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == verticalScrollBar()) {
        if (e->type() == QEvent::MouseButtonPress) {
            //刷新鼠标在滚动条上按下标志位
            QRect scrollGlobaRect(verticalScrollBar()->mapToGlobal(QPoint(0, 0)),
                                  QSize(verticalScrollBar()->geometry().width(), verticalScrollBar()->geometry().height()));
            if (scrollGlobaRect.contains(QCursor::pos())) {
                m_animationEnable = true;
            }
        } else if (e->type() == QEvent::MouseButtonRelease) {
            //刷新鼠标在滚动条上释放标志位
            m_animationEnable = false;
        }
    }

    if (e->type() == QEvent::Wheel && QApplication::keyboardModifiers() == Qt::ControlModifier) {
        return true;
    } else if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_PageDown) {
            DScrollBar *vb = this->verticalScrollBar();
            int posValue = vb->value();
            posValue += this->height();
            vb->setValue(posValue);
            return true;
        } else if (keyEvent->key() == Qt::Key_PageUp) {
            DScrollBar *vb = this->verticalScrollBar();
            int posValue = vb->value();
            posValue -= this->height();
            vb->setValue(posValue);
            return true;
        }
    }
    return false;
}

QStringList ThumbnailListView::getCurrentIndexTime(const QModelIndex &index)
{
    // index对应的是图片，获取所属时间线的时间和数量
    QStringList list;
    if (index.isValid()) {
        for (int i = index.row(); i >= 0; i--) {
            QModelIndex idx = m_model->index(i, 0);
            DBImgInfo tempdata = idx.data(Qt::DisplayRole).value<DBImgInfo>();
            if (tempdata.itemType == ItemTypeBlank || tempdata.itemType == ItemTypeTimeLineTitle
                    || tempdata.itemType == ItemTypeImportTimeLineTitle) {
                list.append(tempdata.date);
                list.append(tempdata.num);
                break;
            }
        }
    }
    return list;
}

bool ThumbnailListView::getCurrentIndexSelectStatus(const QModelIndex &index, bool isPic)
{
    if (!index.isValid()) {
        return false;
    }
    if (isPic) {
        //图片，向前循环判断选中状态
        for (int i = index.row(); i > 0; i--) {
            QModelIndex idx = m_model->index(i, 0);
            DBImgInfo tempdata = idx.data(Qt::DisplayRole).value<DBImgInfo>();
            if (tempdata.itemType == ItemTypeBlank || tempdata.itemType == ItemTypeTimeLineTitle
                    || tempdata.itemType == ItemTypeImportTimeLineTitle) {
                break;
            }
            if (!selectionModel()->isSelected(idx)) {
                return false;
            }
        }
        //图片，向后循环判断选中状态
        for (int j = index.row(); j < m_model->rowCount(); j++) {
            QModelIndex idx = m_model->index(j, 0);
            DBImgInfo tempdata = idx.data(Qt::DisplayRole).value<DBImgInfo>();
            if (tempdata.itemType == ItemTypeBlank || tempdata.itemType == ItemTypeTimeLineTitle
                    || tempdata.itemType == ItemTypeImportTimeLineTitle) {
                break;
            }
            if (!selectionModel()->isSelected(idx)) {
                return false;
            }
        }
    } else {//标题，+1进入第一张
        for (int j = index.row() + 1; j < m_model->rowCount(); j++) {
            QModelIndex idx = m_model->index(j, 0);
            DBImgInfo tempdata = idx.data(Qt::DisplayRole).value<DBImgInfo>();
            if (tempdata.itemType == ItemTypeBlank || tempdata.itemType == ItemTypeTimeLineTitle
                    || tempdata.itemType == ItemTypeImportTimeLineTitle) {
                break;
            }
            if (!selectionModel()->isSelected(idx)) {
                return false;
            }
        }
    }
    return true;
}

bool ThumbnailListView::isAllAppointType(ItemType type)
{
    bool isAllAppointType = true;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if (type == ItemTypePic) {
            if (info.itemType == ItemTypeVideo) {
                isAllAppointType = false;
                break;
            }
        } else if (type == ItemTypeVideo) {
            if (info.itemType == ItemTypePic) {
                isAllAppointType = false;
                break;
            }
        }
    }
    qDebug() << __FUNCTION__ << "---isAllAppointType = " << isAllAppointType;
    return isAllAppointType;
}

void ThumbnailListView::hideAllAppointType(ItemType type)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if (info.itemType == type) {
            setRowHidden(i, true);
        } else {
            setRowHidden(i, false);
        }
    }
    //有可能出现图片或者视频删除好，时间线页面两个标题在一起的情况
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();
        //空白栏下第一个未隐藏的是标题，隐藏标题，同时将标题中的时间和数量传递给空白栏
        if (info.itemType == ItemTypeBlank) {
            bool nextVisibleIsTitle = false;
            for (int j = i + 1; j < m_model->rowCount(); j++) {
                QModelIndex nextIndex = m_model->index(j, 0);
                DBImgInfo nextInfo = nextIndex.data(Qt::DisplayRole).value<DBImgInfo>();
                if (!isRowHidden(j) && (nextInfo.itemType == ItemTypePic || nextInfo.itemType == ItemTypeVideo)) {
                    break;
                }
                if (!isRowHidden(j)) {
                    if (nextInfo.itemType == ItemTypeTimeLineTitle || nextInfo.itemType == ItemTypeImportTimeLineTitle) {
                        setRowHidden(j, true);
                        bool isSelect = getCurrentIndexSelectStatus(nextIndex, false);
                        emit sigTimeLineDataAndNum(nextInfo.date, nextInfo.num, isSelect ? QObject::tr("Unselect") : QObject::tr("Select"));
                        nextVisibleIsTitle = true;
                        break;
                    }
                }
            }
            if (nextVisibleIsTitle) {
                i--;
                continue;
            }
        } else if (info.itemType == ItemTypeTimeLineTitle
                   || info.itemType == ItemTypeImportTimeLineTitle) {
            //当前是标题，下一个未隐藏的也是标题时，隐藏当前标题
            for (int j = i + 1; j < m_model->rowCount(); j++) {
                QModelIndex nextIndex = m_model->index(j, 0);
                DBImgInfo nextInfo = nextIndex.data(Qt::DisplayRole).value<DBImgInfo>();
                if (!isRowHidden(j) && (nextInfo.itemType == ItemTypePic || nextInfo.itemType == ItemTypeVideo)) {
                    break;
                }
                if (!isRowHidden(j)) {
                    if (nextInfo.itemType == ItemTypeTimeLineTitle || nextInfo.itemType == ItemTypeImportTimeLineTitle || nextInfo.itemType == ItemTypeBlank) {
                        setRowHidden(i, true);
                        break;
                    }
                }
                //当前是最后一个标题
                if (isRowHidden(j) && (j == (m_model->rowCount() - 1))) {
                    setRowHidden(i, true);
                }
            }
        }
    }

    flushTopTimeLine(8); //筛完了要刷新顶部
}

int ThumbnailListView::getRow(QPoint point)
{
    return indexAt(point).row();
}

void ThumbnailListView::setListViewUseFor(ThumbnailListView::ListViewUseFor usefor)
{
    m_useFor = usefor;
}

//add start 3975
int ThumbnailListView::getListViewHeight()
{
    return m_height;
}
//add end 3975
//有文件删除，刷新所有列表
void ThumbnailListView::updateThumbnailViewAfterDelete(const QStringList &paths)
{
    //列表上移除所有图片项
    if (m_delegatetype == ThumbnailDelegate::AllPicViewType
            || m_delegatetype == ThumbnailDelegate::AlbumViewCustomType
            || m_delegatetype == ThumbnailDelegate::AlbumViewFavoriteType
            || m_delegatetype == ThumbnailDelegate::TimeLineViewType
            || m_delegatetype == ThumbnailDelegate::AlbumViewImportTimeLineViewType) {
        for (int i = (m_model->rowCount() - 1); i >= 0; i--) {
            QModelIndex index = m_model->index(i, 0);
            DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
            if (paths.contains(data.filePath)) { //你永远可以相信QStringList::contains
                m_model->removeRow(i);
                break;
            }
        }
    }
    if (m_delegatetype == ThumbnailDelegate::TimeLineViewType
            || m_delegatetype == ThumbnailDelegate::AlbumViewImportTimeLineViewType) {
        //给需要移除的项添加标志
        for (int i = 0; i < m_model->rowCount(); i++) {
            QModelIndex index = m_model->index(i, 0);
            DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
            if (i < (m_model->rowCount() - 1)) {
                QModelIndex indexNext = m_model->index((i + 1), 0);
                DBImgInfo dataNext = indexNext.data(Qt::DisplayRole).value<DBImgInfo>();
                //当前项和下一项都是标题，则当前项标记为需要删除
                if ((data.itemType == ItemTypeTimeLineTitle && dataNext.itemType == ItemTypeTimeLineTitle)
                        || (data.itemType == ItemTypeImportTimeLineTitle && dataNext.itemType == ItemTypeImportTimeLineTitle)) {
                    data.bNeedDelete = true;
                    QVariant meta;
                    meta.setValue(data);
                    m_model->setData(index, meta, Qt::DisplayRole);
                }
            } else if (i == (m_model->rowCount() - 1)) {
                //最后一项是标题，标记需要删除
                if (data.itemType == ItemTypeTimeLineTitle || data.itemType == ItemTypeImportTimeLineTitle) {
                    data.bNeedDelete = true;
                    QVariant meta;
                    meta.setValue(data);
                    m_model->setData(index, meta, Qt::DisplayRole);
                }
            }
            if (i == 1) {
                //第二项是标题，标记需要删除
                if (data.itemType == ItemTypeTimeLineTitle || data.itemType == ItemTypeImportTimeLineTitle) {
                    data.bNeedDelete = true;
                    QVariant meta;
                    meta.setValue(data);
                    m_model->setData(index, meta, Qt::DisplayRole);
                }
            }
        }
        //移除添加了标志的项
        for (int i = 0; i < m_model->rowCount(); i++) {
            QModelIndex index = m_model->index(i, 0);
            DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
            if (data.bNeedDelete) {
                m_model->removeRow(i);
                //移除后先减1，后面加1的时候才不会遗漏
                i--;
            }
        }
    }
}

void ThumbnailListView::slotSelectCurrentDatePic(bool isSelect, QStandardItem *item)
{
    if (item == nullptr || !m_model->indexFromItem(item).isValid()) {
        return;
    }

    //disconnect(this->selectionModel(), &QItemSelectionModel::selectionChanged, m_batchOperateWidget, &BatchOperateWidget::sltSelectionChanged);

    int index = m_model->indexFromItem(item).row() + 1;
    for (; index < m_model->rowCount(); index++) {
        QModelIndex itemIndex = m_model->index(index, 0);
        DBImgInfo data = itemIndex.data(Qt::DisplayRole).value<DBImgInfo>();
        if (data.itemType == ItemTypeTimeLineTitle || data.itemType == ItemTypeImportTimeLineTitle) {
            break;
        } else {
            if (isSelect) {
                selectionModel()->select(itemIndex, QItemSelectionModel::Select);
            } else {
                selectionModel()->select(itemIndex, QItemSelectionModel::Deselect);
            }
        }
    }

    //connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, m_batchOperateWidget, &BatchOperateWidget::sltSelectionChanged);

    QItemSelection selected;
    QItemSelection deselected;
    emit this->selectionModel()->selectionChanged(selected, deselected);
    flushTopTimeLine(8);
}

void ThumbnailListView::onKeyPressFromQml(const QString &key)
{
    if (key == "Esc") {
        clearSelection();
    } else if (key == "Ctrl+A") {
        selectAll();
    } else if (key == "Page Up") {
        DScrollBar *vb = this->verticalScrollBar();
        int posValue = vb->value();
        posValue -= this->height();
        vb->setValue(posValue);
    } else if (key == "Page Down") {
        DScrollBar *vb = this->verticalScrollBar();
        int posValue = vb->value();
        posValue += this->height();
        vb->setValue(posValue);
    }
}

//刷新所有标题中选择按钮的状态
void ThumbnailListView::slotChangeAllSelectBtnVisible(bool visible)
{
//    if (m_delegatetype == ThumbnailDelegate::TimeLineViewType) {
//        TimeLineDateWidget *w = nullptr;
//        for (int i = 0; i < m_model->rowCount(); i++) {
//            QModelIndex idx = m_model->index(i, 0);
//            DBImgInfo data = idx.data(Qt::DisplayRole).value<DBImgInfo>();
//            if (data.itemType == ItemTypeTimeLineTitle) {
//                w = static_cast<TimeLineDateWidget *>(this->indexWidget(idx));
//                if (w != nullptr)  {
//                    w->onChangeChooseBtnVisible(visible);
//                }
//            }
//        }
//    } else if (m_delegatetype == ThumbnailDelegate::AlbumViewImportTimeLineViewType) {
//        importTimeLineDateWidget *w = nullptr;
//        for (int i = 0; i < m_model->rowCount(); i++) {
//            QModelIndex idx = m_model->index(i, 0);
//            DBImgInfo data = idx.data(Qt::DisplayRole).value<DBImgInfo>();
//            if (data.itemType == ItemTypeImportTimeLineTitle) {
//                w = static_cast<importTimeLineDateWidget *>(this->indexWidget(idx));
//                if (w != nullptr)  {
//                    w->onChangeChooseBtnVisible(visible);
//                }
//            }
//        }
//    }
}

// 选中重复导入的图片
void ThumbnailListView::selectUrls(const QStringList &urls)
{
    clearSelection();
    selectPhotos(AlbumControl::instance()->urls2localPaths(urls));
}

// 选中重复导入的图片
void ThumbnailListView::selectPhotos(const QStringList &paths)
{
    //遍历选择过程中先不刷新批量操作按钮
//    if (m_batchOperateWidget) {
//        disconnect(this->selectionModel(), &QItemSelectionModel::selectionChanged,
//                   m_batchOperateWidget, &BatchOperateWidget::sltSelectionChanged);
//    }
    QModelIndex firstIndex;

    if (!paths.isEmpty()) {
        this->clearSelection();
        QElapsedTimer timer_4s;
        timer_4s.start();
        for (int i = 0; i < m_model->rowCount(); i++) {
            QModelIndex index = m_model->index(i, 0);
            DBImgInfo info = index.data(Qt::DisplayRole).value<DBImgInfo>();

            if (paths.contains(info.filePath)) {
                // 选中
                selectionModel()->select(index, QItemSelectionModel::Select); //主要耗时点
                if (!firstIndex.isValid()) {
                    firstIndex = index;
                }
            }

            if (timer_4s.elapsed() > 4000) { //只给4秒时间，能选多少选多少
                break;
            }
        }
    }

    // 定位第一个重复导入的照片
    if (firstIndex.isValid()) {
        this->scrollTo(firstIndex, ScrollHint::PositionAtCenter);
    }
    //选择完成后刷新批量操作按钮
//    if (m_batchOperateWidget) {
//        connect(this->selectionModel(), &QItemSelectionModel::selectionChanged,
//                m_batchOperateWidget, &BatchOperateWidget::sltSelectionChanged);
//        m_batchOperateWidget->sltSelectionChanged(QItemSelection(), QItemSelection());
//    }
}

void ThumbnailListView::selectFirstPhoto()
{
    if (m_model->rowCount() < 1)
        return;
    QModelIndex idx = m_model->index(0, 0);
    selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
    selectionModel()->select(idx, QItemSelectionModel::Select);
}

bool ThumbnailListView::isFirstPhotoSelected()
{
    QModelIndexList idx = selectionModel()->selectedIndexes();
    for (QModelIndex index : selectionModel()->selectedIndexes()) {
        if (index.row() == 0) {
            return true;
        }
    }
    return  false;
}

bool ThumbnailListView::isNoPhotosSelected()
{
    QModelIndexList indexList = selectionModel()->selectedIndexes();
    if (indexList.count() > 0)
        return false;
    else
        return true;
}

void ThumbnailListView::clearAll()
{
    m_model->clear();
}
//插入一个空白项，ItemTypeBlank
//插入时间线标题，ItemTypeTimeLineTitle
//插入已导入时间线标题，ItemTypeImportTimeLineTitle
void ThumbnailListView::insertBlankOrTitleItem(ItemType type, const QString &date, const QString &num, int height)
{
    DBImgInfo info;
    info.itemType = type;
    info.imgWidth = this->width();
    if (type == ItemTypeBlank) {
        m_blankItemHeight = height;
    }
    info.imgHeight = height;
    info.date = date;
    info.num = num;
    insertThumbnail(info);
}
//更新空白栏高度
void ThumbnailListView::resetBlankItemHeight(int height)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex idx = m_model->index(i, 0);
        DBImgInfo data = idx.data(Qt::DisplayRole).value<DBImgInfo>();
        if (data.itemType == ItemTypeBlank) {
            m_blankItemHeight = height;
            data.imgHeight = height;

            QVariant infoVariant;
            infoVariant.setValue(data);
            m_model->setData(idx, infoVariant, Qt::DisplayRole);
            break;
        }
    }
    doItemsLayout();
}

void ThumbnailListView::insertThumbnailByImgInfos(DBImgInfoList infoList)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    for (int i = 0; i < infoList.size(); ++i) {
        DBImgInfo info = infoList.at(i);
        info.imgWidth = m_onePicWidth;
        info.imgHeight = m_onePicWidth;
        insertThumbnail(info);
    }
    //如果不是显示全部，则重新过滤显示
    if (m_currentShowItemType != ItemTypeNull) {
        this->showAppointTypeItem(m_currentShowItemType);
    }

    //启动主动update机制
    m_importTimer->start(100);
    m_importActiveCount = 150;
}
//判断所有图片是否全选中
bool ThumbnailListView::isAllSelected(ItemType type)
{
    bool isAllSelected = true;
    qDebug() << __FUNCTION__ << "---type = " << type;

    if (this->selectionModel()->selection().size() == 0) {
        return false;
    }
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex idx = m_model->index(i, 0);
        DBImgInfo data = idx.data(Qt::DisplayRole).value<DBImgInfo>();
        //全选
        if (type == ItemType::ItemTypeNull) {
            if ((data.itemType == ItemTypeVideo || data.itemType == ItemTypePic)
                    && !this->selectionModel()->selection().contains(idx)) {
                isAllSelected = false;
                break;
            }
        } else if (type == ItemTypePic) {
            if (data.itemType == ItemTypePic && !this->selectionModel()->selection().contains(idx)) {
                isAllSelected = false;
                break;
            }
        } else if (type == ItemTypeVideo) {
            if (data.itemType == ItemTypeVideo && !this->selectionModel()->selection().contains(idx)) {
                isAllSelected = false;
                break;
            }
        }
    }
    return isAllSelected;
}

//判断选中图片是否都可旋转
bool ThumbnailListView::isAllSelectedSupportRotate()
{
    QStringList list = selectedPaths();
    if (list.isEmpty()) {
        return false;
    }
    bool isAllSelectedSupportRotate = true;
    for (const auto &path : list) {
        if (!(LibUnionImage_NameSpace::isImageSupportRotate(path) && QFileInfo(path).isReadable() && QFileInfo(path).isWritable())) { //只要有一个不能转，就都不能转
            isAllSelectedSupportRotate = false;
            break;
        }
    }
    return isAllSelectedSupportRotate;
}

bool ThumbnailListView::isSelectedCanUseFavorite()
{
    QStringList list = selectedPaths();
    if (list.isEmpty()) {
        return false;
    }
    bool canUseFavorite = true;
    for (const auto &path : list) {
        if (!QFile::exists(path)) { //当有文件不存在时，不可使用收藏
            canUseFavorite = false;
            break;
        }
    }
    return canUseFavorite;
}

bool ThumbnailListView::isSelectedCanUseDelete()
{
    QStringList list = selectedPaths();
    if (list.isEmpty()) {
        return false;
    }
    bool isSelectedCanUseDelete = false;
    for (const auto &path : list) {
        QFileInfo info(path);
        if (info.isSymLink()) {
            info = QFileInfo(info.readSymLink());
        }
        //路径无写权限
        if (!QFileInfo(info.dir(), info.dir().path()).isWritable()) {
            continue;
        } else {  //只要有一个能写或不存在，就可以删
            isSelectedCanUseDelete = true;
            break;
        }
    }

    return isSelectedCanUseDelete;
}

//删除到相册已删除
void ThumbnailListView::removeSelectToTrash(QStringList paths)
{
    DBImgInfoList infos;
    DBImgInfo info;
    int count = 0;
    for (int i = 0; i < paths.size(); i++) {
        QFileInfo infox(paths.at(i));
        if (infox.isSymLink()) {
            infox = QFileInfo(infox.readSymLink());
        }
        //文件或者文件夹不可写
        if (!QFileInfo(infox.dir(), infox.dir().path()).isWritable()) {
            continue;
        } else {
            count++;
        }
    }
//    ImgDeleteDialog *dialog = new ImgDeleteDialog(this, count, COMMON_STR_TRASH == m_imageType);
//    dialog->setObjectName("deteledialog");
//    if (dialog->exec() > 0) {
//        clearSelection();

//        //清理界面
//        if (COMMON_STR_VIEW_TIMELINE == m_imageType || COMMON_STR_RECENT_IMPORTED == m_imageType) {
//            emit sigMoveToTrash();
//        }

//        //执行删除
//        (COMMON_STR_TRASH == m_imageType) ? ImageEngineApi::instance()->moveImagesToTrash(paths, true, false)
//        : ImageEngineApi::instance()->moveImagesToTrash(paths);
//    }
}
//更新时间线界面内各个按钮的text状态，单选/框选
void ThumbnailListView::updatetimeLimeBtnText()
{
   QModelIndexList indexs = selectionModel()->selectedIndexes();
   emit sigShowCheckBox(!indexs.isEmpty());
   bool isSelectAll = true;
   if (m_delegatetype == ThumbnailDelegate::TimeLineViewType) {
       TimeLineDateWidget *w = nullptr;
       for (int i = 0; i < m_model->rowCount(); i++) {
           QModelIndex idx = m_model->index(i, 0);
           DBImgInfo data = idx.data(Qt::DisplayRole).value<DBImgInfo>();
           if (data.itemType == ItemTypeBlank) {
               continue;
           }
           if (data.itemType == ItemTypeTimeLineTitle) {
               if (w != nullptr) {
                   w->onTimeLinePicSelectAll(isSelectAll);
               }
               w = static_cast<TimeLineDateWidget *>(this->indexWidget(idx));
               isSelectAll = true;
               continue;
           }
           if (!indexs.contains(idx)) {
               isSelectAll = false;
           }
       }
       //最后一个时间线的相关逻辑
       if (w) {
           w->onTimeLinePicSelectAll(isSelectAll);
       }
   }  else if (m_delegatetype == ThumbnailDelegate::AlbumViewImportTimeLineViewType) {
       importTimeLineDateWidget *iw = nullptr;
       isSelectAll = true;
       for (int i = 0; i < m_model->rowCount(); i++) {
           QModelIndex idx = m_model->index(i, 0);
           DBImgInfo data = idx.data(Qt::DisplayRole).value<DBImgInfo>();
           if (data.itemType == ItemTypeBlank) {
               continue;
           }
           if (data.itemType == ItemTypeImportTimeLineTitle) {
               if (iw != nullptr) { //说明是第一个时间线
                   iw->onTimeLinePicSelectAll(isSelectAll);
               }
               iw = static_cast<importTimeLineDateWidget *>(this->indexWidget(idx));
               isSelectAll = true;
               continue;
           }
           if (!indexs.contains(idx)) {
               isSelectAll = false;
           }
       }
       //最后一个时间线的相关逻辑
       if (iw) {
           iw->onTimeLinePicSelectAll(isSelectAll);
       }
   }
    //刷新顶部栏
    flushTopTimeLine(8);
}

void ThumbnailListView::showAppointTypeItem(ItemType type)
{
    //记录当前显示类型
    m_currentShowItemType = type;
    //切换显示之前先清除选中
    this->clearSelection();
    if (type == ItemTypePic) {
        //显示所有图片
        if (isAllAppointType(ItemTypeVideo)) {
            //如果全是视频，显示无结果
            //todo
            emit sigNoPicOrNoVideo(true);
        } else {
            //隐藏视频项
            hideAllAppointType(ItemTypeVideo);
            emit sigNoPicOrNoVideo(false);
        }
    } else if (type == ItemTypeVideo) {
        //显示所有视频
        if (isAllAppointType(ItemTypePic)) {
            //如果全是图片，显示无结果
            //todo
            emit sigNoPicOrNoVideo(true);
        } else {
            //隐藏图片项
            hideAllAppointType(ItemTypePic);
            emit sigNoPicOrNoVideo(false);
        }
    } else if (type == ItemTypeNull) {
        //恢复显示所有
        hideAllAppointType(ItemTypeNull);
        emit sigNoPicOrNoVideo(false);
    }
}

//显示类型数量
int ThumbnailListView::getAppointTypeItemCount(ItemType type)
{
    int count = 0;
    for (int i = 0;  i < m_model->rowCount(); i++) {
        if (isRowHidden(i)) {
            continue;
        }
        QModelIndex index = m_model->index(i, 0);
        DBImgInfo pdata = index.data(Qt::DisplayRole).value<DBImgInfo>();
        //全选
        if (type == ItemType::ItemTypeNull) {
            if (pdata.itemType == ItemTypeVideo || pdata.itemType == ItemTypePic) {
                count++;
            }
        } else if (type == pdata.itemType) {
            count++;
        }
    }
    return count;
}
//显示指定类型选中项数量
int ThumbnailListView::getAppointTypeSelectItemCount(ItemType type)
{
    int count = 0;
    QModelIndexList list = selectedIndexes();
    int size = list.size();
    for (int i = 0;  i < size; i++) {
        QModelIndex index = list.at(i);
        if (isRowHidden(index.row())) {
            continue;
        }
        DBImgInfo pdata = index.data(Qt::DisplayRole).value<DBImgInfo>();
        //全选
        if (type == ItemType::ItemTypeNull) {
            if (pdata.itemType == ItemTypeVideo || pdata.itemType == ItemTypePic) {
                count++;
            }
        } else if (type == pdata.itemType) {
            count++;
        }
    }
    return count;
}
//按类型选择
void ThumbnailListView::selectAllByItemType(ItemType type)
{
    qDebug() << __FUNCTION__ << "---type = " << type;
    this->selectAll();
    emit sigSelectAll();
    updateMenuContents();
    //因为性能问题，未根据类型选择，这里做全选处理，全选后，在获取选中项处过滤隐藏项
//    if (type == ItemTypeNull) {
//        this->selectAll();
//    } else {
//        for (int i = 0;  i < m_model->rowCount(); i++) {
//            QModelIndex index = m_model->index(i, 0);
//            DBImgInfo pdata = index.data(Qt::DisplayRole).value<DBImgInfo>();
//            if (pdata.itemType  == type) {
//                selectionModel()->select(index, QItemSelectionModel::Select);
//            }
//        }
//    }
}

void ThumbnailListView::TimeLineSelectAllBtn()
{
//    if (m_delegatetype == ThumbnailDelegate::TimeLineViewType || m_delegatetype == ThumbnailDelegate:: AlbumViewImportTimeLineViewType) {
//        for (int i = 0; i < m_model->rowCount(); i++) {
//            QModelIndex idx = m_model->index(i, 0);
//            DBImgInfo data = idx.data(Qt::DisplayRole).value<DBImgInfo>();
//            if (data.itemType == ItemTypeTimeLineTitle) {
//                TimeLineDateWidget *w = static_cast<TimeLineDateWidget *>(this->indexWidget(idx));
//                if (w) {
//                    w->onTimeLinePicSelectAll(true);
//                }
//            } else if (data.itemType == ItemTypeImportTimeLineTitle) {
//                importTimeLineDateWidget *w = static_cast<importTimeLineDateWidget *>(this->indexWidget(idx));
//                if (w) {
//                    w->onTimeLinePicSelectAll(true);
//                }
//            }
//        }
//    }
}

void ThumbnailListView::setBatchOperateWidget(BatchOperateWidget *widget)
{
    m_batchOperateWidget = widget;
}

void ThumbnailListView::navigateToMonth(const QString &month)
{
    QString tmpdate = month;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
        //循环找到时间
        if (data.itemType == ItemTypePic || data.itemType == ItemTypeVideo || data.itemType == ItemTypeMountImg) {
            continue;
        }
        //时间线
        if (m_delegatetype == ThumbnailDelegate::TimeLineViewType && (data.itemType == ItemTypeTimeLineTitle || data.itemType == ItemTypeBlank)
            && data.date.contains(tmpdate)) {
            scrollTo(index, QAbstractItemView::PositionAtTop);

            break;
        } else if (m_delegatetype == ThumbnailDelegate::AlbumViewImportTimeLineViewType && (data.itemType == ItemTypeImportTimeLineTitle || data.itemType == ItemTypeBlank)
                   && data.date == tmpdate) { //已导入时间线
            scrollTo(index, QAbstractItemView::PositionAtTop);
        }
    }
}

void ThumbnailListView::slotLoadFirstPageThumbnailsFinish()
{
    // 将缩略图信息插入到listview中
    //通知所有照片界面刷新，如果没有图片则显示导入界面
    emit sigDBImageLoaded();
    //加空白栏
//    if (m_delegatetype == ThumbnailDelegate::AllPicViewType) {
//        insertBlankOrTitleItem(ItemTypeBlank, "", "", AllPicView::SUSPENSION_WIDGET_HEIGHT);
//    }
    //过滤不存在图片后重新加载
    //ImageEngineApi::instance()->reloadAfterFilterUnExistImage();
}

void ThumbnailListView::slotOneImgReady(const QString &path, QPixmap pix)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if (data.filePath == path) {
            data.image = pix;
            cutPixmap(data);
            QVariant meta;
            meta.setValue(data);
//            ImageEngineApi::instance()->m_AllImageData[data.filePath].imgpixmap = pix;
            ImageDataService::instance()->addImage(data.filePath, data.image.toImage());
            m_model->setData(index, meta, Qt::DisplayRole);
            break;
        }
    }
}

void ThumbnailListView::onScrollTimerOut()
{
//    disconnect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &ThumbnailListView::onScrollbarValueChanged);
//    QPropertyAnimation *animation = new QPropertyAnimation(verticalScrollBar(), "value");
//    int value = verticalScrollBar()->value();
//    animation->setDuration(ANIMATION_DRLAY);
//    animation->setEasingCurve(QEasingCurve::InOutQuad);//OutCubic  InOutQuad
//    animation->setStartValue(value);
//    animation->setEndValue(value + 10);
//    connect(animation, &QPropertyAnimation::finished,
//            animation, &QPropertyAnimation::deleteLater);
//    connect(animation, &QPropertyAnimation::finished, this, [ = ]() {
//        connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &ThumbnailListView::onScrollbarValueChanged);
//    });
//    animation->start();

    disconnect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &ThumbnailListView::onScrollbarValueChanged);
    m_animation->start();
    connect(m_animation, &QPropertyAnimation::finished, this, [ = ]() {
        connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &ThumbnailListView::onScrollbarValueChanged);
    });


}

void ThumbnailListView::onScrollbarValueChanged(int value)
{
    Q_UNUSED(value)
    //滚动条向下滑动
    if (m_animation && m_animation->state() == QPropertyAnimation::Running) {
        m_animation->stop();
    }
    if (m_scrollTimer->isActive()) {
        m_scrollTimer->stop();
    }

    flushTopTimeLine(8); //滑完了要刷新顶部
}

void ThumbnailListView::flushTopTimeLine(int offset)
{
    QModelIndex index = this->indexAt(QPoint(80, m_blankItemHeight + offset)); //额外下移x个像素点，避免在筛选状态的时候选择错误
    DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();

    //无选中照片，直接返回
    QModelIndexList list = selectionModel()->selectedIndexes();
    if (list.isEmpty()) {
        emit sigTimeLineDataAndNum(data.date, data.num, QObject::tr("Select"));
        return;
    }

    //先判断当前的类型，是标题则获取是否全选；是图片，则找到它所在时间线的数量和时间以及是否全选
    if (data.itemType == ItemTypeBlank || data.itemType == ItemTypeTimeLineTitle
            || data.itemType == ItemTypeImportTimeLineTitle) {
        bool isSelect = getCurrentIndexSelectStatus(index, false);
        emit sigTimeLineDataAndNum(data.date, data.num, isSelect ? QObject::tr("Unselect") : QObject::tr("Select"));
    } else {
        bool isSelect = getCurrentIndexSelectStatus(index, true);
        QStringList currentIndexTimeList = getCurrentIndexTime(index);
        if (currentIndexTimeList.size() == 2) {
            emit sigTimeLineDataAndNum(currentIndexTimeList.at(0), currentIndexTimeList.at(1), isSelect ? QObject::tr("Unselect") : QObject::tr("Select"));
        }
    }
}

void ThumbnailListView::onDoubleClicked(const QModelIndex &index)
{
    if (ALBUM_PATHTYPE_BY_PHONE != m_imageType && m_imageType.compare(COMMON_STR_TRASH) != 0) {
        DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
        if (data.itemType == ItemTypeBlank
            || data.itemType == ItemTypeTimeLineTitle
            || data.itemType == ItemTypeImportTimeLineTitle) {
            return;
        }

        GlobalStatus::instance()->sigDoubleClickedFromQWidget(QUrl::fromLocalFile(data.filePath).toString());
    }
}

void ThumbnailListView::onClicked(const QModelIndex &index)
{
    if (m_isSelectAllBtn) {
        return;
    }
    DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
    qDebug() << __FUNCTION__ << "---" << data.filePath;
    qDebug() << __FUNCTION__ << "---" << ImageDataService::instance()->imageIsLoaded(data.filePath, m_imageType == COMMON_STR_TRASH);
    qDebug() << __FUNCTION__ << "---" << ImageDataService::instance()->getMovieDurationStrByPath(data.filePath);
#ifdef tablet_PC
    if (activeClick && ALBUM_PATHTYPE_BY_PHONE != m_imageType) {
        if (m_imageType.compare(COMMON_STR_TRASH) != 0) {
            emit openImage(index.row());
        }
    }
#endif
    Q_UNUSED(index)
}

//裁剪图片尺寸
void ThumbnailListView::cutPixmap(DBImgInfo &DBImgInfo)
{
    int width = DBImgInfo.image.width();
    if (width == 0)
        width = m_iBaseHeight;
    int height = DBImgInfo.image.height();
    if (abs((width - height) * 10 / width) >= 1) {
        QRect rect = DBImgInfo.image.rect();
        int x = rect.x() + width / 2;
        int y = rect.y() + height / 2;
        if (width > height) {
            x = x - height / 2;
            y = 0;
            DBImgInfo.image = DBImgInfo.image.copy(x, y, height, height);
        } else {
            y = y - width / 2;
            x = 0;
            DBImgInfo.image = DBImgInfo.image.copy(x, y, width, width);
        }
    }
}

void ThumbnailListView::timeLimeFloatBtnClicked(const QString &date, bool isSelect)
{
   QString tmpdate = date;
   for (int i = 0; i < m_model->rowCount(); i++) {
       QModelIndex index = m_model->index(i, 0);
       DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
       //循环找到时间
       if (data.itemType == ItemTypePic || data.itemType == ItemTypeVideo || data.itemType == ItemTypeMountImg) {
           continue;
       }
       //时间线
       if (m_delegatetype == ThumbnailDelegate::TimeLineViewType && (data.itemType == ItemTypeTimeLineTitle || data.itemType == ItemTypeBlank)
               && data.date == tmpdate) {
           //先同步设置本时间线选择按钮状态
           TimeLineDateWidget *w = static_cast<TimeLineDateWidget *>(this->indexWidget(index));
           if (w) {
               w->onTimeLinePicSelectAll(isSelect);
           }
           //取选中范围
           bool bFirstSelect = true;
           QModelIndex first_idx, end_idx;
           for (int j = i + 1; j < m_model->rowCount(); j++) {
               index = m_model->index(j, 0);
               DBImgInfo pdata = index.data(Qt::DisplayRole).value<DBImgInfo>();
               if (pdata.itemType  == ItemTypeTimeLineTitle) {
                   break;//到达下一个时间线，跳出
               }
               if (pdata.itemType  == ItemTypePic || pdata.itemType == ItemTypeVideo) {
                   if (bFirstSelect) {
                       first_idx = index;
                       bFirstSelect = false;
                   }
                   end_idx = index;
               }
           }
           QItemSelection selection(first_idx, end_idx);
           if (isSelect) {//设置选中还是取消选中
               selectionModel()->select(selection, QItemSelectionModel::Select);
           } else {
               selectionModel()->select(selection, QItemSelectionModel::Deselect);
           }
           break;
       } else if (m_delegatetype == ThumbnailDelegate::AlbumViewImportTimeLineViewType && (data.itemType == ItemTypeImportTimeLineTitle || data.itemType == ItemTypeBlank)
                  && data.date == tmpdate) { //已导入时间线
           //先同步设置本时间线选择按钮状态
           importTimeLineDateWidget *w = static_cast<importTimeLineDateWidget *>(this->indexWidget(index));
           if (w) {
               w->onTimeLinePicSelectAll(isSelect);
           }
           //取选中范围
           bool bFirstSelect = true;
           QModelIndex first_idx, end_idx;
           for (int j = i + 1; j < m_model->rowCount(); j++) {
               index = m_model->index(j, 0);
               DBImgInfo pdata = index.data(Qt::DisplayRole).value<DBImgInfo>();
               if (pdata.itemType  == ItemTypeImportTimeLineTitle) {
                   break;//到达下一个时间线，跳出
               } else if (pdata.itemType  == ItemTypePic || pdata.itemType == ItemTypeVideo) {
                   if (bFirstSelect) {
                       first_idx = index;
                       bFirstSelect = false;
                   }
                   end_idx = index;
               }
           }
           QItemSelection selection(first_idx, end_idx);
           if (isSelect) {//设置选中还是取消选中
               selectionModel()->select(selection, QItemSelectionModel::Select);
           } else {
               selectionModel()->select(selection, QItemSelectionModel::Deselect);
           }
           break;//已完成，跳出循环，后面index的不需要处理
       }
   }
}

void ThumbnailListView::resizeEventF()
{
    int i_totalwidth = width() - RIGHT_MARGIN;
    //计算一行的个数
    m_rowSizeHint = i_totalwidth / m_iBaseHeight;
    int currentwidth = (i_totalwidth - ITEM_SPACING * (m_rowSizeHint - 1)) / m_rowSizeHint;//一张图的宽度
    m_onePicWidth = currentwidth;

    if (m_delegate) {
        m_delegate->setItemSize(QSize(m_onePicWidth, m_onePicWidth));
    }

    this->setSpacing(ITEM_SPACING);
}
