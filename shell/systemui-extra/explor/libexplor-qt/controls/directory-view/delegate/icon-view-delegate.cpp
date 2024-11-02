/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "icon-view-delegate.h"
#include "icon-view.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-item.h"
#include "file-info.h"
#include "file-info-job.h"

#include "file-operation-manager.h"
#include "file-rename-operation.h"
#include "file-batch-rename-operation.h"

#include "emblem-provider.h"

#include <QDebug>
#include <QLabel>

#include <QPainter>
#include <QPalette>

#include <QTextCursor>
#include <QGraphicsTextItem>
#include <QFont>

#include <QStyle>
#include <QApplication>
#include <QPainter>
#include <QDBusReply>

#include "icon-view-editor.h"
#include "icon-view-index-widget.h"

#include <QPushButton>

#include "clipboard-utils.h"
#include "global-settings.h"

#include <QTextLayout>
#include <QFileInfo>

#include <QStyleOptionViewItem>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>

using namespace Peony;
using namespace Peony::DirectoryView;

IconViewDelegate::IconViewDelegate(QObject *parent) : QStyledItemDelegate (parent)
{
    m_styled_button = new QPushButton;
    m_isStartDrag = false;
    //m_watcher = new QFileSystemWatcher;
}

IconViewDelegate::~IconViewDelegate()
{
    m_styled_button->deleteLater();
}

QSize IconViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //QStyleOptionViewItem opt = option;
    //initStyleOption(&opt, index);

    auto view = qobject_cast<IconView*>(this->parent());
    auto iconSize = view->iconSize();
    auto fm = qApp->fontMetrics();
    int width = iconSize.width() + 41 - 4;
    int height = iconSize.height() + fm.ascent()*2 + 20 + 10;
    return QSize(width, height);
    /*
    qDebug()<<option;
    qDebug()<<option.font;
    qDebug()<<option.icon;
    qDebug()<<option.text;
    qDebug()<<option.widget;
    qDebug()<<option.decorationSize;
    qDebug()<<QStyledItemDelegate::sizeHint(option, index);
    */
}

void IconViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //FIXME: how to deal with word wrap correctly?

    bool isDragging = false;
    auto view = qobject_cast<IconView*>(this->parent());
    if (view->state() == IconView::DraggingState) {
        isDragging = true;
        if (view->selectionModel()->selection().contains(index)) {
            painter->setOpacity(0.8);
        }
    }


    //get file info from index
    auto model = static_cast<FileItemProxyFilterSortModel*>(view->model());
    auto item = model->itemFromIndex(index);
    //NOTE: item might be deleted when painting, because we might start a
    //location change during the painting.
    if (!item) {
        return;
    }

#ifdef LINGMO_UDF_BURN
    /* R类型光盘，所有用于刻录的文件（夹）展示在挂载点时都应该半透明显示，区别于普通文件 ,linkto task#122470 */
    if(item->property("isFileForBurning").toBool()){
        painter->setOpacity(0.5);
    }else{
        painter->setOpacity(1.0);
    }
#endif

    //default painter
    //QStyledItemDelegate::paint(painter, option, index);
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    //qDebug()<<option.widget->style();
    //qDebug()<<option.widget;
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    auto style = QApplication::style();

    auto rawDecoSize = opt.decorationSize;
    opt.decorationSize = view->iconSize();
    if (qApp->devicePixelRatio() != 1.0) {
        opt.rect.adjust(1, 1, -1, -1);
    }

    int horizalMargin = 2;
    auto fontMetrics = opt.fontMetrics;
    int pixelsWide = fontMetrics.width(opt.text);
    int width = opt.rect.width() - 2*horizalMargin;

    if(pixelsWide < width){
       opt.rect = opt.rect.adjusted(0,0,0,-31);
    }

    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, nullptr);
    opt.decorationSize = rawDecoSize;

    bool bCutFile = false;
    auto clipedUris = ClipboardUtils::getInstance()->getCutFileUris();
    if (!clipedUris.isEmpty()){
        QString actualDirUri = view->getDirectoryUri();
        bool bSearchTab = false;
        if(actualDirUri.startsWith("search:///search_uris")){
            bSearchTab = true;
            actualDirUri = FileUtils::getActualDirFromSearchUri(actualDirUri);
        }

        QString clipedFilesParentUri = ClipboardUtils::getClipedFilesParentUri();
        if (!clipedUris.isEmpty() && (FileUtils::isSamePath(clipedFilesParentUri, actualDirUri) || (bSearchTab && clipedFilesParentUri.startsWith(actualDirUri)))) {
            if (clipedUris.contains(index.data(FileItemModel::UriRole).toString())) {
                painter->setOpacity(0.5);
                bCutFile = true;
                qDebug()<<"cut item"<<index.data();
            }else{
                //fix bug#145085, same logic to list view
                painter->setOpacity(1.0);
            }
        }
    }

    auto iconSizeExpected = view->iconSize();
    auto iconRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    int y_delta = iconSizeExpected.height() - iconRect.height();
    opt.rect.setY(opt.rect.y() + y_delta);

    auto text = opt.text;
    opt.text = nullptr;
    auto state = opt.state;
    //bug#99340,修改图标选中状态，会变暗
    if((opt.state & QStyle::State_Enabled) && (opt.state & QStyle::State_Selected) && !m_isStartDrag)
    {
        opt.state &= ~QStyle::State_Selected;
    }
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    painter->restore();
    opt.state = state;
    opt.text = text;

    auto rect = view->visualRect(index);

    bool useIndexWidget = false;
    if (view->selectedIndexes().count() == 1 && view->selectedIndexes().first() == index && !bCutFile) {
        useIndexWidget = true;
        if (view->indexWidget(index)) {
        } else if (! view->isDraggingState() && view->m_allow_set_index_widget && !(view->m_ctrl_key_pressed)) {
            IconViewIndexWidget *indexWidget = new IconViewIndexWidget(this, option, index, getView());
            connect(getView()->m_model, &FileItemModel::dataChanged, indexWidget, [=](const QModelIndex &topleft, const QModelIndex &bottomRight){
                // if item has been removed and there is no reference for responding info,
                // clear index widgets.
                if (!indexWidget->m_info.lock()) {
                    getView()->clearIndexWidget();
                    return;
                }
                if (topleft.data(Qt::UserRole).toString() == indexWidget->m_info.lock().get()->uri()) {
                    if (getView()->getSelections().count() == 1 && getView()->getSelections().first() == topleft.data(Qt::UserRole).toString()) {
                        auto selections = getView()->getSelections();
                        getView()->clearSelection();
                        getView()->setSelections(selections);
                    }
                }
            });
            view->setIndexWidget(index, indexWidget);
            indexWidget->adjustPos();
            indexWidget->update();

            auto model = static_cast<FileItemProxyFilterSortModel*>(view->model());
            auto item = model->itemFromIndex(index);
            QString tmpUri = item->info().get()->uri();
            connect(getView()->m_model, &FileItemModel::thumbnailUpdated, indexWidget, [=](QString uri){
                if (getView()->getSelections().count() == 1
                        && view->selectedIndexes().first() == index
                        && tmpUri == uri) {
                       Q_EMIT updateIndexWidget(option);
                   }
            });
//            QString itemPath = item->info().get()->filePath();
//            m_watcher->addPath(itemPath);
//            connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, [=](){
//                if (getView()->getSelections().count() == 1 && view->selectedIndexes().first() == index) {
//                    Q_EMIT updateIndexWidget(option);
//                }
//            });
//            connect(m_watcher, &QFileSystemWatcher::fileChanged, this, [=](){
//                if (getView()->getSelections().count() == 1 && view->selectedIndexes().first() == index) {
//                    Q_EMIT updateIndexWidget(option);
//                }
//            });
//            connect(indexWidget, &IconViewIndexWidget::destroyed, this, [=](){
//                m_watcher->removePath(itemPath);
//            });
        }
    }

    //fix bug#46785, select one file cut has no effect issue
    if (bCutFile && !getView()->getDelegateEditFlag())/* Rename is index is not set to nullptr,link to bug#61119.modified by 2021/06/22 */
        view->setIndexWidget(index, nullptr);

    auto info = item->info();
    // draw color symbols
    int yoffset = 0;
    auto colors = info->getColors();

    int xoffset = 0;

    if (!isDragging || !view->selectedIndexes().contains(index)) {
        //快速访问目录，颜色标记设置后更新不及时问题单独处理,修复bug#118015
        if(info->uri().startsWith("favorite://")){/* 快速访问须特殊处理 */
            auto matchInfo = FileInfo::fromUri(FileUtils::getEncodedUri(FileUtils::getTargetUri(info->uri())));
            colors = matchInfo->getColors();
        }

        if(0 < colors.count())
        {
            const int MAX_LABEL_NUM = 3;
            int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
            int num =  colors.count() - startIndex;

            auto lineSpacing = option.fontMetrics.lineSpacing();

            QString text = opt.text;
            QFont font = opt.font;
            QTextLayout textLayout(text, font);

            QTextOption textOpt;
            textOpt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

            textLayout.setTextOption(textOpt);
            textLayout.beginLayout();

            QTextLine line = textLayout.createLine();
            if (!line.isValid())
                return;
            int width = opt.rect.width() - (num+1)*6 - 2*2 - 4;
            line.setLineWidth(width);
            xoffset = (width - line.naturalTextWidth())/2 ;
            if(xoffset < 0)
            {
                xoffset = 2;
            }
            yoffset = (lineSpacing -12 )/2+2;
            for (int i = startIndex; i < colors.count(); ++i) {
                auto color = colors.at(i);
                painter->save();
                //fix bug#147348
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                painter->translate(opt.rect.topLeft());
                painter->translate(0, iconRect.size().height() + 5);
                painter->setPen(opt.palette.highlightedText().color());
                painter->setBrush(color);
                painter->drawEllipse(QRectF(xoffset, yoffset, 12, 12));
                painter->restore();
                xoffset += 12/2;
            }

            yoffset = 0;
            xoffset += 10;

            textLayout.endLayout();
        }
    }

    painter->save();
    painter->translate(opt.rect.topLeft());
    painter->translate(0, iconRect.size().height() + 5);
    IconViewTextHelper::paintText(painter,
                                  opt,
                                  9999,
                                  xoffset,
                                  m_regFindKeyWords,
                                  2,
                                  2);

    painter->restore();

    QList<int> emblemPoses = {4, 3, 2, 1}; //bottom right, bottom left, top right, top left

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    //paint symbolic link emblems
    if (info->isSymbolLink()) {
        emblemPoses.removeOne(3);
        QIcon icon = QIcon::fromTheme("emblem-link-symbolic");
        //qDebug()<<info->symbolicIconName();
        //icon.paint(painter, rect.x() + rect.width() - 30, rect.y() + 10, 20, 20, Qt::AlignCenter);
        //Adjust link emblem to topLeft.link story#8354
        icon.paint(painter, rect.x() + 10, opt.rect.y() + opt.decorationSize.height() - 10, 20, 20, Qt::AlignCenter);
    }

    if(view->isEnableMultiSelect()) {
        if(view->selectedIndexes().contains(index)) {
            QIcon icon = QIcon(":/icons/icon-selected.png");
            icon.paint(painter, rect.x()+rect.width() - 20, rect.y()+4, 16, 16, Qt::AlignCenter);
        } else {
            QIcon icon = QIcon(":/icons/icon-select.png");
            icon.paint(painter, rect.x()+rect.width() - 20, rect.y()+4, 16, 16, Qt::AlignCenter);
        }
    }

    //paint access emblems
    //NOTE: we can not query the file attribute in smb:///(samba) and network:///.
    if (info->uri().startsWith("file:")) {
        if (!info->canRead()) {
            emblemPoses.removeOne(1);
            QIcon icon = QIcon::fromTheme("emblem-unreadable");
            icon.paint(painter, rect.x() + 10, rect.y() + 10, 20, 20);
        } else if (!info->canWrite()/* && !info->canExecute()*/) {
            //只读图标对应可读不可写情况，与可执行权限无关，link to bug#99998
            emblemPoses.removeOne(1);
            QIcon icon = QIcon::fromTheme("emblem-readonly");
            icon.paint(painter, rect.x() + 10, rect.y() + 10, 20, 20);
        }
    }

    // paint extension emblems, FIXME: adjust layout, and implemet on indexwidget, other view.
    auto extensionsEmblems = EmblemProviderManager::getInstance()->getAllEmblemsForUri(info->uri());

    for (auto extensionsEmblem : extensionsEmblems) {
        if (emblemPoses.isEmpty()) {
            break;
        }

        QIcon icon = QIcon::fromTheme(extensionsEmblem);
        if (!icon.isNull()) {
            int pos = emblemPoses.takeFirst();
            switch (pos) {
            case 1: {
                icon.paint(painter, rect.x() + 10, rect.y() + 10, 20, 20, Qt::AlignCenter);
                break;
            }
            case 2: {
                icon.paint(painter, rect.x() + rect.width() - 30, rect.y() + 10, 20, 20, Qt::AlignCenter);
                break;
            }
            case 3: {
                icon.paint(painter, rect.x() + 10, opt.rect.y() + opt.decorationSize.height() - 10, 20, 20, Qt::AlignCenter);
                break;
            }
            case 4: {
                icon.paint(painter, rect.right() - 30, opt.rect.y() + opt.decorationSize.height() - 10, 20, 20, Qt::AlignCenter);
                break;
            }
            default:
                break;
            }
        }
    }
    painter->restore();

    //single selection, we have to repaint the emblems.

}

void IconViewDelegate::setCutFiles(const QModelIndexList &indexes)
{
    m_cut_indexes = indexes;
}

QWidget *IconViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto edit = new IconViewEditor(parent);
    edit->setContentsMargins(0, 0, 0, 0);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto size = sizeHint(option, index);
    edit->setMinimumWidth(size.width());
    edit->setMinimumHeight(size.height() - getView()->iconSize().height() - 5);

    edit->blockSignals(true);
    auto displayString = index.data(Qt::DisplayRole).toString();
    auto displayName = index.data(Qt::UserRole + 1).toString();
    auto uri = index.data(Qt::UserRole).toString();
    auto suffix = displayName.remove(displayString);
    auto fsType = FileUtils::getFsTypeFromFile(uri);
    auto info = FileInfo::fromUri(uri);
    if (info->isDesktopFile()) {
        suffix = ".desktop";
    }
    if (FileUtils::isFuseFileSystem(uri)) {
        fsType = "fuse.kyfs";
    }
    if (fsType.contains("ext")) {
        edit->setMaxLengthLimit(255 - suffix.toLocal8Bit().length());
    } else if (fsType.contains("ntfs")) {
        edit->setLimitBytes(false);
        edit->setMaxLengthLimit(255 - suffix.length());
    } else if (fsType.contains("fuse.kyfs")) {
        int32_t maxLength = 255;
        edit->setLimitBytes(false);
        QDBusInterface iface ("com.lingmo.file.system.fuse","/com/lingmo/file/system/fuse","com.lingmo.file.system.fuse",QDBusConnection::systemBus());
        QDBusReply<int32_t> reply = iface.call("GetFilenameLength");
        if (reply.isValid()) {
            maxLength = reply.value();
        }
        edit->setMaxLengthLimit(maxLength - suffix.length());
    }
    edit->setText(displayString);
    edit->blockSignals(false);

    edit->setAlignment(Qt::AlignCenter);
    //NOTE: if we directly call this method, there will be
    //nothing happen. add a very short delay will ensure that
    //the edit be resized.
    QTimer::singleShot(1, edit, [=]() {
        edit->minimalAdjust();
    });

    connect(edit, &IconViewEditor::returnPressed, this, &IconViewDelegate::slot_finishEdit);

    connect(edit, &QWidget::destroyed, this, [=]() {
        // NOTE: resort view after edit closed.
        // it's because if we not, the viewport might
        // not be updated in some cases.
        Q_EMIT isEditing(false);     
#if QT_VERSION > QT_VERSION_CHECK(5, 12, 0)
        QTimer::singleShot(100, this, [=]() {
#else
        QTimer::singleShot(100, [=]() {
#endif
            auto model = qobject_cast<QSortFilterProxyModel*>(getView()->model());
            //fix rename file back to default sort order
            //model->sort(-1, Qt::SortOrder(getView()->getSortOrder()));
            //model->sort(getView()->getSortType(), Qt::SortOrder(getView()->getSortOrder()));
        });
    });

    return edit;
}

void IconViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    IconViewEditor *edit = qobject_cast<IconViewEditor*>(editor);
    if (!edit)
        return;

    Q_EMIT isEditing(true);
    auto cursor = edit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    bool isDir = FileUtils::getFileIsFolder(index.data(Qt::UserRole).toString());
    bool isDesktopFile = index.data(Qt::UserRole).toString().endsWith(".desktop");
    bool isSoftLink = FileUtils::getFileIsSymbolicLink(index.data(Qt::UserRole).toString());
    if (!isDesktopFile && !isSoftLink && !isDir && edit->toPlainText().contains(".") && !edit->toPlainText().startsWith(".")) {
        int n = 1;
        if(index.data(Qt::DisplayRole).toString().contains(".tar.")) //ex xxx.tar.gz xxx.tar.bz2
            n = 2;
        while(n){
            cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor, 1);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
            --n;
        }
    }
    //qDebug()<<cursor.anchor();
    edit->setTextCursor(cursor);
}

void IconViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    auto edit = qobject_cast<IconViewEditor*>(editor);
    if (!edit)
        return;

    auto opt = option;
    auto iconExpectedSize = getView()->iconSize();
    //auto iconRect = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    //auto y_delta = iconExpectedSize.height() - iconRect.height();
    //edit->move(opt.rect.x(), opt.rect.y() + y_delta + 10);
    edit->move(opt.rect.x(), opt.rect.y() + iconExpectedSize.height() + 5);

    edit->resize(edit->document()->size().width(), edit->document()->size().height() + 10);
}

void IconViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    IconViewEditor *edit = qobject_cast<IconViewEditor*>(editor);
    if (!edit)
        return;
    auto newName = edit->toPlainText();
    auto oldName = index.data(Qt::DisplayRole).toString();
    if (newName.isNull())
        return;
    //process special name . or .. or only space
    if (newName == "." || newName == ".." || newName.trimmed() == "")
        newName = "";
    //comment new name != suffix check to fix feedback issue
    if (newName.length() >0 && newName != oldName/* && newName != suffix*/) {
        if (getView()->getSelections().count() > 1) {
            auto fileOpMgr = FileOperationManager::getInstance();
            QStringList uris = getView()->getSelections();
            auto renameOp = new FileBatchRenameOperation(uris, newName);
            connect(renameOp, &FileBatchRenameOperation::operationFinished, getView(), [=](){
                auto info = renameOp->getOperationInfo().get();
                auto uri = info->target();
                QTimer::singleShot(100, getView(), [=](){
                    auto infoJob = new Peony::FileInfoJob(Peony::FileInfo::fromUri(uri));
                    infoJob->setAutoDelete();
                    connect(infoJob, &Peony::FileInfoJob::queryAsyncFinished, this, [=]() {
                        getView()->setSelections(QStringList()<<uri);
                        getView()->scrollToSelection(uri);
                        //set focus to fix bug#54061
                        getView()->setFocus();
                    });
                    infoJob->queryAsync();
                });
            }, Qt::BlockingQueuedConnection);

            fileOpMgr->startOperation(renameOp, true);
        } else {
            auto fileOpMgr = FileOperationManager::getInstance();
            auto renameOp = new FileRenameOperation(index.data(FileItemModel::UriRole).toString(), newName);
            connect(renameOp, &FileRenameOperation::operationFinished, getView(), [=](){
                auto info = renameOp->getOperationInfo().get();
                auto uri = info->target();
                QTimer::singleShot(100, getView(), [=](){
                    auto infoJob = new Peony::FileInfoJob(Peony::FileInfo::fromUri(uri));
                    infoJob->setAutoDelete();
                    connect(infoJob, &Peony::FileInfoJob::queryAsyncFinished, this, [=]() {
                        getView()->setSelections(QStringList()<<uri);
                        getView()->scrollToSelection(uri);
                        //set focus to fix bug#54061
                        getView()->setFocus();
                    });
                    infoJob->queryAsync();
                });
            }, Qt::BlockingQueuedConnection);

            fileOpMgr->startOperation(renameOp, true);
        }
    }
    else if (newName == oldName)
    {
        //create new file, should select the file or folder
        getView()->selectionModel()->select(index, QItemSelectionModel::Select);
        //set focus to fix bug#54061
        getView()->setFocus();
    }
}

void IconViewDelegate::setIndexWidget(const QModelIndex &index, QWidget *widget) const
{
    auto view = qobject_cast<IconView*>(this->parent());
    view->setIndexWidget(index, widget);
}

void IconViewDelegate::slot_finishEdit()
{
    auto edit = qobject_cast<QWidget *>(sender());
    commitData(edit);
    closeEditor(edit, QAbstractItemDelegate::SubmitModelCache);
    if(edit){
        delete edit;
        edit = nullptr;
    }
    Q_EMIT isEditing(false);
}

void IconViewDelegate::setStartDrag(bool isDrag)
{
    m_isStartDrag = isDrag;
}

void IconViewDelegate::initIndexOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    initStyleOption(option, index);
}

IconView *IconViewDelegate::getView() const
{
    return qobject_cast<IconView*>(parent());
}

const QBrush IconViewDelegate::selectedBrush() const
{
    return m_styled_button->palette().highlight();
}

void IconViewDelegate::setSearchKeyword(QString regFindKeyWords)
{
    m_regFindKeyWords = regFindKeyWords;
}

const QString IconViewDelegate::getRegFindKeyWords() const
{
    return m_regFindKeyWords;
}

void IconViewTextHelper::paintText(QPainter *painter, const QStyleOptionViewItem &option, int textMaxHeight, int xOffset, const QString &regFindKeyWords, int horizalMargin, int maxLineCount)
{
    painter->save();
    QFont font = option.font;
    QTextLayout textLayout(option.text, font);
    QTextOption textOpt;
    textOpt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textLayout.setTextOption(textOpt);
    textLayout.beginLayout();

    auto fontMetrics = option.fontMetrics;
    auto lineSpacing = fontMetrics.lineSpacing();
    int width = option.rect.width() - 2*horizalMargin;
    int y = 0;
    int lineCount = 0;
    QString elidedText = option.text;

    QTextDocument document;
    textOpt.setAlignment(Qt::AlignHCenter);
    document.setDefaultTextOption(textOpt);
    document.setTextWidth(width);
    document.setDefaultFont(option.font);
    document.setIndentWidth(0);
    document.setDocumentMargin(0);

    bool isElided= false;
    //计算text的长度
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        int nextLineY = y + lineSpacing;
        lineCount++;
        y = nextLineY;
        if (1 == lineCount) {
           line.setLineWidth(width-xOffset);
           xOffset = xOffset > 0 ? xOffset - (width - xOffset - line.naturalTextWidth()) : 0;
           xOffset = xOffset > 0 ? xOffset : 0;
        } else {
            line.setLineWidth(width);
        }
        if (textMaxHeight < nextLineY + lineSpacing || lineCount == maxLineCount) {
            QString lastLine = option.text.mid(line.textStart());
            QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width);
            if (elidedLastLine != lastLine) {
                elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width - 2);
                isElided = true;
            }
            elidedText = option.text.left(line.textStart()) + elidedLastLine;
            textOpt.setWrapMode(QTextOption::NoWrap);
            line = textLayout.createLine();
            break;
        }
    }
    document.setPlainText(elidedText);

    painter->translate(horizalMargin, 0);

    //设置关键字高亮
    QTextCursor highlightCursor(&document);
    QTextCursor cursor(&document);

    cursor.beginEditBlock();
    QTextBlock textStyleBlock = cursor.block();
    QTextBlockFormat textStyleFormat = textStyleBlock.blockFormat();
    textStyleFormat.setLineHeight(lineSpacing, QTextBlockFormat::FixedHeight);
    textStyleFormat.setTextIndent(xOffset);
    cursor.setBlockFormat(textStyleFormat);
    QTextCharFormat plainFormat(highlightCursor.charFormat());
    QTextCharFormat colorFormat = plainFormat;
    colorFormat.setBackground(Qt::green);
    if (option.state.testFlag(QStyle::State_Selected)) {
        QTextCharFormat selectColorFormat(cursor.charFormat());
        selectColorFormat.setForeground(Qt::white);
        cursor.select(QTextCursor::Document);
        cursor.mergeCharFormat(selectColorFormat);
    }

    if (!regFindKeyWords.isEmpty()) {
        //对特殊字符进行处理
        QString escapedKeywords = QRegularExpression::escape(regFindKeyWords);
        QRegularExpression regex(escapedKeywords, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator matchIterator = regex.globalMatch(option.text);

        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            int startPos = match.capturedStart();
            int endPos = match.capturedEnd();
          int oo = elidedText.size();
            // 判断是否关键字被省略
            if (isElided && startPos >= elidedText.size() - 1) {
                break; // 关键字被完全省略，退出循环
            }

            // 调整关键字的结束位置
            if (endPos > elidedText.size()) {
                endPos = elidedText.size() ; // 关键字的一部分被省略，将结束位置调整为最后一个字符的位置
            }

            // 执行关键字高亮
            highlightCursor.setPosition(startPos);
            highlightCursor.setPosition(endPos, QTextCursor::KeepAnchor);
            highlightCursor.mergeCharFormat(colorFormat);
        }
    }
    cursor.endEditBlock();
    document.drawContents(painter);

    textLayout.endLayout();
    painter->restore();
}

qreal IconViewTextHelper::drawText(QPainter *painter, const QStyleOptionViewItem &option, int textMaxHeight, int xOffset, const QString &regFindKeyWords, int horizalMargin, int maxLineCount)
{
    painter->save();
    QFont font = option.font;
    QTextLayout textLayout(option.text, font);
    QTextOption textOpt;
    textOpt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textLayout.setTextOption(textOpt);
    textLayout.beginLayout();

    auto fontMetrics = option.fontMetrics;
    auto lineSpacing = fontMetrics.lineSpacing();
    int width = option.rect.width() - 2*horizalMargin;
    int y = 0;
    int lineCount = 0;
    QString elidedText = option.text;

    QTextDocument document;
    textOpt.setAlignment(Qt::AlignHCenter);
    document.setDefaultTextOption(textOpt);
    document.setTextWidth(width);
    document.setDefaultFont(option.font);
    document.setIndentWidth(0);
    document.setDocumentMargin(0);

    bool isElided= false;
    //计算text的长度
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        int nextLineY = y + lineSpacing;
        lineCount++;
        y = nextLineY;
        if (1 == lineCount) {
           line.setLineWidth(width-xOffset);
           xOffset = xOffset > 0 ? xOffset - (width - xOffset - line.naturalTextWidth()) : 0;
           xOffset = xOffset > 0 ? xOffset : 0;
        } else {
            line.setLineWidth(width);
        }
        if (textMaxHeight < nextLineY + lineSpacing || lineCount == maxLineCount) {
            QString lastLine = option.text.mid(line.textStart());
            QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width);
            if (elidedLastLine != lastLine) {
                isElided = true;
            }
            elidedText = option.text.left(line.textStart()) + elidedLastLine;
            textOpt.setWrapMode(QTextOption::NoWrap);
            line = textLayout.createLine();
            break;
        }
    }
    document.setPlainText(elidedText);

    painter->translate(horizalMargin, 0);

    //设置关键字高亮
    QTextCursor highlightCursor(&document);
    QTextCursor cursor(&document);

    cursor.beginEditBlock();
    QTextBlock textStyleBlock = cursor.block();
    QTextBlockFormat textStyleFormat = textStyleBlock.blockFormat();
    textStyleFormat.setLineHeight(lineSpacing, QTextBlockFormat::FixedHeight);
    textStyleFormat.setTextIndent(xOffset);
    cursor.setBlockFormat(textStyleFormat);
    QTextCharFormat plainFormat(highlightCursor.charFormat());
    QTextCharFormat colorFormat = plainFormat;
    colorFormat.setBackground(Qt::green);
    if (option.state.testFlag(QStyle::State_Selected)) {
        QTextCharFormat selectColorFormat(cursor.charFormat());
        selectColorFormat.setForeground(Qt::white);
        cursor.select(QTextCursor::Document);
        cursor.mergeCharFormat(selectColorFormat);
    }

    if (!regFindKeyWords.isEmpty()) {
        //对特殊字符进行处理
        QString escapedKeywords = QRegularExpression::escape(regFindKeyWords);
        QRegularExpression regex(escapedKeywords, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator matchIterator = regex.globalMatch(option.text);

        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            int startPos = match.capturedStart();
            int endPos = match.capturedEnd();

            // 判断是否关键字被省略
            if (isElided && startPos >= elidedText.size() - 1) {
                break; // 关键字被完全省略，退出循环
            }

            // 调整关键字的结束位置
            if (endPos > elidedText.size()) {
                endPos = elidedText.size() ; // 关键字的一部分被省略，将结束位置调整为最后一个字符的位置
            }

            // 执行关键字高亮
            highlightCursor.setPosition(startPos);
            highlightCursor.setPosition(endPos, QTextCursor::KeepAnchor);
            highlightCursor.mergeCharFormat(colorFormat);
        }
    }
    cursor.endEditBlock();
    document.drawContents(painter);

    textLayout.endLayout();
    painter->restore();

    QSizeF docSize = document.size();
    qreal docHeight = docSize.height(); // 获取文档的高度
    return docHeight;
}

QSize IconViewTextHelper::getTextSizeForIndex(const QStyleOptionViewItem &option, const QModelIndex &index, int horizalMargin, int maxLineCount)
{
    int fixedWidth = option.rect.width() - horizalMargin*2;
    QString text = option.text;
    QFont font = option.font;
    QFontMetrics fontMetrics = option.fontMetrics;
    int lineSpacing = fontMetrics.lineSpacing();
    int textHight = 0;

    QTextLayout textLayout(text, font);

    QTextOption opt;
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    opt.setAlignment(Qt::AlignHCenter);

    textLayout.setTextOption(opt);
    textLayout.beginLayout();

    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(fixedWidth);
        textHight += lineSpacing;
    }

    textLayout.endLayout();
    if (maxLineCount > 0) {
        textHight = qMin(maxLineCount * lineSpacing, textHight);
    }

    return QSize(fixedWidth, textHight);
}

void IconViewTextHelper::paintText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, int textMaxHeight, int horizalMargin, int maxLineCount, bool useSystemPalette, const QColor &customColor)
{
    painter->save();
    painter->translate(horizalMargin, 0);

    if (useSystemPalette) {
        if (option.state.testFlag(QStyle::State_Selected))
            painter->setPen(option.palette.highlightedText().color());
        else
            painter->setPen(option.palette.text().color());
    }

    if (customColor != Qt::transparent) {
        painter->setPen(customColor);
    }

    int lineCount = 0;

    QString text = option.text;
    QFont font = option.font;
    QFontMetrics fontMetrics = option.fontMetrics;
    int lineSpacing = fontMetrics.lineSpacing();

    QTextLayout textLayout(text, font);

    QTextOption opt;
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    opt.setAlignment(Qt::AlignHCenter);

    textLayout.setTextOption(opt);
    textLayout.beginLayout();

    int width = option.rect.width() - 2*horizalMargin;

    int y = 0;
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(width);
        int nextLineY = y + lineSpacing;
        lineCount++;

        if (textMaxHeight >= nextLineY + lineSpacing && lineCount != maxLineCount) {
            line.draw(painter, QPoint(0, y));
            y = nextLineY;
        } else {
            QString lastLine = option.text.mid(line.textStart());
            QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width);
            auto rect = QRect(horizalMargin, y /*+ fontMetrics.ascent()*/, width, textMaxHeight);
            //opt.setWrapMode(QTextOption::NoWrap);
            opt.setWrapMode(QTextOption::NoWrap);
            painter->drawText(rect, elidedLastLine, opt);
            //painter->drawText(QPoint(0, y + fontMetrics.ascent()), elidedLastLine);
            line = textLayout.createLine();
            break;
        }
    }
    textLayout.endLayout();

    painter->restore();
}
