/*
 * Peony-Qt
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "explorer-desktop-application.h"
#include "desktop-icon-view.h"

#include "icon-view-style.h"
#include "desktop-icon-view-delegate.h"

#include "desktop-item-model.h"
#include "desktop-item-proxy-model.h"

#include "file-operation-manager.h"
#include "file-move-operation.h"
#include "file-copy-operation.h"
#include "file-trash-operation.h"
#include "clipboard-utils.h"

#include "properties-window.h"
#include "file-utils.h"
#include "file-operation-utils.h"

#include "desktop-menu.h"

#include "file-item-model.h"
#include "file-info-job.h"
#include "file-launch-manager.h"
#include <QProcess>

#include <QDesktopServices>

#include "desktop-index-widget.h"

#include "file-meta-info.h"

#include "global-settings.h"
#include "audio-play-manager.h"

#include <QAction>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

#include <QHoverEvent>

#include <QWheelEvent>
#include <QApplication>

#include <QStringList>
#include <QMessageBox>
#include <QDir>

#include <QDebug>
#include <QToolTip>
#include <QGSettings>

#include <QDrag>
#include <QMimeData>
#include <QPixmap>
#include <QPainter>
#include <QClipboard>

#include <QtX11Extras/QX11Info>
#include <kstartupinfo.h>

using namespace Peony;
#define MINGRIDSIZE 64
#define MAXGRIDSIZE 200
#define ITEM_POS_ATTRIBUTE "metadata::explorer-qt-desktop-item-position"
#define PANEL_SETTINGS "org.lingmo.panel.settings"
#define LINGMO_STYLE_SETTINGS "org.lingmo.style"
#define RESTORE_ITEM_POS_ATTRIBUTE "metadata::explorer-qt-desktop-restore-item-position"
#define RESTORE_EXTEND_ITEM_POS_ATTRIBUTE "metadata::explorer-qt-desktop-restore-extend-item-position"
#define RESTORE_SINGLESCREEN_ITEM_POS_ATTRIBUTE "metadata::explorer-qt-desktop-restore-singlescreen-item-position"

static bool iconSizeLessThan (const QPair<QRect, QString> &p1, const QPair<QRect, QString> &p2);
static bool posLessThan(const int& p1, const int& p2);

static bool refreshing = false;
static bool g_isHighVersion = false;
//static bool g_initialized = false;

int getGreatestCommonDivisor(QList<int> &position)
{
    std::stable_sort(position.begin(), position.end(), posLessThan);
    QList<int> spacingList;
    for (int i = 0; i < position.size() - 1; i++) {
        int spacing = position[i+1] - position[i];
        if (spacing < MINGRIDSIZE &&  0 < spacing) {
            if(0 < spacingList.count()) {
                spacingList.pop_back();
            }
            i++;
            continue;
        }
        if (!spacingList.contains(spacing) && 0 != spacing) {
            spacingList.append(spacing);
        }
    }
    int gridWidth = spacingList.size() > 0 ? spacingList[0] : 0;
    for (int i = 0; i < spacingList.size() - 1; i++) {
        int num1 = spacingList[i];
        int num2 = spacingList[i+1];
        int temp = 0;
        if (num1 < num2) {
            temp = num1;
            num1 = num2;
            num2 = temp;
        }
        while(num2 != 0) {
            temp = num1%num2;
            num1 = num2;
            num2 = temp;
        }
        if (num1 > MINGRIDSIZE && num1 < gridWidth) {
            gridWidth = num1;
        }
    }
    return gridWidth;
}

DesktopIconView::DesktopIconView(QWidget *parent) : QListView(parent)
{
    //m_refresh_timer.setInterval(500);
    //m_refresh_timer.setSingleShot(true);
    QString localeName = QLocale::system().name();
    if (localeName.contains("ug") || localeName.contains("kk") || localeName.contains("ky")) {
         setLayoutDirection(Qt::RightToLeft);
    }

    setAttribute(Qt::WA_AlwaysStackOnTop);

    installEventFilter(this);

    initShoutCut();
    //initMenu();
    initDoubleClick();

    connect(qApp, &QApplication::paletteChanged, this, [=]() {
        viewport()->update();
    });

    auto globalSettings = Peony::GlobalSettings::getInstance();
    QString widgetThemeName = globalSettings->getValue("widgetThemeName").toString();
    if (widgetThemeName.contains("classical")) {
        m_radius = 0;
    } else {
        m_radius = 6;
    }
    connect(globalSettings, &GlobalSettings::valueChanged, this, [=](const QString &key){
        if (key == "widgetThemeName") {
            QString widgetThemeName = globalSettings->getValue("widgetThemeName").toString();
            if (widgetThemeName.contains("classical")) {
                m_radius = 0;
            } else {
                m_radius = 6;
            }
            viewport()->update();
        }
    });

    m_edit_trigger_timer.setSingleShot(true);
    m_edit_trigger_timer.setInterval(3000);
    m_last_index = QModelIndex();

    setAttribute(Qt::WA_TranslucentBackground);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //fix rubberband style.
    setStyle(DirectoryView::IconViewStyle::getStyle());

    setItemDelegate(new DesktopIconViewDelegate(this));

    setDefaultDropAction(Qt::MoveAction);

    setSelectionMode(QListView::ExtendedSelection);
    setEditTriggers(QListView::NoEditTriggers);
    setViewMode(QListView::IconMode);
    setMovement(QListView::Snap);
    setFlow(QListView::TopToBottom);
    setResizeMode(QListView::Adjust);
    setWordWrap(true);

    setDragDropMode(QListView::DragDrop);

    setSelectionMode(QListView::ExtendedSelection);

    auto zoomLevel = this->zoomLevel();
    setDefaultZoomLevel(zoomLevel);

    //task#74174 修改model为全局的
    m_model = PeonyDesktopApplication::getModel();
    m_proxy_model = new DesktopItemProxyModel(m_model);

    m_proxy_model->setSourceModel(m_model);
    m_proxy_model->setId(m_id);
    QString version = qVersion();
    if (version >= QString("5.14.0")) {
        g_isHighVersion = true;
    } else {
        g_isHighVersion = false;
    }
    qDebug() << "qt version: " << version <<  g_isHighVersion;
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, [=](){
        for (auto uri : getAllFileUris()) {
            auto pos = getFileMetaInfoPos(uri);
            if (pos.x() >= 0)
                updateItemPosByUri(uri, pos);
        }
    });

    connect(m_proxy_model, &QAbstractItemModel::rowsRemoved, this, [=](){
        auto itemsNeedRelayout = m_model->m_items_need_relayout;
        if (!itemsNeedRelayout.isEmpty()) {
            this->relayoutExsitingItems(itemsNeedRelayout);
        }

        for (auto uri : getAllFileUris()) {
            auto pos = getFileMetaInfoPos(uri);
            if (pos.x() >= 0)
                updateItemPosByUri(uri, pos);
        }
    });


    //task#74174 扩展模式下支持拖拽图标放置到扩展屏,拖拽后需要通过设置的id重新过滤
    connect(this, &DesktopIconView::updateView, this, [=]() {
        m_proxy_model->invalidateModel();
        for (auto uri : getAllFileUris()) {
            auto pos = getFileMetaInfoPos(uri);
            if (pos.x() >= 0) {
                updateItemPosByUri(uri, pos);
            } else {
                ensureItemPosByUri(uri);
            }
        }
        checkItemsOver();
    });

    connect(m_model, &DesktopItemModel::refreshed, this, [=]() {
        this->setCursor(QCursor(Qt::ArrowCursor));
        refreshing = false;

        // check if there are items overlapped.
        QTimer::singleShot(150, this, [=](){
            initViewport();
            checkItemsOver();

            // check icon is out of screen
            auto geo = viewport()->rect();
            if (geo.width() != 0 && geo.height() != 0) {
                for (auto rec : m_item_rect_hash.values()) {
                    if (!geo.contains(rec)) {
                        resolutionChange();
                        break;
                    }
                }
            }
            auto app = static_cast<PeonyDesktopApplication *>(qApp);
            Q_EMIT app->emitFinish();
            qInfo()<<"desktop finish";

        });

        return;
    });

    connect(m_model, &DesktopItemModel::requestClearIndexWidget, this, &DesktopIconView::clearAllIndexWidgets);

    connect(m_proxy_model, &QSortFilterProxyModel::layoutChanged, this, [=]() {
        //qDebug()<<"layout changed=========================\n\n\n\n\n";
        if (m_proxy_model->getSortType() == DesktopItemProxyModel::Other) {
            return;
        }
        if (m_proxy_model->sortColumn() != 0) {
            return;
        }
        //qDebug()<<"save====================================";

        QTimer::singleShot(100, this, [=]() {
            //this->saveAllItemPosistionInfos();
        });
    });

    connect(m_proxy_model, &DesktopItemProxyModel::showHiddenFile, this, [=]() {
//        QTimer::singleShot(100, this, [=]() {
//            resetAllItemPositionInfos();
//            refresh();
//        });
        //fix#181595 桌面图标设置隐藏后排序
        Q_EMIT updateView();
    });

    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (SHOW_HIDDEN_PREFERENCE == key) {
            m_show_hidden= GlobalSettings::getInstance()->getValue(key).toBool();
        }
    });

    connect(this, &QListView::iconSizeChanged, this, [=]() {
        //qDebug()<<"save=============";
        this->setSortType(GlobalSettings::getInstance()->getValue(LAST_DESKTOP_SORT_ORDER).toInt());

        QTimer::singleShot(100, this, [=]() {
            bool isFull = false;
            auto geo = viewport()->rect();
            this->saveAllItemPosistionInfos();
            for (int i = 0; i < m_proxy_model->rowCount(); i++) {
                auto index = m_proxy_model->index(i, 0);
                m_item_rect_hash.insert(index.data(Qt::UserRole).toString(), visualRectInRightToLeft(index));
                updateItemPosByUri(index.data(Qt::UserRole).toString(), visualRectInRightToLeft(index).topLeft());

                if (geo.width() != 0 && geo.height() != 0) {
                    QRect itemRect = visualRectInRightToLeft(index);
                    if (verifyBoundaries(itemRect, Direction::All)) {
                        isFull = true;
                    }
                }
            }
            if (isFull) resolutionChange();
        });
    });

    setModel(m_proxy_model);
    //m_proxy_model->sort(0);

    m_explorerDbusSer = new PeonyDbusService(this);
    m_explorerDbusSer->DbusServerRegister();

    setMouseTracking(true);//追踪鼠标

    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString &key) {
        if (key == DISPLAY_STANDARD_ICONS ||
            key == HOME_ICON_VISIBLE ||
            key == TRASH_ICON_VISIBLE ||
            key == COMPUTER_ICON_VISIBLE) {
            //this->refresh();
            m_proxy_model->invalidate();
            this->resolutionChange();
            if (isItemsOverlapped()) {
                // refresh again?
                this->refresh();
                QStringList needRelayoutItems;
                QRegion notEmptyRegion;
                for (auto value : m_item_rect_hash.values()) {
                    auto keys = m_item_rect_hash.keys(value);
                    if (keys.count() > 1) {
                        keys.pop_front();
                        for (auto key : keys) {
                            needRelayoutItems.append(key);
                            m_item_rect_hash.remove(key);
                        }
                    }
                    notEmptyRegion += value;
                }

                int gridWidth = gridSize().width();
                int gridHeight = gridSize().height();
                // aligin exsited rect
                int marginTop = notEmptyRegion.boundingRect().top();
                while (marginTop - gridHeight > 0) {
                    marginTop -= gridHeight;
                }
                int marginLeft = notEmptyRegion.boundingRect().left();
                while (marginLeft - gridWidth > 0) {
                    marginLeft -= gridWidth;
                }
                marginLeft = marginLeft < 0? 0: marginLeft;
                marginTop = marginTop < 0? 0: marginTop;
                int posX = marginLeft;
                int posY = marginTop;
                for (auto item : needRelayoutItems) {
                    QRect itemRect = QRect(posX, posY, gridWidth, gridHeight);
                    while (notEmptyRegion.contains(itemRect.center())) {
                        if (posY + 2*gridHeight > this->viewport()->height()) {
                            posY = marginTop;
                            posX += gridWidth;
                        } else {
                            posY += gridHeight;
                        }
                        if (this->viewport()->geometry().contains(itemRect.topLeft())) {
                            itemRect.moveTo(posX, posY);
                        } else {
                            itemRect.moveTo(0, 0);
                        }
                    }
                    notEmptyRegion += itemRect;
                    m_item_rect_hash.insert(item, itemRect);
                }
                for (auto uri : m_item_rect_hash.keys()) {
                    auto rect = m_item_rect_hash.value(uri);
                    updateItemPosByUri(uri, rect.topLeft());
                    setFileMetaInfoPos(uri, rect.topLeft());
                }
                this->saveAllItemPosistionInfos();
            }
        }
    });

    //fix task bar overlap with desktop icon and can drag move issue
    //bug #27811,33188
    if (QGSettings::isSchemaInstalled(PANEL_SETTINGS))
    {
        //panel monitor
        if (!m_panelSetting)
            m_panelSetting = new QGSettings(PANEL_SETTINGS, QByteArray(), this);
        int position = m_panelSetting->get("panelposition").toInt();
        int margins = m_panelSetting->get("panelsize").toInt();
        connect(m_panelSetting, &QGSettings::changed, this, [=](const QString &key){
            if (key == "panelposition" || key == "panelsize") {
                int position = m_panelSetting->get("panelposition").toInt();
                int margins = m_panelSetting->get("panelsize").toInt();
                setMarginsBasedOnPosition(position,  margins);
                if (m_initialized)
                    resolutionChange();
            }
        });
        setMarginsBasedOnPosition(position, margins);
    }

    // try fixing #63358
    if (QGSettings::isSchemaInstalled(LINGMO_STYLE_SETTINGS)) {
        auto styleSettings = new QGSettings(LINGMO_STYLE_SETTINGS, QByteArray(), this);
        connect(styleSettings, &QGSettings::changed, this, [=](const QString &key){
            if (key == "iconThemeName") {
                QTimer::singleShot(1000, viewport(), [=]{
                    viewport()->update();
                });
            }
        });
    }
}

void DesktopIconView::setMarginsBasedOnPosition(int position, int margins)
{
    switch (position) {
    case 1: {
        setViewportMargins(0, margins, 0, 0);
        m_panel_margin = QMargins(0, margins, 0, 0);
        break;
    }
    case 2: {
        setViewportMargins(margins, 0, 0, 0);
        m_panel_margin = QMargins(margins, 0, 0, 0);
        break;
    }
    case 3: {
        setViewportMargins(0, 0, margins, 0);
        m_panel_margin = QMargins(0, 0, margins, 0);
        break;
    }
    default: {
        setViewportMargins(0, 0, 0, margins);
        m_panel_margin = QMargins(0, 0, 0, margins);
        break;
    }
    }
}

void DesktopIconView::setId(int id)
{
    m_proxy_model->setId(id);
    m_id = id ;
}

DesktopIconView::~DesktopIconView()
{
    delete m_explorerDbusSer;
    //saveAllItemPosistionInfos();
}

bool DesktopIconView::eventFilter(QObject *obj, QEvent *e)
{
    //fixme:
    //comment to fix change night style refresh desktop issue
    if (e->type() == QEvent::StyleChange || e->type() == QEvent::ApplicationFontChange || e->type() == QEvent::FontChange) {
        auto type = e->type();
        if (m_proxy_model) {
            for (auto uri : getAllFileUris()) {
                auto pos = getFileMetaInfoPos(uri);
                if (pos.x() >= 0)
                    updateItemPosByUri(uri, pos);
            }
            QTimer::singleShot(0, this, [=]{
                for (auto uri : getAllFileUris()) {
                    auto pos = getFileMetaInfoPos(uri);
                    if (pos.x() >= 0)
                        updateItemPosByUri(uri, pos);
                }
            });
        }
    }
    return false;
}

static bool meetSpecialConditions(const QStringList& selectedUris)
{
    /* The desktop home directory, computer, and trash do not allow operations such as copying, cutting,
     * deleting, renaming, moving, or using shortcut keys for corresponding operations.add by 2021/06/17 */
    static QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    if (selectedUris.contains("computer:///")
       ||selectedUris.contains("trash:///")
       ||selectedUris.contains(homeUri)){
        return true;
    }

    return false;
}

void DesktopIconView::initShoutCut()
{
    QAction *copyAction = new QAction(this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, [=]() {
        auto selectedUris = this->getSelections();
        if (!selectedUris.isEmpty() && !meetSpecialConditions(selectedUris)){
            ClipboardUtils::setClipboardFiles(selectedUris, false);
            this->viewport()->update();
        }
    });
    addAction(copyAction);

    QAction *cutAction = new QAction(this);
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, [=]() {
        auto selectedUris = this->getSelections();
        if (!selectedUris.isEmpty() && !meetSpecialConditions(selectedUris))
        {
            ClipboardUtils::setClipboardFiles(selectedUris, true);
            //this->update();
            this->viewport()->update();
        }
    });
    addAction(cutAction);

    QAction *pasteAction = new QAction(this);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, [=]() {
        if (qApp->clipboard()->mimeData()->hasFormat ("uos/remote-copy")) {
            auto op = ClipboardUtils::pasteClipboardFiles(this->getDirectoryUri());
            if (!op) {
                viewport()->update();
            }
        } else {
            //auto clipUris = ClipboardUtils::getClipboardFilesUris();
            if (ClipboardUtils::getInstance()->isClipboardHasFiles() && !meetSpecialConditions(this->getSelections())) {
                auto op = ClipboardUtils::pasteClipboardFiles(this->getDirectoryUri());
                if (!op) {
                    viewport()->update();
                }
            }
        }
    });
    addAction(pasteAction);

    //add CTRL+D for delete operation
    auto trashAction = new QAction(this);
    trashAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Delete<<QKeySequence(Qt::CTRL + Qt::Key_D));
    connect(trashAction, &QAction::triggered, [=]() {
        auto selectedUris = getSelections();
        if (!selectedUris.isEmpty() && !meetSpecialConditions(selectedUris)){
           FileOperationUtils::trash(selectedUris, true);
        }
    });
    addAction(trashAction);

    QAction *undoAction = new QAction(this);
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered,
    [=]() {
        // do not relayout item with undo.
        setRenaming(true);
        FileOperationManager::getInstance()->undo();
    });
    addAction(undoAction);

    QAction *redoAction = new QAction(this);
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered,
    [=]() {
        // do not relayout item with redo.
        setRenaming(true);
        FileOperationManager::getInstance()->redo();
    });
    addAction(redoAction);

    QAction *zoomInAction = new QAction(this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, [=]() {
        this->zoomIn();
    });
    addAction(zoomInAction);

    QAction *zoomOutAction = new QAction(this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, [=]() {
        this->zoomOut();
    });
    addAction(zoomOutAction);

    QAction *renameAction = new QAction(this);
    renameAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_E));
    connect(renameAction, &QAction::triggered, [=]() {
        auto selections = this->getSelections();
        if (selections.count() == 1 && !meetSpecialConditions(selections)) {
            this->editUri(selections.first());
        }
    });
    addAction(renameAction);

    QAction *removeAction = new QAction(this);
    removeAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Delete));
    connect(removeAction, &QAction::triggered, [=]() {
        auto selectedUris = this->getSelections();
        if (!meetSpecialConditions(selectedUris)){
            qDebug() << "delete" << selectedUris;
            clearAllIndexWidgets();
            FileOperationUtils::executeRemoveActionWithDialog(selectedUris);
        }
    });
    addAction(removeAction);

    QAction *helpAction = new QAction(this);
    helpAction->setShortcut(Qt::Key_F1);
    connect(helpAction, &QAction::triggered, this, [=]() {
        PeonyDesktopApplication::showGuide();
    });
    addAction(helpAction);

    auto propertiesWindowAction = new QAction(this);
    propertiesWindowAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::ALT + Qt::Key_Return)
                                         <<QKeySequence(Qt::ALT + Qt::Key_Enter));
    connect(propertiesWindowAction, &QAction::triggered, this, [=]() {
        DesktopMenu menu(this);
        if (this->getSelections().count() > 0)
        {
            menu.showProperties(this->getSelections());
        }
        else
        {
            QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            menu.showProperties(desktopPath);
        }
    });
    addAction(propertiesWindowAction);

    auto newFolderAction = new QAction(this);
    newFolderAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    connect(newFolderAction, &QAction::triggered, this, [=]() {
        CreateTemplateOperation op(this->getDirectoryUri(), CreateTemplateOperation::EmptyFolder, tr("New Folder"));
        op.run();
        auto targetUri = op.target();

        QTimer::singleShot(500, this, [=]() {
            this->scrollToSelection(targetUri);
        });
    });
    addAction(newFolderAction);

    QAction *refreshWinAction = new QAction(this);
    refreshWinAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    connect(refreshWinAction, &QAction::triggered, [=]() {
        this->refresh();
    });
    addAction(refreshWinAction);

    QAction *reverseSelectAction = new QAction(this);
    reverseSelectAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
    connect(reverseSelectAction, &QAction::triggered, [=]() {
        this->invertSelections();
    });
    addAction(reverseSelectAction);

    QAction *normalIconAction = new QAction(this);
    normalIconAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
    connect(normalIconAction, &QAction::triggered, [=]() {
        if (this->zoomLevel() == DesktopIconView::Normal)
            return;
        this->setDefaultZoomLevel(DesktopIconView::Normal);
    });
    addAction(normalIconAction);

    auto refreshAction = new QAction(this);
    refreshAction->setShortcut(Qt::Key_F5);
    connect(refreshAction, &QAction::triggered, this, [=]() {
        this->refresh();
    });
    addAction(refreshAction);

    QAction *editAction = new QAction(this);
    editAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::ALT + Qt::Key_E)<<Qt::Key_F2);
    connect(editAction, &QAction::triggered, this, [=]() {
        auto selections = this->getSelections();
        if (selections.count() == 1) {
            this->editUri(selections.first());
        } else if (selections.count() > 1) {
            this->editUris(selections);
        }
    });
    addAction(editAction);

    auto settings = GlobalSettings::getInstance();
    m_show_hidden = settings->isExist(SHOW_HIDDEN_PREFERENCE)? settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool(): false;
    //show hidden action
    QAction *showHiddenAction = new QAction(this);
    showHiddenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    addAction(showHiddenAction);
    connect(showHiddenAction, &QAction::triggered, this, [=]() {
        this->setShowHidden();
    });

    auto cancelAction = new QAction(this);
    cancelAction->setShortcut(Qt::Key_Escape);
    connect(cancelAction, &QAction::triggered, [=]() {
        if (Peony::ClipboardUtils::isClipboardHasFiles())
        {
            Peony::ClipboardUtils::clearClipboard();
            this->update();
        }
    });
    addAction(cancelAction);
}

void DesktopIconView::initMenu()
{
    return;
    setContextMenuPolicy(Qt::CustomContextMenu);

    // menu
    connect(this, &QListView::customContextMenuRequested, this,
    [=](const QPoint &pos) {
        // FIXME: use other menu
        qDebug() << "menu request  in desktop icon view";
        if (!this->indexAt(pos).isValid()) {
            this->clearSelection();
        } else {
            this->clearSelection();
            this->selectionModel()->select(this->indexAt(pos), QItemSelectionModel::Select);
        }

        QTimer::singleShot(1, [=]() {
            DesktopMenu menu(this);
            if (this->getSelections().isEmpty()) {
                auto action = menu.addAction(tr("Set Background"));
                connect(action, &QAction::triggered, [=]() {
                    //go to control center set background
                    PeonyDesktopApplication::gotoSetBackground();
                });
            }
            menu.exec(QCursor::pos());
            auto urisToEdit = menu.urisToEdit();
            if (urisToEdit.count() == 1) {
                QTimer::singleShot(
                100, this, [=]() {
                    this->editUri(urisToEdit.first());
                });
            }
        });
    }, Qt::UniqueConnection);
}

void DesktopIconView::setShowHidden()
{
    m_show_hidden = !GlobalSettings::getInstance()->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
    m_proxy_model->setShowHidden(m_show_hidden);
    //fix show hidden file desktop icons overlapped issue
    //QTimer::singleShot(100, this, [=]() {
        //resetAllItemPositionInfos();
        //refresh();
    //});
    //fix#181595 桌面图标设置隐藏后排序
    Q_EMIT updateView();
}

void DesktopIconView::resolutionChange()
{
    qInfo()<<"start resolutionChange()";
    QSize screenSize = this->viewport()->size();

    // do not relayout items while screen size is empty.
    if (screenSize.isEmpty()) {
        qWarning()<<"screen size is not available";
        return;
    }
    if (m_item_rect_hash.isEmpty()) {
        return;
    }
    float iconWidth = 0;
    float iconHeigth = 0;

    // icon size
    QSize icon = gridSize();
    iconWidth = icon.width();
    iconHeigth = icon.height();

    QRect screenRect = QRect(0, 0, screenSize.width(), screenSize.height());

    qInfo()<<"screen rect:"<<screenRect;

    if (!m_item_rect_hash.isEmpty()) {
        QList<QPair<QRect, QString>> newPosition;

        for (auto i = m_item_rect_hash.constBegin(); i != m_item_rect_hash.constEnd(); ++i) {
            newPosition << QPair<QRect, QString>(i.value(), i.key());
        }

        // not get current size
        if (iconWidth == 0 || iconHeigth == 0) {
            qDebug() << "Unable to get icon size, need to get it another way!";
            return;
        }

    //    qDebug() << "icon width: " << iconWidth << " icon heigth: " << iconHeigth;
    //    qDebug() << "width:" << screenSize.width() << " height:" << screenSize.height();

        std::stable_sort(newPosition.begin(), newPosition.end(), iconSizeLessThan);

        //m_item_rect_hash.clear();

        // only reset items over viewport.
        QRegion notEmptyRegion;
        QList<QPair<QRect, QString>> needChanged;
        QSize size;
        for (auto pair : newPosition) {
            QRect itemRect = pair.first;
            //itemRect.moveTo(itemRect.topLeft() + offset());
            if (verifyBoundaries(itemRect, Direction::All)) {
                needChanged.append(pair);
                if (!m_resolution_item_rect.contains(pair.second)) {
                    // remember item position before resolution changed.
                    m_resolution_item_rect.insert(pair.second, m_item_rect_hash.value(pair.second));
                    //m_item_rect_hash.remove(pair.second);
                }
            } else {
                QModelIndex srcIndex = m_model->indexFromUri(pair.second);
                QModelIndex index = m_proxy_model->mapFromSource(srcIndex);
                QRect rect = getDataRect(index);
                rect.moveTo(m_item_rect_hash.value(pair.second).topLeft());
                size = rect.size();
                notEmptyRegion += rect;
            }
        }

        qInfo()<<"need changed item"<<needChanged;


        // aligin exsited rect
        int marginTop = notEmptyRegion.boundingRect().top();
        while (marginTop - iconHeigth >= 0) {
            marginTop -= iconHeigth;
        }
        int marginLeft = notEmptyRegion.boundingRect().left();
        while (marginLeft - iconWidth >= 0) {
            marginLeft -= iconWidth;
        }
        marginTop = marginTop <= 0? offset().y(): marginTop;
        marginLeft = marginLeft < 0? 0: marginLeft;

        if (!needChanged.isEmpty()) {
            qInfo()<<"屏幕过小，有元素超过屏幕范围";
            int posX = marginLeft;
            int posY = marginTop;

            for (int i = 0; i < needChanged.count(); i++) {
                QSize iconSize = m_item_rect_hash.value(needChanged.at(i).second).size();
                while (notEmptyRegion.intersects(QRect(posX, posY, iconWidth, iconHeigth))) {
                    QRect tmpRect = QRect(QPoint(posX, posY+iconHeigth), iconSize);
                    if (verifyBoundaries(tmpRect, Direction::Bottom)) {
                        posY = marginTop;
                        posX += iconWidth;
                    } else {
                        posY += iconHeigth;
                    }
                }
                QRect newRect = QRect(QPoint(posX, posY), iconSize);
                if (verifyBoundaries(newRect, Direction::Right)) {
                    newRect.moveTo(0, 0);
                }
                m_item_rect_hash.insert(needChanged.at(i).second, newRect);
                newRect.setSize(size);
                notEmptyRegion += newRect;
            }
        } else {
            qInfo()<<"没有元素超过屏幕范围";
            QStringList itemNeedBeRelayout;

            qInfo()<<"尝试恢复超过屏幕范围的元素" <<m_resolution_item_rect;
            QMap<QString, QRect> destoryItemMap;
            for (auto uri : m_resolution_item_rect.keys()) {
                m_storageBox.removeOne(uri);
                auto originalRect = m_resolution_item_rect.value(uri);
                //优先恢复主屏的图标
                if (m_model->m_destoryItems.contains(uri)) {
                    destoryItemMap.insert(uri, originalRect);
                    continue;
                }
                //QRect itemRect = originalRect;
                if (!verifyBoundaries(originalRect, Direction::All)) {
                    m_item_rect_hash.insert(uri, originalRect);
                    m_resolution_item_rect.remove(uri);
                } else {
                    m_item_rect_hash.remove(uri);
                    itemNeedBeRelayout<<uri;
                }
            }

            for (auto uri : destoryItemMap.keys()) {
                auto rect = destoryItemMap.value(uri);
                if (verifyBoundaries(rect, Direction::All)) {
                    m_item_rect_hash.insert(uri, rect);
                    m_resolution_item_rect.remove(uri);
                } else {
                    m_item_rect_hash.remove(uri);
                    itemNeedBeRelayout<<uri;
                }
            }
            qInfo()<<"重排需要更新位置的元素";
            itemNeedBeRelayout.append(m_storageBox);
            m_storageBox.clear();
            relayoutExsitingItems(itemNeedBeRelayout);

            // re-layout overlayed items
//            for (auto pair : newPosition) {
//                if (QRect(0, 0, 10, 10).contains(pair.first.topLeft())) {
//                    needChanged.append(pair);
//                }
//            }
//            // first item doesn't need re-layout
//            if (!needChanged.isEmpty())
//                needChanged.removeFirst();

//            // 重新计算非空区域
//            notEmptyRegion = QRegion();
//            for (auto pair : needChanged) {
//                m_item_rect_hash.remove(pair.second);
//            }
//            for (auto rect : m_item_rect_hash.values()) {
//                notEmptyRegion += rect;
//            }

//            int posX = marginLeft;
//            int posY = marginTop;
//            for (int i = 0; i < needChanged.count(); i++) {
//                while (notEmptyRegion.contains(QPoint(posX + iconWidth/2, posY + iconHeigth/2))) {
//                    if (posY + iconHeigth * 2 > screenSize.height()) {
//                        posY = marginTop;
//                        posX += iconWidth;
//                    } else {
//                        posY += iconHeigth;
//                    }
//                }
//                QRect newRect = QRect(QPoint(posX, posY), gridSize());
//                if (posX + iconWidth > screenSize.width()) {
//                    newRect.moveTo(0, 0);
//                }
//                notEmptyRegion += newRect;
//                m_item_rect_hash.insert(needChanged.at(i).second, newRect);
//            }

//            // try restore items which's positions changed by resolution changed.
//            QStringList needRelayoutItems2;
//            for (auto uri : m_resolution_item_rect.keys()) {
//                auto rect = m_resolution_item_rect.value(uri);
//                if (screenRect.contains(rect)) {
//                    m_item_rect_hash.insert(uri, rect);
//                } else {
//                    // need be relayout.
//                    needRelayoutItems2<<uri;
//                }
//            }
//            relayoutExsitingItems(needRelayoutItems2);
        }
    }

    for (auto uri : m_item_rect_hash.keys()) {
        auto rect = m_item_rect_hash.value(uri);
        updateItemPosByUri(uri, rect.topLeft());
        setFileMetaInfoPos(uri, rect.topLeft());
    }
    setAllRestoreInfo();
    this->saveAllItemPosistionInfos();
}

void DesktopIconView::openFileByUri(QString uri)
{
    auto info = FileInfo::fromUri(uri);
    auto job = new FileInfoJob(info);
    job->setAutoDelete();
    job->connect(job, &FileInfoJob::queryAsyncFinished, [=]() {
        if ((info->isDir() || info->isVolume() || info->isVirtual())) {
            QDir dir(info->filePath());
            if (! dir.exists())
            {
                Peony::AudioPlayManager::getInstance()->playWarningAudio();
                auto result = QMessageBox::question(nullptr, tr("Open Link failed"),
                                      tr("File not exist, do you want to delete the link file?"),
                                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (result == QMessageBox::Yes) {
                    qDebug() << "Delete unused symbollink in desktop.";
                    QStringList selections;
                    selections.push_back(uri);
                    FileOperationUtils::trash(selections, true);
                }
                return;
            }

            if (! info->uri().startsWith("trash://")
                    && ! info->uri().startsWith("computer://")
                    &&  ! info->canExecute())
            {
                Peony::AudioPlayManager::getInstance()->playWarningAudio();
                QMessageBox::critical(nullptr, tr("Open failed"),
                                      tr("Open directory failed, you have no permission!"));
                return;
            }

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            QProcess p;
            QUrl url = uri;
            p.setProgram("explorer");
            p.setArguments(QStringList() << url.toEncoded() <<"%U&");
            qint64 pid;
            p.startDetached(&pid);

            // send startinfo to kwindowsystem
            quint32 timeStamp = QX11Info::isPlatformX11() ? QX11Info::appUserTime() : 0;
            KStartupInfoId startInfoId;
            startInfoId.initId(KStartupInfo::createNewStartupIdForTimestamp(timeStamp));
            startInfoId.setupStartupEnv();
            KStartupInfoData data;
            data.setHostname();
            data.addPid(pid);
            QRect rect = info.get()->property("iconGeometry").toRect();
#ifdef KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
            if (rect.isValid())
                data.setIconGeometry(rect);
#endif
            data.setLaunchedBy(getpid());
            KStartupInfo::sendStartup(startInfoId, data);
#else
            QProcess p;
            QString strq;
            for (int i = 0;i < uri.length();++i) {
                if(uri[i] == ' '){
                    strq += "%20";
                }else{
                    strq += uri[i];
                }
            }

            p.startDetached("/usr/bin/explorer", QStringList()<<strq<<"%U&");
#endif
        } else {
            if (!(info->isDesktopFile() && execSharedFileLink(uri))) {
                FileLaunchManager::openAsync(uri, false, false);
            }
        }
        this->clearSelection();
    });
    job->queryAsync();
}

void DesktopIconView::initDoubleClick()
{
    connect(this, &QListView::activated, this, [=](const QModelIndex &index) {
        qDebug() << "double click" << index.data(FileItemModel::UriRole);
        auto uri = index.data(FileItemModel::UriRole).toString();
        openFileByUri(uri);
    }, Qt::UniqueConnection);
}

void DesktopIconView::saveAllItemPosistionInfos()
{
    //qDebug()<<"======================save";
    for (int i = 0; i < m_proxy_model->rowCount(); i++) {
        auto index = m_proxy_model->index(i, 0);
        auto indexRect = visualRectInRightToLeft(index);
        QStringList topLeft;
        topLeft<<QString::number(indexRect.top());
        topLeft<<QString::number(indexRect.left());

        auto metaInfo = FileMetaInfo::fromUri(index.data(Qt::UserRole).toString());
        if (metaInfo) {
            //qDebug()<<"save real"<<index.data()<<topLeft;
            metaInfo->setMetaInfoStringList(ITEM_POS_ATTRIBUTE, topLeft);

            QRect rect(mapToGlobal(indexRect.topLeft())*qApp->devicePixelRatio(), indexRect.size()*qApp->devicePixelRatio());
            FileInfo::fromUri(index.data(Qt::UserRole).toString()).get()->setProperty("iconGeometry", rect);
        }
    }
    //qDebug()<<"======================save finished";
}

void DesktopIconView::saveItemPositionInfo(const QString &uri)
{
    return;
}

void DesktopIconView::resetAllItemPositionInfos()
{
    if (!m_proxy_model)
        return;

    clearAllRestoreInfo();
    m_item_rect_hash.clear();
    for (int i = 0; i < m_proxy_model->rowCount(); i++) {
        auto index = m_proxy_model->index(i, 0);
        auto indexRect = visualRectInRightToLeft(index);
        QStringList topLeft;
        topLeft<<QString::number(indexRect.top());
        topLeft<<QString::number(indexRect.left());
        auto metaInfo = FileMetaInfo::fromUri(index.data(Qt::UserRole).toString());
        if (metaInfo) {
            QStringList tmp;
            tmp<<"-1"<<"-1";
            metaInfo->setMetaInfoStringList(ITEM_POS_ATTRIBUTE, tmp);

            FileInfo::fromUri(index.data(Qt::UserRole).toString()).get()->setProperty("iconGeometry", QVariant());
        }
    }
}

void DesktopIconView::resetItemPosistionInfo(const QString &uri)
{
    return;
}

void DesktopIconView::updateItemPosistions(const QString &uri)
{
    return;
}

QPoint DesktopIconView::getFileMetaInfoPos(const QString &uri)
{
    auto value = m_item_rect_hash.value(uri);
    if (!value.isEmpty())
        return value.topLeft();

    auto metaInfo = FileMetaInfo::fromUri(uri);
    if (metaInfo) {
        auto list = metaInfo->getMetaInfoStringList(ITEM_POS_ATTRIBUTE);
        if (!list.isEmpty()) {
            if (list.count() == 2) {
                int top = list.first().toInt();
                int left = list.at(1).toInt();
                if (top >= 0 && left >= 0) {
                    QPoint p(left, top);
                    return p;
                }
            }
        }
    }
    return QPoint(-1, -1);
}

void DesktopIconView::setFileMetaInfoPos(const QString &uri, const QPoint &pos)
{
    auto delegate = qobject_cast<DesktopIconViewDelegate *>(itemDelegate());
    QSize iconSize = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());

    m_item_rect_hash.remove(uri);
    m_item_rect_hash.insert(uri, QRect(pos, iconSize));

    QRect rect(mapToGlobal(pos)*qApp->devicePixelRatio(), iconSize*qApp->devicePixelRatio());
    FileInfo::fromUri(uri).get()->setProperty("iconGeometry", rect);

    auto metaInfo = FileMetaInfo::fromUri(uri);
    if (metaInfo) {
        QStringList topLeft;
        topLeft<<QString::number(pos.y());
        topLeft<<QString::number(pos.x());
        metaInfo->setMetaInfoStringList(ITEM_POS_ATTRIBUTE, topLeft);
        metaInfo->setMetaInfoInt("explorer-qt-desktop-id", m_id);
    }
}

QMap<QString, QRect> DesktopIconView::getCurrentItemRects()
{
    return m_item_rect_hash;
}

int DesktopIconView::removeItemRect(const QString &uri)
{
    return m_item_rect_hash.remove(uri);
}

void DesktopIconView::updateItemPosByUri(const QString &uri, const QPoint &pos)
{
    auto delegate = qobject_cast<DesktopIconViewDelegate *>(itemDelegate());
    QSize iconSize = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());

    auto srcIndex = m_model->indexFromUri(uri);
    auto index = m_proxy_model->mapFromSource(srcIndex);
    if (index.isValid()) {
        setPositionForIndex(pos, index);
        m_item_rect_hash.remove(uri);
        m_item_rect_hash.insert(uri, QRect(pos, iconSize));

        QRect rect(mapToGlobal(pos)*qApp->devicePixelRatio(), iconSize*qApp->devicePixelRatio());
        FileInfo::fromUri(uri).get()->setProperty("iconGeometry", rect);
        //qDebug()<<"DesktopIconView::updateItemPosByUri"<<m_item_rect_hash[uri]<<" uri:"<<uri;
    }
}

void DesktopIconView::ensureItemPosByUri(const QString &uri)
{
    auto srcIndex = m_model->indexFromUri(uri);
    auto index = m_proxy_model->mapFromSource(srcIndex);
    auto rect = visualRectInRightToLeft(index);
    if (index.isValid()) {
        m_item_rect_hash.remove(uri);
        m_item_rect_hash.insert(uri, rect);
        setFileMetaInfoPos(uri, rect.topLeft());
    }
}

const QStringList DesktopIconView::getSelections()
{
    QStringList uris;
    auto indexes = selectionModel()->selection().indexes();
    for (auto index : indexes) {
        uris<<index.data(Qt::UserRole).toString();
    }
    uris.removeDuplicates();
    return uris;
}

const QStringList DesktopIconView::getAllFileUris()
{
    QStringList uris;
    for (int i = 0; i < m_proxy_model->rowCount(); i++) {
        auto index = m_proxy_model->index(i, 0);
        uris<<index.data(Qt::UserRole).toString();
    }
    return uris;
}

const int DesktopIconView::getAllDisplayFileCount()
{
    if(m_proxy_model)
        return m_proxy_model->rowCount();
    return 0;
}

void DesktopIconView::setSelections(const QStringList &uris)
{
    clearSelection();
    for (int i = 0; i < m_proxy_model->rowCount(); i++) {
        auto index = m_proxy_model->index(i, 0);
        if (uris.contains(index.data(Qt::UserRole).toString())) {
            selectionModel()->select(index, QItemSelectionModel::Select);
        }
    }
}

void DesktopIconView::invertSelections()
{
    QItemSelectionModel *selectionModel = this->selectionModel();
    const QItemSelection currentSelection = selectionModel->selection();
    this->selectAll();
    selectionModel->select(currentSelection, QItemSelectionModel::Deselect);
    clearAllIndexWidgets();
}

void DesktopIconView::scrollToSelection(const QString &uri)
{

}

int DesktopIconView::getSortType()
{
    return m_proxy_model->getSortType();
}

void DesktopIconView::setSortType(int sortType)
{
    clearAllRestoreInfo();
    m_item_rect_hash.clear();
    //resetAllItemPositionInfos();
    m_proxy_model->setSortType(sortType);
    m_proxy_model->sort(1);
    m_proxy_model->sort(0, m_proxy_model->sortOrder());
    saveAllItemPosistionInfos();
    bool isFull = false;
    auto geo = viewport()->rect();
    if (geo.width() != 0 && geo.height() != 0) {
        for (int i = 0; i < m_proxy_model->rowCount(); i++) {
            auto index = m_proxy_model->index(i, 0);
            m_item_rect_hash.insert(index.data(Qt::UserRole).toString(), visualRectInRightToLeft(index));
            updateItemPosByUri(index.data(Qt::UserRole).toString(), visualRectInRightToLeft(index).topLeft());
            QRect itemRect = visualRectInRightToLeft(index);
            //itemRect.moveTo(itemRect.topLeft()+offset());
            if (verifyBoundaries(itemRect, Direction::All)) {
                isFull = true;
            }
        }

        if (isFull) {
            resolutionChange();
        }
    }
}

int DesktopIconView::getSortOrder()
{
    return m_proxy_model->sortOrder();
}

void DesktopIconView::setSortOrder(int sortOrder)
{
    m_proxy_model->sort(0, Qt::SortOrder(sortOrder));
}

void DesktopIconView::editUri(const QString &uri)
{
    clearAllIndexWidgets();
    qDebug() << "editUri clearAllIndexWidgets";
    auto origin = FileUtils::getOriginalUri(uri);
    QTimer::singleShot(100, this, [=]() {
        auto index = m_proxy_model->mapFromSource(m_model->indexFromUri(origin));
        edit(index);
        qDebug() << "editUri index:"<<index<<uri;
    });
}

void DesktopIconView::editUris(const QStringList uris)
{
    clearAllIndexWidgets();
    auto origin = FileUtils::getOriginalUri(uris.first());
    QTimer::singleShot(100, this, [=]() {
        edit(m_proxy_model->mapFromSource(m_model->indexFromUri(origin)));
    });
}


void DesktopIconView::UpdateToEditUris(QStringList uris)
{
    m_uris_to_edit = uris;
}

void DesktopIconView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    return;
}

void DesktopIconView::setCutFiles(const QStringList &uris)
{
    ClipboardUtils::setClipboardFiles(uris, true);
    this->viewport()->update();
}

void DesktopIconView::closeView()
{
    deleteLater();
}

void DesktopIconView::wheelEvent(QWheelEvent *e)
{
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        if (e->delta() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
    }
}

void DesktopIconView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Home: {
        auto boundingRect = getBoundingRect();
        QRect homeRect = QRect(boundingRect.topLeft(), this->gridSize());
        while (!indexAt(homeRect.center()).isValid()) {
            homeRect.translate(0, gridSize().height());
        }
        auto homeIndex = indexAt(homeRect.center());
        selectionModel()->select(homeIndex, QItemSelectionModel::SelectCurrent);
        break;
    }
    case Qt::Key_End: {
        auto boundingRect = getBoundingRect();
        QRect endRect = QRect(boundingRect.bottomRight(), this->gridSize());
        endRect.translate(-gridSize().width(), -gridSize().height());
        while (!indexAt(endRect.center()).isValid()) {
            endRect.translate(0, -gridSize().height());
        }
        auto endIndex = indexAt(endRect.center());
        selectionModel()->select(endIndex, QItemSelectionModel::SelectCurrent);
        break;
    }
    case Qt::Key_Up: {
        if (getSelections().isEmpty()) {
            selectionModel()->select(model()->index(0, 0), QItemSelectionModel::SelectCurrent);
        } else {
            auto index = selectionModel()->selectedIndexes().first();
            auto center = visualRect(index).center();
            auto up = center - QPoint(0, gridSize().height());
            auto upIndex = indexAt(up);
            if (upIndex.isValid()) {
                clearAllIndexWidgets();
                selectionModel()->select(upIndex, QItemSelectionModel::SelectCurrent);
                auto delegate = qobject_cast<DesktopIconViewDelegate *>(itemDelegate());
                auto indexWidget = new DesktopIndexWidget(delegate, viewOptions(), upIndex, this);
                setIndexWidget(upIndex, indexWidget);
                indexWidget->move(visualRect(upIndex).topLeft());

                if (g_isHighVersion) {
                    for (auto uri : getAllFileUris()) {
                        auto pos = getFileMetaInfoPos(uri);
                        if (pos.x() >= 0)
                           updateItemPosByUri(uri, pos);
                    }
                }
            }
        }
        return;
    }
    case Qt::Key_Down: {
        if (getSelections().isEmpty()) {
            selectionModel()->select(model()->index(0, 0), QItemSelectionModel::SelectCurrent);
        } else {
            auto index = selectionModel()->selectedIndexes().first();
            auto center = visualRect(index).center();
            auto down = center + QPoint(0, gridSize().height());
            auto downIndex = indexAt(down);
            if (downIndex.isValid()) {
                clearAllIndexWidgets();
                selectionModel()->select(downIndex, QItemSelectionModel::SelectCurrent);
                auto delegate = qobject_cast<DesktopIconViewDelegate *>(itemDelegate());
                auto indexWidget = new DesktopIndexWidget(delegate, viewOptions(), downIndex, this);
                setIndexWidget(downIndex, indexWidget);
                indexWidget->move(visualRect(downIndex).topLeft());

                if (g_isHighVersion) {
                    for (auto uri : getAllFileUris()) {
                        auto pos = getFileMetaInfoPos(uri);
                        if (pos.x() >= 0)
                            updateItemPosByUri(uri, pos);
                    }
                }
            }
        }
        return;
    }
    case Qt::Key_Left: {
        if (getSelections().isEmpty()) {
            selectionModel()->select(model()->index(0, 0), QItemSelectionModel::SelectCurrent);
        } else {
            auto index = selectionModel()->selectedIndexes().first();
            auto center = visualRect(index).center();
            auto left = center - QPoint(gridSize().width(), 0);
            auto leftIndex = indexAt(left);
            if (leftIndex.isValid()) {
                clearAllIndexWidgets();
                selectionModel()->select(leftIndex, QItemSelectionModel::SelectCurrent);
                auto delegate = qobject_cast<DesktopIconViewDelegate *>(itemDelegate());
                auto indexWidget = new DesktopIndexWidget(delegate, viewOptions(), leftIndex, this);
                setIndexWidget(leftIndex, indexWidget);
                indexWidget->move(visualRect(leftIndex).topLeft());

                if (g_isHighVersion) {
                    for (auto uri : getAllFileUris()) {
                        auto pos = getFileMetaInfoPos(uri);
                        if (pos.x() >= 0)
                            updateItemPosByUri(uri, pos);
                    }
                }
            }
        }
        return;
    }
    case Qt::Key_Right: {
        if (getSelections().isEmpty()) {
            selectionModel()->select(model()->index(0, 0), QItemSelectionModel::SelectCurrent);
        } else {
            auto index = selectionModel()->selectedIndexes().first();
            auto center = visualRect(index).center();
            auto right = center + QPoint(gridSize().width(), 0);
            auto rightIndex = indexAt(right);
            if (rightIndex.isValid()) {
                clearAllIndexWidgets();
                selectionModel()->select(rightIndex, QItemSelectionModel::SelectCurrent);
                auto delegate = qobject_cast<DesktopIconViewDelegate *>(itemDelegate());
                auto indexWidget = new DesktopIndexWidget(delegate, viewOptions(), rightIndex, this);
                setIndexWidget(rightIndex, indexWidget);

                if (g_isHighVersion) {
                    for (auto uri : getAllFileUris()) {
                        auto pos = getFileMetaInfoPos(uri);
                        if (pos.x() >= 0)
                            updateItemPosByUri(uri, pos);
                    }
                }
            }
        }
        return;
    }
    case Qt::Key_Shift:
    case Qt::Key_Control:
        m_ctrl_or_shift_pressed = true;
        m_ctrl_key_pressed = true;
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
        auto selections = this->getSelections();
        for (auto uri : selections)
        {
           openFileByUri(uri);
        }
    }
        break;
    default:
        return QListView::keyPressEvent(e);
    }
}

void DesktopIconView::keyReleaseEvent(QKeyEvent *e)
{
    QListView::keyReleaseEvent(e);
    m_ctrl_or_shift_pressed = false;
    m_ctrl_key_pressed = false;
}

void DesktopIconView::focusOutEvent(QFocusEvent *e)
{
    QListView::focusOutEvent(e);
    m_ctrl_or_shift_pressed = false;
}

void DesktopIconView::resizeEvent(QResizeEvent *e)
{
    qInfo()<<"resize event";

    QListView::resizeEvent(e);
    //refresh();

    if (m_initialized) {
        resolutionChange();
    } else {
        qWarning()<<"model not initialized";
    }
}

void DesktopIconView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    relayoutExsitingItems(m_model->m_items_need_relayout);
    QListView::rowsInserted(parent, start, end);
    for (auto uri : getAllFileUris()) {
        auto pos = getFileMetaInfoPos(uri);
        if (pos.x() >= 0) {
            updateItemPosByUri(uri, pos);
        }
    }
    // try fix item overrlapped sometimes, link to #58739
    //task#74174 扩展模式下支持拖拽图标放置到扩展屏,当扩展屏没有时会崩溃
    if (start == end && m_item_rect_hash.count() > 1) {
        auto index = model()->index(start, 0);
        auto uri = index.data(Qt::UserRole).toString();
        auto itemRectHash = m_item_rect_hash;
        itemRectHash.remove(uri);
        QRegion notEmptyRegion;
        QSize itemRectSize = m_item_rect_hash.first().size();
        for (auto rect : itemRectHash.values()) {
            notEmptyRegion += rect;
        }

        auto itemRect = QRect(m_item_rect_hash.value(uri).topLeft(), itemRectSize);
        if (notEmptyRegion.intersects(itemRect)) {
            // handle overlapped
            qWarning()<<"unexpected overrlapped happened";
            qDebug()<<"check item rect hash"<<m_item_rect_hash;
            QStringList fakeList;
            fakeList<<uri;
            relayoutExsitingItems(fakeList);
        }

        if (uri == m_model->m_renaming_file_pos.first || uri == m_model->m_renaming_file_pos.first + ".desktop") {
            updateItemPosByUri(uri, m_model->m_renaming_file_pos.second);
        } else if (m_model->m_renaming_operation_info.get()) {
            if (m_model->m_renaming_operation_info.get()->target() == uri) {
                updateItemPosByUri(uri, m_model->m_renaming_file_pos.second);
            }
        }
    }
    clearAllIndexWidgets();
}

void DesktopIconView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; row++) {
        auto uri = model()->index(row, 0).data(Qt::UserRole).toString();
        m_item_rect_hash.remove(uri);
        m_resolution_item_rect.remove(uri);
        m_storageBox.removeOne(uri);
//        QPoint itemPos(-1, -1);
//        setRestoreInfo(uri, itemPos);
    }
    qDebug() << "[DesktopIconView::rowsAboutToBeRemove] need relayout:" << m_model->m_items_need_relayout;
    relayoutExsitingItems(m_model->m_items_need_relayout);
    QListView::rowsAboutToBeRemoved(parent, start, end);
//    QTimer::singleShot(1, this, [=](){
//        for (auto uri : getAllFileUris()) {
//            auto pos = getFileMetaInfoPos(uri);
//            if (pos.x() >= 0)
//                updateItemPosByUri(uri, pos);
//        }
//    });

    clearAllIndexWidgets();
}

bool DesktopIconView::isItemsOverlapped()
{
    QList<QRect> itemRects;
    if (model()) {
        for (int i = 0; i < model()->rowCount(); i++) {
            auto index = model()->index(i, 0);
            auto rect = visualRectInRightToLeft(index);
            if (itemRects.contains(rect))
                return true;
            itemRects<<visualRectInRightToLeft(index);
        }
    }

    return false;
}

bool DesktopIconView::isRenaming()
{
    return m_is_renaming;
}

void DesktopIconView::setRenaming(bool renaming)
{
    m_is_renaming = renaming;
}

const QRect DesktopIconView::getBoundingRect()
{
    QRegion itemsRegion;
    for (int i = 0; i < m_proxy_model->rowCount(); i++) {
        auto index = m_proxy_model->index(i, 0);
        QRect indexRect = visualRectInRightToLeft(index);
        itemsRegion += indexRect;
    }
    return itemsRegion.boundingRect();
}

void DesktopIconView::relayoutExsitingItems(const QStringList &uris)
{
    if (uris.isEmpty() || m_item_rect_hash.isEmpty()) {
        return;
    }
    auto allFileUris = getAllFileUris();

    auto ensuredItemRectHash = m_item_rect_hash;
    for (auto uri : uris) {
        ensuredItemRectHash.remove(uri);
    }

    QRegion notEmptyRegion;
    QSize size = m_item_rect_hash.values().first().size();
    for (auto uri : ensuredItemRectHash.keys()) {
        QModelIndex srcIndex = m_model->indexFromUri(uri);
        QModelIndex index = m_proxy_model->mapFromSource(srcIndex);
        QRect rect = getDataRect(index);
        rect.moveTo(m_item_rect_hash.value(uri).topLeft());
        size = rect.size();
        notEmptyRegion += rect;
    }

    auto grid = this->gridSize();
    auto viewRect = this->viewport()->rect();

    // aligin exsited rect
    int marginTop = notEmptyRegion.isEmpty()? offset().y() : notEmptyRegion.boundingRect().top();
    while (marginTop - grid.height() >= 0) {
        marginTop -= grid.height();
    }

    int marginLeft = notEmptyRegion.boundingRect().left();
    while (marginLeft - grid.width() >= 0) {
        marginLeft -= grid.width();
    }
    marginLeft = marginLeft < 0? 0: marginLeft;
    marginTop = marginTop <= 0? offset().y(): marginTop;

    for (auto uri : uris) {
        if (!allFileUris.contains(uri))
            continue;
        auto indexRect = QRect(QPoint(marginLeft, marginTop), size);
        if (notEmptyRegion.intersects(indexRect)) {

            // move index to closest empty grid.
            auto next = indexRect;
            bool isEmptyPos = false;
            while (!isEmptyPos) {
                next.translate(0, grid.height());
                if (verifyBoundaries(next, Direction::Bottom)) {
                    int top = next.y();
                    while (true) {
                        if (top < grid.height()) {
                            break;
                        }
                        top-=grid.height();
                    }
                    //put item to next column first row
                    next.moveTo(next.x() + grid.width(), top);
                    //如果满了，就放到（0，0） 位置
                    if (verifyBoundaries(next, Direction::Right)) {
                        if (!m_storageBox.contains(uri)) {
                            m_storageBox.append(uri);
                        }
                        next.moveTo(0, 0);
                        isEmptyPos = true;
                        next.setSize(m_item_rect_hash.values().first().size());
                        m_item_rect_hash.insert(uri, next);
                        setFileMetaInfoPos(uri, next.topLeft());
                        qDebug() << "满屏 " << uri << " point:" <<next.topLeft();
                        break;
                    }
                }
                if (notEmptyRegion.intersects(next))
                    continue;

                isEmptyPos = true;
                QRect tmp = next;
                tmp.setSize(m_item_rect_hash.values().first().size());
                m_item_rect_hash.insert(uri, tmp);
                notEmptyRegion += next;

                setFileMetaInfoPos(uri, next.topLeft());
            }
        } else {
            notEmptyRegion += indexRect;
            setFileMetaInfoPos(uri, indexRect.topLeft());
        }
    }
}

void DesktopIconView::checkItemsOver()
{
    QStringList needRelayoutItems;
    QRegion notEmptyRegion;
    QSize dataSize;
    if (model()) {
        QMap<QString, QRect> destoryItemMap;
        for (int i = 0; i < model()->rowCount(); i++) {
            QModelIndex index = model()->index(i, 0);
            QString uri = index.data(Qt::UserRole).toString();
            QRect rect = getDataRect(index);
            dataSize = rect.size();
            if (m_model->m_destoryItems.contains(uri)) {
                destoryItemMap.insert(uri, rect);
                continue;
            }
            if (!viewport()->rect().contains(rect)) {
                needRelayoutItems.append(uri);
                m_resolution_item_rect.insert(uri, rect);
                continue;
            }
            if (notEmptyRegion.intersects(rect)) {
                needRelayoutItems.append(uri);
            } else {
                notEmptyRegion += rect;
            }
        }
        for (auto uri : destoryItemMap.keys()) {
            auto rect = destoryItemMap.value(uri);
            if (notEmptyRegion.intersects(rect) || !viewport()->rect().contains(rect)) {
                needRelayoutItems.append(uri);
            } else {
                notEmptyRegion += rect;
            }
        }
    }
    if (0 == needRelayoutItems.size()) {
        return;
    }

    int gridWidth = gridSize().width();
    int gridHeight = gridSize().height();
    // aligin exsited rect
    int marginTop = notEmptyRegion.boundingRect().top();
    while (marginTop - gridHeight >= 0) {
        marginTop -= gridHeight;
    }
    int marginLeft = notEmptyRegion.boundingRect().left();
    while (marginLeft - gridWidth >= 0) {
        marginLeft -= gridWidth;
    }
    marginLeft = marginLeft < 0? 0: marginLeft;
    marginTop = marginTop <= 0? offset().y(): marginTop;
    int posX = marginLeft;
    int posY = marginTop;
    qDebug()<<"need relayout items:"<<needRelayoutItems;
    bool isFull = false;
    for (auto item : needRelayoutItems) {
        QRect itemRect = QRect(QPoint(posX, posY), dataSize);
        while (notEmptyRegion.contains(itemRect) && !isFull) {
            QRect tmpRect = QRect(QPoint(posX, posY+gridHeight), dataSize);
            if (verifyBoundaries(tmpRect, Direction::Bottom)) {
                posY = marginTop;
                posX += gridWidth;
            } else {
                posY += gridHeight;
            }
            itemRect.moveTo(posX, posY);
            if (verifyBoundaries(itemRect, Direction::Right)) {
                itemRect.moveTo(0, 0);
                posX = 0;
                posY = 0;
                isFull = true;
                qDebug() << "满屏 " << item << " point:" <<itemRect.topLeft();
                break;
            }
        }
        notEmptyRegion += itemRect;
        itemRect.setSize(m_item_rect_hash.values().first().size());
        m_item_rect_hash.insert(item, itemRect);
    }
    for (auto uri : m_item_rect_hash.keys()) {
        auto rect = m_item_rect_hash.value(uri);
        if (isFull && 0 == rect.x() && 0 == rect.y() && !m_storageBox.contains(uri) && !m_resolution_item_rect.contains(uri)) {
            m_storageBox.append(uri);
        }
        updateItemPosByUri(uri, rect.topLeft());
        setFileMetaInfoPos(uri, rect.topLeft());
    }
}

void DesktopIconView::zoomOut()
{
    clearAllIndexWidgets();
    switch (zoomLevel()) {
    case Huge:
        setDefaultZoomLevel(Large);
        break;
    case Large:
        setDefaultZoomLevel(Normal);
        break;
    case Normal:
        setDefaultZoomLevel(Small);
        break;
    default:
        //setDefaultZoomLevel(zoomLevel());
        break;
    }
}

void DesktopIconView::zoomIn()
{
    clearAllIndexWidgets();
    switch (zoomLevel()) {
    case Small:
        setDefaultZoomLevel(Normal);
        break;
    case Normal:
        setDefaultZoomLevel(Large);
        break;
    case Large:
        setDefaultZoomLevel(Huge);
        break;
    default:
        //setDefaultZoomLevel(zoomLevel());
        break;
    }
}

/*
Small, //icon: 32x32; grid: 56x64; hover rect: 40x56; font: system*0.8
Normal, //icon: 48x48; grid: 64x72; hover rect = 56x64; font: system
Large, //icon: 64x64; grid: 115x135; hover rect = 105x118; font: system*1.2
Huge //icon: 96x96; grid: 130x150; hover rect = 120x140; font: system*1.4
*/
void DesktopIconView::setDefaultZoomLevel(ZoomLevel level)
{
    //qDebug()<<"set default zoom level:"<<level;
    m_zoom_level = level;
    switch (level) {
    case Small:
        setIconSize(QSize(24, 24));
        setGridSize(QSize(5, 5) + itemDelegate()->sizeHint(viewOptions(), QModelIndex()));
        break;
    case Large:
        setIconSize(QSize(64, 64));
        setGridSize(QSize(15, 15) + itemDelegate()->sizeHint(viewOptions(), QModelIndex()));
        break;
    case Huge:
        setIconSize(QSize(96, 96));
        setGridSize(QSize(20, 20) + itemDelegate()->sizeHint(viewOptions(), QModelIndex()));
        break;
    default:
        m_zoom_level = Normal;
        setIconSize(QSize(48, 48));
        setGridSize(QSize(10, 10) + itemDelegate()->sizeHint(viewOptions(), QModelIndex()));
        break;
    }
    clearAllIndexWidgets();
    auto metaInfo = FileMetaInfo::fromUri("computer:///");
    if (metaInfo) {
        qDebug()<<"set zoom level"<<m_zoom_level;
        metaInfo->setMetaInfoInt("explorer-qt-desktop-zoom-level", int(m_zoom_level));
    }

    resetAllItemPositionInfos();
    if (m_model) {
        m_model->clearFloatItems();
    }

    auto settings = Peony::GlobalSettings::getInstance();
    if (m_initialized && settings) {
        settings->setValue(DEFAULT_GRID_SIZE, gridSize());
    }
}

DesktopIconView::ZoomLevel DesktopIconView::zoomLevel() const
{
    //FIXME:
    if (m_zoom_level != Invalid)
        return m_zoom_level;

    auto metaInfo = FileMetaInfo::fromUri("computer:///");
    if (metaInfo) {
        auto i = metaInfo->getMetaInfoInt("explorer-qt-desktop-zoom-level");
        return ZoomLevel(i);
    }

    GFile *computer = g_file_new_for_uri("computer:///");
    GFileInfo *info = g_file_query_info(computer,
                                        "metadata::explorer-qt-desktop-zoom-level",
                                        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                        nullptr,
                                        nullptr);
    char* zoom_level = g_file_info_get_attribute_as_string(info, "metadata::explorer-qt-desktop-zoom-level");
    if (!zoom_level) {
        //qDebug()<<"======================no zoom level meta info\n\n\n";
        g_object_unref(info);
        g_object_unref(computer);
        return Normal;
    }
    g_object_unref(info);
    g_object_unref(computer);
    QString zoomLevel = zoom_level;
    g_free(zoom_level);
    //qDebug()<<ZoomLevel(QString(zoomLevel).toInt())<<"\n\n\n\n\n\n\n\n";
    return ZoomLevel(zoomLevel.toInt()) == Invalid? Normal: ZoomLevel(QString(zoomLevel).toInt());
}

void DesktopIconView::setEditFlag(bool edit)
{
    qDebug() << "setEditFlag:" <<edit;
    m_is_edit = edit;
    if (! m_is_edit)
        m_edit_uri = "";
}

bool DesktopIconView::getEditFlag()
{
    return m_is_edit;
}

int DesktopIconView::verticalOffset() const
{
    return 0;
}

int DesktopIconView::horizontalOffset() const
{
    return 0;
}

void DesktopIconView::restoreItemsPosByMetaInfo()
{
    for (auto uri : getAllFileUris()) {
        auto pos = getFileMetaInfoPos(uri);
        if (pos.x() >= 0) {
            updateItemPosByUri(uri, pos);
        }
    }
}

void DesktopIconView::mousePressEvent(QMouseEvent *e)
{
    m_press_pos = e->pos();
    // bug extend selection bug
    m_real_do_edit = false;

    if (e->modifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else {
        m_ctrl_key_pressed = false;
        if (! (e->modifiers() & Qt::ShiftModifier))
            m_ctrl_or_shift_pressed = false;
    }

    auto index = indexAt(e->pos());

    if (!m_ctrl_or_shift_pressed) {
        if (!index.isValid()) {
            clearAllIndexWidgets();
            clearSelection();
        } else {
            m_last_index = index;
            //fix rename state has no menuRequest issue, bug#44107
            if (! m_is_edit)
            {
                clearAllIndexWidgets();
                //force to recreate new DesktopIndexWidget, to fix not show name issue
                if (indexWidget(m_last_index))
                    setIndexWidget(m_last_index, nullptr);
                auto indexWidget = new DesktopIndexWidget(qobject_cast<DesktopIconViewDelegate *>(itemDelegate()), viewOptions(), m_last_index);
                setIndexWidget(m_last_index,
                               indexWidget);
                indexWidget->move(visualRect(m_last_index).topLeft());
            }

            if (g_isHighVersion) {
                for (auto uri : getAllFileUris()) {
                    auto pos = getFileMetaInfoPos(uri);
                    if (pos.x() >= 0)
                        updateItemPosByUri(uri, pos);
                }
            }
        }
    }

    //qDebug()<<m_last_index.data();
    if (e->button() != Qt::LeftButton) {
        // fix #115384, context menu key issue
        if (e->button() == Qt::RightButton) {
            auto index = indexAt(e->pos());
            if (!selectedIndexes().contains(index)) {
                selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
            }
        }
        return;
    }

    if (e->button() == Qt::LeftButton && e->modifiers() & Qt::ControlModifier && selectedIndexes().contains(index)) {
        m_noSelectOnPress = true;
    } else {
        m_noSelectOnPress = false;
    }

    QListView::mousePressEvent(e);
}

void DesktopIconView::mouseReleaseEvent(QMouseEvent *e)
{
    QListView::mouseReleaseEvent(e);

    m_noSelectOnPress = false;

    this->viewport()->update(viewport()->rect());
}

void DesktopIconView::mouseMoveEvent(QMouseEvent *e)
{
    QModelIndex itemIndex = indexAt(e->pos());
    if (!itemIndex.isValid()) {
        if (QToolTip::isVisible()) {
            QToolTip::hideText();
        }
    }

    QListView::mouseMoveEvent(e);
}

void DesktopIconView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QListView::mouseDoubleClickEvent(event);
    m_real_do_edit = false;
}

void DesktopIconView::dragEnterEvent(QDragEnterEvent *e)
{
    m_real_do_edit = false;

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    qDebug()<<"drag enter event" <<action;
    if (e->mimeData()->hasUrls()) {
        if (FileUtils::containsStandardPath(e->mimeData()->urls())) {
            e->ignore();
            return;
        }
        e->setDropAction(action);
        e->accept();
        //e->acceptProposedAction();
    }

    if (e->source() == this) {
        m_drag_indexes = selectedIndexes();
    } else {
        //task#74174 扩展模式下支持拖拽图标放置到扩展屏,获取选中项
        //fix bug#165132, do nothing when in rename status
        auto view = qobject_cast<DesktopIconView*>(e->source());
        if (view) {
            m_drag_indexes = view->selectedIndexes();
        }
    }
}

void DesktopIconView::dragMoveEvent(QDragMoveEvent *e)
{
    m_real_do_edit = false;
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    auto index = indexAt(e->pos());
    if (index.isValid() && index != m_last_index) {
        QHoverEvent he(QHoverEvent::HoverMove, e->posF(), e->posF());
        viewportEvent(&he);
    } else {
        QHoverEvent he(QHoverEvent::HoverLeave, e->posF(), e->posF());
        viewportEvent(&he);
    }
    e->setDropAction(action);
    if (e->isAccepted())
        return;
    qDebug()<<"drag move event" <<action;
    if (this == e->source()) {
        e->accept();
        return QListView::dragMoveEvent(e);
    }
    e->accept();
}

void DesktopIconView::dropEvent(QDropEvent *e)
{
    // fix #122768, dirty region issues.
    this->viewport()->update();
    m_model->clearFloatItems();
    m_real_do_edit = false;
    //qDebug()<<"drop event";
    /*!
      \todo
      fix the bug that move drop action can not move the desktop
      item to correct position.

      i use copy action to avoid this bug, but the drop indicator
      is incorrect.
      */
    m_edit_trigger_timer.stop();
    if (e->keyboardModifiers() & Qt::ControlModifier) {
        m_ctrl_key_pressed = true;
    } else {
        m_ctrl_key_pressed = false;
    }

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    if (e->keyboardModifiers() & Qt::ShiftModifier) {
        action = Qt::TargetMoveAction;
    }
    qDebug() << "DesktopIconView dropEvent" <<action;
    auto index = indexAt(e->pos());
    if (index.isValid() || m_ctrl_key_pressed)
    {
        qDebug() <<"DesktopIconView index copyAction:";
        auto urls = e->mimeData()->urls();
        QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        QStringList uris;
        for (auto url : urls)
        {
            if (url.toString() == "computer:///")
                uris << "computer:///";
            else
                uris << url.path();
        }

        //fix can drag copy home folder issue, link to bug#64824
        if (uris.contains(homePath) || uris.contains("computer:///"))
            return;
    }

    if (this == e->source() && !m_ctrl_key_pressed) {
        qDebug() <<"DesktopIconView index:" <<index <<index.isValid();
        bool bmoved = false;
        if (index.isValid()) {
            auto uri = m_model->indexUri(m_proxy_model->mapToSource(index));
            auto info = FileInfo::fromUri(index.data(Qt::UserRole).toString());
            if (!info->isDir()||e->mimeData()->urls().contains(uri)) {
                return;
            }
            bmoved = true;
        }

        if (bmoved) {
            //move file to desktop folder
            qDebug() << "DesktopIconView move file to folder";
            for (auto uuri : e->mimeData()->urls()) {
                if ("trash:///" == uuri.toDisplayString() || "computer:///" == uuri.toDisplayString()) {
                    return;
                }
            }

            m_model->dropMimeData(e->mimeData(), action, -1, -1, this->indexAt(e->pos()));
        } else {
            // do not trigger file operation, link to: #66345
            m_model->setAcceptDropAction(false);
            QListView::dropEvent(e);
            m_model->setAcceptDropAction(true);
        }

        QRegion dirtyRegion;
        QHash<QModelIndex, QRect> currentIndexesRects;

        for (int i = 0; i < m_proxy_model->rowCount(); i++) {
            auto tmp = m_proxy_model->index(i, 0);
            QRect rect = getDataRect(tmp);
            currentIndexesRects.insert(tmp, rect);
            if (!m_drag_indexes.contains(tmp)) {
                dirtyRegion += rect;
            }
        }

        //fixme: handle overlapping.
        if (!m_drag_indexes.isEmpty()) {
            // remove info from resolution item rect map.
            for (auto index : m_drag_indexes) {
                m_resolution_item_rect.remove(index.data(Qt::UserRole).toString());
                m_model->m_destoryItems.removeOne(index.data(Qt::UserRole).toString());
                QPoint itemPos(-1, -1);
                QString uri = index.data(Qt::UserRole).toString();
                setRestoreInfo(uri, itemPos);
            }

            QModelIndexList overlappedIndexes;
            QModelIndexList unoverlappedIndexes = m_drag_indexes;

            for (auto index : unoverlappedIndexes) {
                QRect visualRect = getDataRect(index);
                if (dirtyRegion.intersects(visualRect)) {
                    unoverlappedIndexes.removeOne(index);
                    overlappedIndexes.append(index);
                }
            }

            for (auto index : unoverlappedIndexes) {
                // save pos
                QTimer::singleShot(1, this, [=]() {
                    setFileMetaInfoPos(index.data(Qt::UserRole).toString(), visualRectInRightToLeft(index).topLeft());
                });
            }

            auto grid = this->gridSize();
            auto viewRect = this->viewport()->rect();

            QRegion notEmptyRegion;
            for (auto rect : currentIndexesRects) {
                notEmptyRegion += rect;
            }

            for (auto dragedIndex : overlappedIndexes) {
                auto indexRect = visualRectInRightToLeft(dragedIndex);
                auto dataRect = getDataRect(dragedIndex);
                if (notEmptyRegion.intersects(dataRect)) {
                    // move index to closest empty grid.
                    auto next = indexRect;
                    bool isEmptyPos = false;
                    while (!isEmptyPos) {
                        next.translate(0, grid.height());
                        if (verifyBoundaries(next, Direction::Bottom)) {
                            int top = next.y();
                            while (true) {
                                if (top < gridSize().height()) {
                                    break;
                                }
                                top-=gridSize().height();
                            }
                            //put item to next column first column
                            next.moveTo(next.x() + grid.width(), top);
                            //如果满了，就放到（0，0） 位置
                            if (verifyBoundaries(next, Direction::Right)) {
                                next.moveTo(0, 0);
                                isEmptyPos = true;
                                setPositionForIndex(next.topLeft(), dragedIndex);
                                setFileMetaInfoPos(dragedIndex.data(Qt::UserRole).toString(), next.topLeft());
                                qDebug() << "满屏 " << dragedIndex.data(Qt::UserRole).toString() << " point:" <<next.topLeft();
                                break;
                            }
                        }

                        if (notEmptyRegion.intersects(next)) {
                            continue;
                        }

                        isEmptyPos = true;

                        setPositionForIndex(next.topLeft(), dragedIndex);
                        setFileMetaInfoPos(dragedIndex.data(Qt::UserRole).toString(), next.topLeft());
                        notEmptyRegion += next;
                    }
                }
            }

            // check if there is any item out of view
            for (auto index : m_drag_indexes) {
                auto indexRect = visualRectInRightToLeft(index);
                if (!verifyBoundaries(indexRect, Direction::All)) {
                    continue;
                }

                // try relocating invisible item
                QRect next(0, 0, 0, 0);
                for (auto existedRect = notEmptyRegion.begin(); existedRect != notEmptyRegion.end(); ++existedRect) {
                    if (this->viewport()->rect().contains(*existedRect)) {
                        next = *existedRect;
                        break;
                    } else if (existedRect == notEmptyRegion.end() - 1
                               && next == QRect(0, 0, 0, 0)) {
                        next = *existedRect;
                        next.moveTo(0, 0);
                    }
                }

                while (next.translated(-grid.width(), 0).x() >= 0) {
                    next.translate(-grid.width(), 0);
                }
                while (next.translated(0, -grid.height()).top() >= 0) {
                    next.translate(0, -grid.height());
                }
                QRect dataRect = getDataRect(index);
                next.setSize(dataRect.size());
                while (notEmptyRegion.intersects(next)) {
                    next.translate(0, grid.height());
                    if (verifyBoundaries(next, Direction::Bottom)) {
                        int top = next.y();
                        while (true) {
                            if (top < gridSize().height()) {
                                break;
                            }
                            top-=gridSize().height();
                        }
                        //put item to next column first column
                        top = top > 0 ? top : offset().y();
                        next.moveTo(next.x() + grid.width(), top);
                        //如果满了，就放到（0，0） 位置
                        if (verifyBoundaries(next, Direction::Right)) {
                            next.moveTo(0, 0);
                            qDebug() << "满屏 " << index.data(Qt::UserRole).toString() << " point:" <<next.topLeft();
                            break;
                        }
                    }
                }

                setPositionForIndex(next.topLeft(), index);
                setFileMetaInfoPos(index.data(Qt::UserRole).toString(), next.topLeft());
                notEmptyRegion += next;
            }
        }

        m_drag_indexes.clear();

        auto urls = e->mimeData()->urls();
        for (auto url : urls) {
//            if (url.path() == QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
//                continue;
            saveItemPositionInfo(url.toDisplayString());
        }
        return;
    }

    //task#74174 扩展模式下支持拖拽图标放置到扩展屏,拖拽释放后进行设置过滤器，proxyModel刷新
    if (!m_ctrl_key_pressed && dragToOtherScreen(e))
        return;

    m_model->dropMimeData(e->mimeData(), action, -1, -1, this->indexAt(e->pos()));
    //FIXME: save item position
}

void DesktopIconView::startDrag(Qt::DropActions supportedActions)
{
    auto indexes = selectedIndexes();
    if (indexes.count() > 0) {
        auto pos = m_press_pos;
        qreal scale = 1.0;
        QWidget *window = this->window();
        if (window) {
            auto windowHandle = window->windowHandle();
            if (windowHandle) {
                scale = windowHandle->devicePixelRatio();
            }
        }

        auto drag = new QDrag(this);
        drag->setMimeData(model()->mimeData(indexes));

        QRegion rect;
        QHash<QModelIndex, QRect> indexRectHash;
        for (auto index : indexes) {
            rect += (visualRect(index));
            indexRectHash.insert(index, visualRect(index));
        }

        QRect realRect = rect.boundingRect();

        // fix #78263, text displayment is not completed.
        //realRect.adjust(-5, -5, 5, 5);

        realRect.adjust(-15, -15, 15, 15);
        QPixmap pixmap(realRect.size() * scale);
        pixmap.fill(Qt::transparent);
        pixmap.setDevicePixelRatio(scale);
        QPainter painter(&pixmap);
        // try fixing #190315, text shadow displayment issue while compositing not running.
        bool shouldDrawBackground = !QX11Info::isCompositingManagerRunning();
        for (auto index : indexes) {
            painter.save();
            painter.translate(indexRectHash.value(index).topLeft() - rect.boundingRect().topLeft());
            if (shouldDrawBackground) {
                painter.setPen(qApp->palette().highlight().color());
                painter.setBrush(qApp->palette().highlight());
                painter.drawRoundedRect(QRect(0, 0, this->gridSize().width(), this->gridSize().height()).adjusted(1, 1, -1, -1), 6, 6);
            }
            QStyleOptionViewItem opt = viewOptions();
            auto viewItemDelegate = static_cast<DesktopIconViewDelegate *>(itemDelegate());
            viewItemDelegate->initIndexOption(&opt, index);
            opt.rect.setSize(visualRect(index).size());
            itemDelegate()->paint(&painter, opt, index);
            painter.restore();
        }

        drag->setPixmap(pixmap);
        drag->setHotSpot(pos - rect.boundingRect().topLeft() - QPoint(viewportMargins().left(), viewportMargins().top()));
        drag->setDragCursor(QPixmap(), m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
        drag->exec(m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);

    } else {
        return QListView::startDrag(Qt::MoveAction|Qt::CopyAction);
    }
}

const QFont DesktopIconView::getViewItemFont(QStyleOptionViewItem *item)
{
    return item->font;
//    auto font = item->font;
//    if (font.pixelSize() <= 0) {
//        font = QApplication::font();
//    }
//    switch (zoomLevel()) {
//    case DesktopIconView::Small:
//        font.setPixelSize(int(font.pixelSize() * 0.8));
//        break;
//    case DesktopIconView::Large:
//        font.setPixelSize(int(font.pixelSize() * 1.2));
//        break;
//    case DesktopIconView::Huge:
//        font.setPixelSize(int(font.pixelSize() * 1.4));
//        break;
//    default:
//        break;
//    }
//    return font;
}

void DesktopIconView::clearAllIndexWidgets(const QStringList &uris)
{
    if (!model())
        return;

    //fix bug#164160, when edit new file, infoUpdate call clearAllIndexWidgets issue
    if (m_is_edit && uris.length()>0 && m_edit_uri == uris.first())
        return;

    if(uris.length()>0 )
       qDebug() << "clearAllIndexWidgets uris:"<<uris.first()<<uris.length();

    int row = 0;
    auto index = model()->index(row, 0);
    while (index.isValid()) {
        if (uris.isEmpty() || uris.contains(index.data(Qt::UserRole).toString())) {
            auto widget = indexWidget(index);
            if (widget) {
                widget->hide();
            }
            setIndexWidget(index, nullptr);
            qDebug() << "clearAllIndexWidgets setIndexWidget"<<index;
        }
        row++;
        index = model()->index(row, 0);
    }

    // avoid dirty region out of index visual rect.
    // link to: #77272.
    viewport()->update();
}

void DesktopIconView::refresh()
{
    this->setCursor(QCursor(Qt::WaitCursor));
//    if (m_refresh_timer.isActive())
//        return;
    //fix refresh clear copy files issue, link to bug#109247
    if (Peony::ClipboardUtils::isDesktopFilesBeCut())
        Peony::ClipboardUtils::clearClipboard();/* Refresh clear cut status */
    if (!m_model)
        return;

    if (refreshing)
        return;
    refreshing = true;
    m_model->refresh();

    //m_refresh_timer.start();
}

QRect DesktopIconView::visualRect(const QModelIndex &index) const
{
    auto rect = QListView::visualRect(index);
    QPoint p(10, 5);

    switch (zoomLevel()) {
    case Small:
        p *= 0.8;
        break;
    case Large:
        p *= 1.2;
        break;
    case Huge:
        p *= 1.4;
        break;
    default:
        break;
    }
    rect.moveTo(rect.topLeft() + p);
    auto size = itemDelegate()->sizeHint(QStyleOptionViewItem(), index);
    rect.setSize(size);

    return rect;
}

QPoint DesktopIconView::offset()
{
    QSize interval = (gridSize() - itemDelegate()->sizeHint(viewOptions(), QModelIndex()))/2;
    if (interval.isEmpty()) {
        return QPoint(0, 0);
    }
    QPoint p(interval.width(), interval.height());
    return p;
}

QRect DesktopIconView::getViewRect()
{
    QRect rect = viewport()->rect();
    QPoint p(10, 5);

    switch (zoomLevel()) {
    case Small:
        p *= 0.8;
        break;
    case Large:
        p *= 1.2;
        break;
    case Huge:
        p *= 1.4;
        break;
    default:
        break;
    }
    rect.moveTo(rect.topLeft() + p);
    return rect;
}

int DesktopIconView::updateBWList()
{
    m_proxy_model->updateBlackAndWriteLists();
    /*
    * 重新按照既定规则排序，这样可以避免出现空缺和图标重叠的情况
    */
//    int sortType = GlobalSettings::getInstance()->getValue(LAST_DESKTOP_SORT_ORDER).toInt();
//    setSortType(sortType);
    //不可重新排序，会丢失位置，只做检查调整空缺和重叠情况，相关bug#161875
    resolutionChange();
    return 0;
}

QString DesktopIconView::getBlackAndWhiteModel()
{
    return m_proxy_model->getBlackAndWhiteModel();
}

QSet<QString> DesktopIconView::getBWListInfo()
{
    return m_proxy_model->getBWListInfo();
}

bool DesktopIconView::getBlackAndWhiteListExist(QString name)
{
   return m_proxy_model->getBlackAndWhiteListExist(name);
}

void DesktopIconView::setRestoreInfo(QString &uri, QPoint &itemPos)
{
    //bug#108126 根据url重新记录要恢复的元素的位置
    auto metaInfo = FileMetaInfo::fromUri(uri);
    if (metaInfo) {
        QStringList restoreInfo;
      //  restoreInfo<<pixelRatio;
        restoreInfo<<QString::number(itemPos.x());
        restoreInfo<<QString::number(itemPos.y());
        restoreInfo<<QString::number(m_id);
        metaInfo->setMetaInfoStringList(RESTORE_ITEM_POS_ATTRIBUTE, restoreInfo);
    }
}

void DesktopIconView::setAllRestoreInfo()
{
    //bug#108126 超出屏幕的元素记录到metInfo,以便以后恢复
    for (auto uri : m_resolution_item_rect.keys()) {
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            auto rect = m_resolution_item_rect.value(uri);
            QStringList restoreInfo;
            restoreInfo<<QString::number(rect.topLeft().x());
            restoreInfo<<QString::number(rect.topLeft().y());
            restoreInfo<<QString::number(m_id);
            metaInfo->setMetaInfoStringList(RESTORE_ITEM_POS_ATTRIBUTE, restoreInfo);
        }
    }
}

void DesktopIconView::getAllRestoreInfo()
{
    //bug#108126 一开始加载将上一次超出屏幕的元素恢复
    for (auto uri : m_item_rect_hash.keys()) {
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            QStringList restoreInfo = metaInfo->getMetaInfoStringList(RESTORE_ITEM_POS_ATTRIBUTE);
            if (restoreInfo.count() == 3) {
                int top = restoreInfo.at(0).toInt();
                int left = restoreInfo.at(1).toInt();
                int id = restoreInfo.at(2).toInt();
                if (id == m_id && top >= 0 && left >= 0) {
                    QPoint topLeft(top, left);
                    updateItemPosByUri(uri, topLeft);
                    setFileMetaInfoPos(uri, topLeft);
                    QStringList resetInfo;
                    resetInfo<<"";
                    metaInfo->setMetaInfoStringList(RESTORE_ITEM_POS_ATTRIBUTE, resetInfo);
                }
            }
        }
    }
}

void DesktopIconView::clearAllRestoreInfo()
{
    //bug#108126 超出屏幕的元素记录清空
    for (auto uri : m_resolution_item_rect.keys()) {
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            auto rect = m_resolution_item_rect.value(uri);
            QStringList restoreInfo;
            restoreInfo<<"";
            metaInfo->setMetaInfoStringList(RESTORE_ITEM_POS_ATTRIBUTE, restoreInfo);
        }
    }
    m_resolution_item_rect.clear();
}

void DesktopIconView::clearCache()
{
    m_item_rect_hash.clear();
    m_resolution_item_rect.clear();
}

bool DesktopIconView::execSharedFileLink(const QString uri)
{
    auto info = FileInfo::fromUri(uri);
    if (info->isEmptyInfo()) {
        FileInfoJob j(info);
        j.querySync();
    }
    if (uri.endsWith(".desktop")) {
        GKeyFile* key_file = g_key_file_new();
        QUrl url = uri;
        QString desktopfp = url.path();
        g_key_file_load_from_file(key_file, desktopfp.toUtf8().constData(), G_KEY_FILE_KEEP_COMMENTS, nullptr);
        GError* error = NULL;
        if (g_key_file_has_key(key_file, G_KEY_FILE_DESKTOP_GROUP, "X-Peony-CMD", nullptr)) {
            if (g_key_file_has_key(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, nullptr)) {
                g_autofree char* val = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, &error);
                if (error) {
                    qWarning() << "get desktop file:" << uri << " name error:" << error->code << " -- " << error->message;
                    g_error_free(error);
                    error = nullptr;
                } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                    QProcess p;
                    p.setProgram("explorer");
                    QString str = val;
                    str = str.replace("explorer ","");
                    p.setArguments(QStringList() << str);
                    qint64 pid;
                    p.startDetached(&pid);

                    // send startinfo to kwindowsystem
                    quint32 timeStamp = QX11Info::isPlatformX11() ? QX11Info::appUserTime() : 0;
                    KStartupInfoId startInfoId;
                    startInfoId.initId(KStartupInfo::createNewStartupIdForTimestamp(timeStamp));
                    startInfoId.setupStartupEnv();
                    KStartupInfoData data;
                    data.setHostname();
                    data.addPid(pid);
#ifdef KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
                    QRect rect = info.get()->property("iconGeometry").toRect();
                    if (rect.isValid()) {
                        data.setIconGeometry(rect);
                    }
#endif
                    data.setLaunchedBy(getpid());
                    KStartupInfo::sendStartup(startInfoId, data);
#else
                    QProcess p;
                    QString strq;
                    for (int i = 0;i < uri.length();++i) {
                        if(uri[i] == ' '){
                            strq += "%20";
                        }else{
                            strq += uri[i];
                        }
                    }
                    p.startDetached("/usr/bin/explorer", QStringList()<<strq<<"%U&");
#endif
                    return true;
                }

            }
        }
    }
    return false;
}

void DesktopIconView::refreshResolutionChange()
{
    setAllRestoreInfo();
    getAllRestoreInfo();
}

void DesktopIconView::fileCreated(const QString &uri)
{
    qDebug()<<"DesktopIconView::fileCreated,view:" << this <<m_new_files_to_be_selected.length();
    if (m_new_files_to_be_selected.isEmpty()) {
        m_new_files_to_be_selected<<uri;

        QTimer::singleShot(500, this, [=]() {
            qDebug() << "m_new_files_to_be_selected isEmpty:"<<this->state();
            if (this->state() & QAbstractItemView::EditingState)
                return;

            if (! this->m_uris_to_edit.isEmpty())
                return;
            qDebug() << "fileCreated setSelections"<<m_new_files_to_be_selected.length();
            this->setSelections(m_new_files_to_be_selected);
            m_new_files_to_be_selected.clear();
        });
    } else {
        if (!m_new_files_to_be_selected.contains(uri)) {
            m_new_files_to_be_selected<<uri;
        }
    }

    auto geo = viewport()->rect();
    if (geo.width() != 0 && geo.height() != 0) {
        QModelIndex index = m_proxy_model->mapToSource(m_model->indexFromUri(uri));
        if (index.isValid()) {
            QRect indexRect = visualRectInRightToLeft(index);
            if (verifyBoundaries(indexRect, Direction::All)) {
                resolutionChange();
            }
        } else {
            qWarning()<<"file is created but not valid in proxy model now";
        }
    }

    /* 新建文件/文件夹，可编辑文件名，copy时不能编辑 */
    //fix bug#164160, use same way as mainwindow
    if(this->m_uris_to_edit.isEmpty())
        return;

    QString editUri = Peony::FileUtils::urlDecode(this->m_uris_to_edit.first());
    QString infoUri = Peony::FileUtils::urlDecode(uri);
    qDebug() << "fileCreated editUri:"<<editUri<<infoUri;
    if (editUri == infoUri ) {
        QTimer::singleShot(100, this, [=]() {
            this->editUri(uri);
            m_edit_uri = uri;
        });
    }
    this->m_uris_to_edit.clear();
}

bool DesktopIconView::dragToOtherScreen(QDropEvent *e)
{
    auto view = static_cast<DesktopIconView*>(e->source());
    if (this != e->source() && view) {
        bool bDropToOtherScreen = false;
        for (QModelIndex index : m_drag_indexes) {
            auto metaInfo = FileMetaInfo::fromUri(index.data(Qt::UserRole).toString());
            if (metaInfo) {
                int id = metaInfo->getMetaInfoInt("explorer-qt-desktop-id");
                if (m_id != id) {
                    metaInfo->setMetaInfoInt("explorer-qt-desktop-id", m_id);
                    bDropToOtherScreen =true;
                }
            }
        }

        if (bDropToOtherScreen) {
            QRegion notEmptyRegion;
            for (int i = 0; i < m_proxy_model->rowCount(); i++) {
                auto tmp = m_proxy_model->index(i, 0);
                QRect rect = getDataRect(tmp);
                notEmptyRegion += rect;
            }

            auto grid = this->gridSize();
            QRect viewRect = getViewRect();
            QPoint startPos = view->visualRect(m_drag_indexes[0]).topLeft();
            for (QModelIndex index : m_drag_indexes) {
                bool isOverRect = false;
                QRect dataRect = view->getDataRect(index);
                QRect rect = view->visualRect(index);
                QPoint relativePos = QPoint(rect.topLeft().x() - startPos.x(),rect.topLeft().y() - startPos.y());
                QPoint currentPos = e->pos() + relativePos;
                int x = currentPos.x()/grid.width()*grid.width();
                int y = currentPos.y()/grid.height()*grid.height()+viewRect.topLeft().y();
                rect.moveTo(QPoint(x,y));
                dataRect.moveTo(QPoint(x,y));
                if (verifyBoundaries(rect, Direction::All)) {
                    if (isFull()) {
                        rect.moveTo(0, 0);
                        setFileMetaInfoPos(index.data(Qt::UserRole).toString(), rect.topLeft());
                        continue;
                    } else {
                        rect.moveTo(0, viewRect.topLeft().y());
                        isOverRect = true;
                    }
                }

                if (notEmptyRegion.contains(dataRect)) {
                    auto next = rect;
                    bool isEmptyPos = false;
                    while (!isEmptyPos) {
                        next.translate(0, grid.height());
                        if (verifyBoundaries(next, Direction::Bottom)) {
                            int top = next.y();
                            while (true) {
                                if (top < gridSize().height()) {
                                    break;
                                }
                                top-=gridSize().height();
                            }
                            //put item to next column first column
                            next.moveTo(next.x() + grid.width(), top);
                            if (verifyBoundaries(next, Direction::Right)) {
                                if (isFull() || isOverRect) {
                                    next.moveTo(0, 0);
                                    isEmptyPos = true;
                                    setFileMetaInfoPos(index.data(Qt::UserRole).toString(), next.topLeft());
                                    continue;
                                } else {
                                    next.moveTo(0, top);
                                    isOverRect = true;
                                }
                            }
                        }
                        if (notEmptyRegion.contains(next.center())) {
                            continue;
                        }

                        isEmptyPos = true;

                        setFileMetaInfoPos(index.data(Qt::UserRole).toString(), next.topLeft());
                        notEmptyRegion += next;
                    }
                }
                else{
                    if (layoutDirection() == Qt::RightToLeft) {
                        int vewportX = viewport()->rect().topRight().x();
                        int x = vewportX - rect.topRight().x() ;
                        rect = QRect(x, rect.y(), rect.width(), rect.height());
                    }
                     setFileMetaInfoPos(index.data(Qt::UserRole).toString(), rect.topLeft());
                     notEmptyRegion += rect;
                }
            }

            Q_EMIT updateView();
            Q_EMIT view->updateView();
            return bDropToOtherScreen;
        }
    }
    return false;
}

QItemSelectionModel::SelectionFlags DesktopIconView::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    if (!event)
        return QListView::selectionCommand(index, event);

    if (event->type() == QEvent::MouseButtonPress) {
        auto e = static_cast<const QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton && e->modifiers() & Qt::ControlModifier && selectedIndexes().contains(index)) {
            return QItemSelectionModel::NoUpdate;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto e = static_cast<const QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton && e->modifiers() & Qt::ControlModifier) {
            QItemSelectionModel::SelectionFlags flags;
            if (m_noSelectOnPress) {
                flags = QItemSelectionModel::Deselect;
            }
            return flags;
        }
    }
    return QListView::selectionCommand(index, event);
}

int DesktopIconView::radius() const
{
    return m_radius;
}

void DesktopIconView::saveExtendItemInfo()
{
    if (!m_proxy_model)
        return;
    //task#74174 扩展屏的元素记录到metInfo,以便以后恢复
    for (int i = 0; i < m_proxy_model->rowCount(); i++) {
        auto index = m_proxy_model->index(i, 0);
        auto indexRect = visualRectInRightToLeft(index);
        QStringList topLeft;
        topLeft<<QString::number(indexRect.top());
        topLeft<<QString::number(indexRect.left());
        topLeft<<QString::number(m_id);
        QString uri = index.data(Qt::UserRole).toString();
        if (0 != m_id && m_model) {
            m_model->m_destoryItems.append(uri);
        }
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            qDebug() << "uri:"<<uri<<" topLeft:"<<topLeft;
            QStringList extendPos = metaInfo->getMetaInfoStringList(RESTORE_EXTEND_ITEM_POS_ATTRIBUTE);
            if (extendPos.count() == 3) {
                continue;
            }
            QStringList pos = metaInfo->getMetaInfoStringList(RESTORE_SINGLESCREEN_ITEM_POS_ATTRIBUTE);
            if (pos.count() == 2) {
                metaInfo->setMetaInfoStringList(ITEM_POS_ATTRIBUTE, pos);
                QStringList tmp;
                tmp<<"";
                metaInfo->setMetaInfoStringList(RESTORE_SINGLESCREEN_ITEM_POS_ATTRIBUTE, tmp);
            }
            metaInfo->setMetaInfoStringList(RESTORE_EXTEND_ITEM_POS_ATTRIBUTE, topLeft);
            metaInfo->setMetaInfoInt("explorer-qt-desktop-id", 0);
        }
    }
}

bool DesktopIconView::isFull()
{
    //bug#123045 主屏桌面空间不足，新建的文件未放置到扩展屏桌面上
    int colNum = viewport()->width()/gridSize().width();
    int rowNum = viewport()->height()/gridSize().height();

    if (m_proxy_model->rowCount() > 1) {
        auto index = m_proxy_model->index(0, 0);
        auto indexRect = visualRectInRightToLeft(index);
        auto left = viewport()->width() - colNum * gridSize().width() - offset().x();
        if (left >= indexRect.width()) {
            colNum++;
        }
        auto bottom = viewport()->height() - rowNum * gridSize().height() - offset().y() - getViewRect().y();
        if (bottom >= indexRect.height()) {
            rowNum++;
        }
    }

    qDebug() << "colNum:" <<colNum << "rowNum:"<< rowNum << "total:" << m_proxy_model->rowCount();
    if (colNum * rowNum <= m_proxy_model->rowCount()) {
        return true;
    }
    return false;
}

void DesktopIconView::resetExtendItemInfo()
{
    if (!m_model)
        return;

    //bug#108126 一开始加载将上一次超出屏幕的元素恢复
    for (int i = 0; i < m_model->rowCount(); i++) {
        auto index = m_model->index(i, 0);
        QString uri = index.data(Qt::UserRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        m_model->m_destoryItems.removeOne(uri);
        if (metaInfo) {
            QStringList list = metaInfo->getMetaInfoStringList(RESTORE_EXTEND_ITEM_POS_ATTRIBUTE);
            QStringList pos = metaInfo->getMetaInfoStringList(ITEM_POS_ATTRIBUTE);
            if (list.count() == 3) {
                QString id = list.takeLast();
                qDebug() << "[DesktopIconView::resetExtendItemInfo] uri:"<<uri<<" explorer-qt-desktop-restore-extend-item-position:"<<list<<" id:"<<m_id;
                if (id.toInt() != m_id) {
                    continue;
                }
                metaInfo->setMetaInfoStringList(ITEM_POS_ATTRIBUTE, list);
                metaInfo->setMetaInfoInt("explorer-qt-desktop-id", m_id);
                QStringList tmp;
                tmp<<"";
                metaInfo->setMetaInfoStringList(RESTORE_EXTEND_ITEM_POS_ATTRIBUTE, tmp);
                qDebug() << "[DesktopIconView::resetExtendItemInfo] need relayout:"<<m_model->m_items_need_relayout;
            }
            QStringList screenlist = metaInfo->getMetaInfoStringList(RESTORE_SINGLESCREEN_ITEM_POS_ATTRIBUTE);
            if (screenlist.count() != 2) {
                metaInfo->setMetaInfoStringList(RESTORE_SINGLESCREEN_ITEM_POS_ATTRIBUTE, pos);
            }
        }
    }
}

void DesktopIconView::clearItemRect()
{
    m_resolution_item_rect.clear();
    m_item_rect_hash.clear();
    m_storageBox.clear();
}

void DesktopIconView::clearExtendItemPos(bool saveId)
{
    if (!m_proxy_model)
        return;
    //重置扩展屏和单屏坐标
    qDebug() << "primary screen id:" << m_id ;
    for (int i = 0; i < m_proxy_model->rowCount(); i++) {
        auto index = m_proxy_model->index(i, 0);
        QString uri = index.data(Qt::UserRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            QStringList tmp;
            tmp<<"";
            metaInfo->setMetaInfoStringList(RESTORE_SINGLESCREEN_ITEM_POS_ATTRIBUTE, tmp);
            if (saveId) {
                metaInfo->setMetaInfoStringList(RESTORE_ITEM_POS_ATTRIBUTE, tmp);
                QStringList extendPos = metaInfo->getMetaInfoStringList(RESTORE_EXTEND_ITEM_POS_ATTRIBUTE);
                if (extendPos.count() == 3) {
                    tmp.clear();
                    tmp<<"-1"<<"-1"<<extendPos[2];
                }
            }
            metaInfo->setMetaInfoStringList(RESTORE_EXTEND_ITEM_POS_ATTRIBUTE, tmp);
        }
    }
}

QRect DesktopIconView::getDataRect(const QModelIndex &index)
{
    DesktopIconViewDelegate *delegate = qobject_cast<DesktopIconViewDelegate *>(itemDelegate());
    QStyleOptionViewItem opt = viewOptions();
    delegate->initStyleOption(&opt, index);
    opt.rect = visualRectInRightToLeft(index);
    QWidget *widget = indexWidget(index);
    QFont font = qApp->font();
    auto fm = QFontMetrics(font);
    int lineSpacing = fm.lineSpacing();
    int textHeight = lineSpacing + 5;
    QRect iconRect = style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, widget);
    QRect rect = opt.rect;
    rect.setHeight(iconRect.height() + textHeight);
    return rect;
}

void DesktopIconView::modifyGridSize()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings) {
        QSize sizeFromConfig = settings->getValue(DEFAULT_GRID_SIZE).toSize();
        if (!sizeFromConfig.isEmpty() && sizeFromConfig.width() > MINGRIDSIZE && sizeFromConfig.width() < MAXGRIDSIZE && sizeFromConfig.height() > MINGRIDSIZE && sizeFromConfig.height() < MAXGRIDSIZE) {
            setGridSize(sizeFromConfig);
            return;
        }
    }
    if (0 == m_item_rect_hash.size()) {
        return;
    }
    QList<int> positionX;
    QList<int> positionY;
    for (auto i = m_item_rect_hash.constBegin(); i != m_item_rect_hash.constEnd(); ++i) {
        QRect itemRect = i.value();
        positionX << itemRect.x();
        positionY << itemRect.y();
    }
    int gridWidth = getGreatestCommonDivisor(positionX);
    int gridHeight = getGreatestCommonDivisor(positionY);
    QSize size = QSize(gridWidth,gridHeight);
    if (gridWidth < MINGRIDSIZE || gridHeight < MINGRIDSIZE || gridWidth > MAXGRIDSIZE || gridHeight > MAXGRIDSIZE ) {
       //重新排序
        setDefaultZoomLevel(zoomLevel());
        Q_EMIT updateView();
        if (settings) {
            settings->setValue(DEFAULT_GRID_SIZE, gridSize());
        }
        return ;
    }

    if (settings) {
        settings->setValue(DEFAULT_GRID_SIZE, size);
    }
    Q_EMIT resetGridSize(size);
}

bool DesktopIconView::verifyBoundaries(const QRect &rect, Direction direction)
{
    QRect dataRect = rect;
    if (!m_item_rect_hash.isEmpty() && m_item_rect_hash.first().size().isValid()) {
        QSize size = gridSize().height() > m_item_rect_hash.first().size().height() ? m_item_rect_hash.first().size() : gridSize();
        dataRect.setSize(size);
    }
    dataRect.moveTo(dataRect.topLeft() + getViewRect().topLeft());

    if (Direction::Bottom != direction) {
        if (dataRect.right() > this->viewport()->rect().right()) {
            return true;
        }
    }
    if (Direction::Right != direction){
        if (dataRect.bottom() > this->viewport()->rect().bottom()) {
            return true;
        }
    }
    if (!this->viewport()->rect().contains(dataRect)) {
        return true;
    }
    return false;
}

void DesktopIconView::initViewport()
{
    if (!m_initialized) {
        qInfo()<<"desktop icon view model inited";

        if (!QGSettings::isSchemaInstalled(PANEL_SETTINGS)) {
            m_initialized = true;
            return;
        }
        //panel
        if (!m_panelSetting)
            m_panelSetting = new QGSettings(PANEL_SETTINGS, QByteArray(), this);

        int position = m_panelSetting->get("panelposition").toInt();
        int margins = m_panelSetting->get("panelsize").toInt();

        setMarginsBasedOnPosition(position,  margins);

        getAllRestoreInfo();
        modifyGridSize();
        resolutionChange();
        setAllRestoreInfo();
        m_initialized = true;
    }
}

QRect DesktopIconView::visualRectInRightToLeft(const QModelIndex &index)
{
    QRect rect = QListView::visualRect(index);
    if (layoutDirection() == Qt::RightToLeft) {
        int vewportX = viewport()->rect().topRight().x();
        int x = vewportX - rect.topRight().x() ;
        rect = QRect(x, rect.y(), rect.width(), rect.height());
    }
    return rect;
}

static bool iconSizeLessThan (const QPair<QRect, QString>& p1, const QPair<QRect, QString>& p2)
{
    if (p1.first.x() > p2.first.x())
        return false;

    if (p1.first.x() < p2.first.x())
        return true;

    if ((p1.first.x() == p2.first.x()))
        return p1.first.y() < p2.first.y();

    return true;
}

static bool posLessThan (const int& p1, const int& p2)
{
    if (p1 > p2) {
        return false;
    }

    return true;
}
