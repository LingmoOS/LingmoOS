/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "desktop-icon-view-delegate.h"
#include "desktop-icon-view.h"

#include "icon-view-editor.h"

#include "file-operation-manager.h"
#include "file-rename-operation.h"
#include "file-batch-rename-operation.h"
#include "file-utils.h"

#include "icon-view-delegate.h"
#include "clipboard-utils.h"
#include "desktop-item-model.h"
#include "emblem-provider.h"

#include <QPushButton>
#include <QWidget>
#include <QPainter>
#include <QFileInfo>
#include <file-info-job.h>

#include <QApplication>

#include <QDebug>
#include <file-info.h>

//qt's global function
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

using namespace Peony;
using namespace Peony::DirectoryView;

DesktopIconViewDelegate::DesktopIconViewDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_styled_button = new QPushButton;
}

DesktopIconViewDelegate::~DesktopIconViewDelegate()
{
    m_styled_button->deleteLater();
}

void DesktopIconViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    auto view = qobject_cast<Peony::DesktopIconView*>(parent());

    auto style = option.widget->style();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    opt.font = qApp->font();
    opt.fontMetrics = qApp->fontMetrics();

    if (view->state() == DesktopIconView::DraggingState) {
        if (auto widget = view->indexWidget(index)) {
            view->setIndexWidget(index, nullptr);
        }

        if (view->selectionModel()->selection().contains(index)) {
            painter->setOpacity(0.8);
        }
    } else if (opt.state.testFlag(QStyle::State_Selected)) {
        if (view->indexWidget(index)) {
            opt.text = nullptr;
        }
    }

    bool bCutFile = false;
    auto clipedUris = ClipboardUtils::getInstance()->getCutFileUris();
    if (!clipedUris.isEmpty()){
        if (clipedUris.contains(index.data(DesktopItemModel::UriRole).toString())) {
            painter->setOpacity(0.5);
            bCutFile = true;
            qDebug()<<"cut item in desktop"<<index.data();
        }
    }

    //paint background
    if (!view->indexWidget(index)) {
        //painter->setClipRect(opt.rect);
        int radius = view->radius();
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        if (opt.state.testFlag(QStyle::State_MouseOver) && !opt.state.testFlag(QStyle::State_Selected)) {
            QColor color = m_styled_button->palette().highlight().color();
            color.setAlpha(255*0.3);//half transparent
            //painter->fillRect(opt.rect, color);
            color.setAlpha(255*0.5);
            painter->setPen(color.darker(100));
            painter->setBrush(color);
            painter->drawRoundedRect(opt.rect.adjusted(1, 1, -1, -1), radius, radius);
        }
        if (opt.state.testFlag(QStyle::State_Selected)) {
            QColor color = m_styled_button->palette().highlight().color();
            color.setAlpha(255*0.7);//half transparent
            //painter->fillRect(opt.rect, color);
            color.setAlpha(255*0.8);
            painter->setPen(color);
            painter->setBrush(color);
            painter->drawRoundedRect(opt.rect.adjusted(1, 1, -1, -1), radius, radius);
        }
        painter->restore();
    }

    //fix bug#46785, select one file cut has no effect issue
    if (bCutFile && !getView()->getEditFlag())/* Rename is index is not set to nullptr,link to bug#61119.modified by 2021/06/22 */
        view->setIndexWidget(index, nullptr);

    auto iconSizeExpected = view->iconSize();
    auto iconRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    int y_delta = iconSizeExpected.height() - iconRect.height();
    opt.rect.translate(0, y_delta);

    int maxTextHight = opt.rect.height() - iconSizeExpected.height() + 10;
    if (maxTextHight < 0) {
        maxTextHight = 0;
    }

    //paint icon item
    auto color = QColor(230, 230, 230);
    opt.palette.setColor(QPalette::Text, color);
    color.setRgb(240, 240, 240);
    opt.palette.setColor(QPalette::HighlightedText, color);

    auto text = opt.text;
    opt.text = nullptr;

    auto state = opt.state;
    if((opt.state & QStyle::State_Enabled) && (opt.state & QStyle::State_Selected))
    {
        opt.state &= ~QStyle::State_Selected;
    }
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    painter->restore();

    opt.state = state;
    opt.text = text;
    opt.font = qApp->font();
    opt.fontMetrics = qApp->fontMetrics();

    painter->save();
    //painter->translate(visualRect.topLeft());
    painter->translate(opt.rect.topLeft());
    painter->translate(0, -y_delta);

    //paint text shadow
    painter->save();
    painter->translate(1, 1 + iconSizeExpected.height() + 10);

    int maxLineCount = 2;

    auto expectedSize = IconViewTextHelper::getTextSizeForIndex(opt, index, 2);
    if(option.fontMetrics.height()*2 > view->viewport()->height() - option.rect.y() - iconSizeExpected.height() - 5) {
        maxLineCount = 1;
    }
    QPixmap pixmap(expectedSize);
    pixmap.fill(Qt::transparent);
    QPainter shadowPainter(&pixmap);
    QColor shadow = Qt::black;
    shadowPainter.setPen(shadow);
    IconViewTextHelper::paintText(&shadowPainter, opt, index, maxTextHight, 0, maxLineCount, false, shadow);
    shadowPainter.end();

    QImage shadowImage(expectedSize + QSize(4, 4), QImage::Format_ARGB32_Premultiplied);
    shadowImage.fill(Qt::transparent);
    shadowPainter.begin(&shadowImage);
    shadowPainter.drawPixmap(2, 2, pixmap);
    qt_blurImage(shadowImage, 8, false, false);

    for (int x = 0; x < shadowImage.width(); x++) {
        for (int y = 0; y < shadowImage.height(); y++) {
            auto color = shadowImage.pixelColor(x, y);
            if (color.alpha() > 0) {
                color.setAlphaF(qMin(color.alphaF() * 1.5, 1.0));
                shadowImage.setPixelColor(x, y, color);
            }
        }
    }

    shadowPainter.end();

    painter->translate(-2, -2);
    painter->drawImage(0, 0, shadowImage);

    //painter->setFont(opt.font);
//    painter->setPen(shadow);
//    IconViewTextHelper::paintText(painter,
//                                  opt,
//                                  index,
//                                  maxTextHight,
//                                  0,
//                                  2,
//                                  false);
    painter->restore();

    //paint text
    painter->save();
    painter->translate(0, 0 + iconSizeExpected.height() + 10);
    //painter->setFont(opt.font);
    painter->setFont(qApp->font());
    QColor textColor = Qt::white;
    textColor.setAlphaF(0.9);
    painter->setPen(textColor);
    IconViewTextHelper::paintText(painter,
                                  opt,
                                  index,
                                  maxTextHight,
                                  0,
                                  maxLineCount,
                                  false);
    painter->restore();

    painter->restore();

    QList<int> emblemPoses = {4, 3, 2, 1}; //bottom right, bottom left, top right, top left

    //paint link icon and locker icon
    FileInfo* file = FileInfo::fromUri(index.data(Qt::UserRole).toString()).get();
    if ((index.data(Qt::UserRole).toString() != "computer:///") && (index.data(Qt::UserRole).toString() != "trash:///")) {
        QSize lockerIconSize = QSize(16, 16);
        int offset = 8;
        switch (view->zoomLevel()) {
        case DesktopIconView::Small: {
            lockerIconSize = QSize(8, 8);
            offset = 10;
            break;
        }
        case DesktopIconView::Normal: {
            break;
        }
        case DesktopIconView::Large: {
            offset = 4;
            lockerIconSize = QSize(24, 24);
            break;
        }
        case DesktopIconView::Huge: {
            offset = 2;
            lockerIconSize = QSize(32, 32);
            break;
        }
        default: {
            break;
        }
        }
        auto topRight = opt.rect.topRight();
        topRight.setX(topRight.x() - opt.rect.width() + 10);
        topRight.setY(topRight.y() + 10);
        auto linkRect = QRect(topRight, lockerIconSize);

        if (! file->canRead())
        {
            emblemPoses.removeOne(1);
            QIcon symbolicLinkIcon = QIcon::fromTheme("emblem-unreadable");
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            symbolicLinkIcon.paint(painter, linkRect, Qt::AlignCenter);
            painter->restore();
        }
        else if(! file->canWrite()/* && ! file->canExecute()*/)
        {
            //只读图标对应可读不可写情况，与可执行权限无关，link to bug#99998
            emblemPoses.removeOne(1);
            QIcon symbolicLinkIcon = QIcon::fromTheme("emblem-readonly");
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            symbolicLinkIcon.paint(painter, linkRect, Qt::AlignCenter);
            painter->restore();
        }
    }

    if (index.data(Qt::UserRole + 1).toBool()) {
        emblemPoses.removeOne(3);
        QSize symbolicIconSize = QSize(16, 16);
        int offset = 8;
        switch (view->zoomLevel()) {
        case DesktopIconView::Small: {
            symbolicIconSize = QSize(8, 8);
            offset = 10;
            break;
        }
        case DesktopIconView::Normal: {
            break;
        }
        case DesktopIconView::Large: {
            offset = 4;
            symbolicIconSize = QSize(24, 24);
            break;
        }
        case DesktopIconView::Huge: {
            offset = 2;
            symbolicIconSize = QSize(32, 32);
            break;
        }
        default: {
            break;
        }
        }
//        auto topRight = opt.rect.topRight();
//        topRight.setX(topRight.x() - offset - symbolicIconSize.width());
//        topRight.setY(topRight.y() + offset);
//        auto linkRect = QRect(topRight, symbolicIconSize);
        //Adjust link emblem to topLeft.link story#8354
        auto topLeft = opt.rect.topLeft();
        topLeft.setX(opt.rect.topLeft().x() + 10);
        topLeft.setY(opt.rect.topLeft().y() + offset + iconRect.height() - symbolicIconSize.height());
        auto linkRect = QRect(topLeft, symbolicIconSize);
        QIcon symbolicLinkIcon = QIcon::fromTheme("emblem-link-symbolic");
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        symbolicLinkIcon.paint(painter, linkRect, Qt::AlignCenter);
        painter->restore();
    }

    // paint extension emblems, FIXME: adjust layout, and implemet on indexwidget, other view.
    auto extensionsEmblems = EmblemProviderManager::getInstance()->getAllEmblemsForUri(file->uri());
    for (auto extensionsEmblem : extensionsEmblems) {
        if (emblemPoses.isEmpty()) {
            break;
        }

        QIcon icon = QIcon::fromTheme(extensionsEmblem);

        QSize emblemsIconSize = QSize(16, 16);
        int offset = 8;
        switch (view->zoomLevel()) {
        case DesktopIconView::Small: {
            emblemsIconSize = QSize(8, 8);
            offset = 10;
            break;
        }
        case DesktopIconView::Normal: {
            break;
        }
        case DesktopIconView::Large: {
            offset = 4;
            emblemsIconSize = QSize(24, 24);
            break;
        }
        case DesktopIconView::Huge: {
            offset = 2;
            emblemsIconSize = QSize(32, 32);
            break;
        }
        default: {
            break;
        }
        }

        if (!icon.isNull()) {
            int pos = emblemPoses.takeFirst();
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            switch (pos) {
            case 1: {
                icon.paint(painter,
                           opt.rect.topLeft().x() + 10,
                           opt.rect.topLeft().y() + 10,
                           emblemsIconSize.width(),
                           emblemsIconSize.height(),
                           Qt::AlignCenter);
                break;
            }
            case 2: {
                icon.paint(painter,
                           opt.rect.topRight().x() - offset - emblemsIconSize.width(),
                           opt.rect.topRight().y() + 10,
                           emblemsIconSize.width(),
                           emblemsIconSize.height(),
                           Qt::AlignCenter);
                break;
            }
            case 3: {
                icon.paint(painter,
                           opt.rect.topLeft().x() + 10,
                           opt.rect.topLeft().y() + offset + iconRect.height() - emblemsIconSize.height(),
                           emblemsIconSize.width(),
                           emblemsIconSize.height(),
                           Qt::AlignCenter);
                break;
            }
            case 4: {
                icon.paint(painter,
                           opt.rect.topRight().x() - offset - emblemsIconSize.width(),
                           opt.rect.topRight().y() + offset + iconRect.height() - emblemsIconSize.height(),
                           emblemsIconSize.width(),
                           emblemsIconSize.height(),
                           Qt::AlignCenter);
                break;
            }
            default:
                break;
            }
            painter->restore();
        }
    }

    /*
    qDebug()<<index.data();
    qDebug()<<opt.rect;
    qDebug()<<view->visualRect(index);
    qDebug()<<view->rectForIndex(index);
    qDebug()<<style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
    */
    painter->restore();
    //return QStyledItemDelegate::paint(painter, option, index);
}

QSize DesktopIconViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    auto view = qobject_cast<DesktopIconView*>(this->parent());
    auto iconSize = view->iconSize();
    QFont font = view->font();
    //font.setFamily(view->font().defaultFamily());
    // asume max text size.
    font.setPointSize(15);
    auto fm = QFontMetrics(font);
    int width = iconSize.width() + 41;
    int height = iconSize.height() + fm.ascent()*2 + 20;
    return QSize(width, height);
}

QWidget *DesktopIconViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto edit = new IconViewEditor(parent);
    auto font = option.font;
    auto view = qobject_cast<Peony::DesktopIconView*>(this->parent());
//    switch (view->zoomLevel()) {
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

    edit->setFont(font);

    edit->setContentsMargins(0, 0, 0, 0);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setMinimumSize(sizeHint(option, index).width(), 54);

    edit->blockSignals(true);
    auto displayString = index.data(Qt::DisplayRole).toString();
    auto uri = index.data(Qt::UserRole).toString();
    auto info = FileInfo::fromUri(uri);
    auto displayName = info->displayName();
    auto suffix = displayName.remove(displayString);
    auto fsType = FileUtils::getFsTypeFromFile(uri);
    if (info->isDesktopFile()) {
        suffix = ".desktop";
    }
    if (fsType.contains("ext")) {
        edit->setMaxLengthLimit(255 - suffix.toLocal8Bit().length());
    } else if (fsType.contains("ntfs")) {
        edit->setLimitBytes(false);
        edit->setMaxLengthLimit(255 - suffix.length());
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

    getView()->setEditFlag(true);
    connect(edit, &IconViewEditor::returnPressed, this, &DesktopIconViewDelegate::slot_finishEdit);
    auto editDestroyConn = connect(edit, &IconViewEditor::destroyed, getView(), [=](){
        getView()->setEditFlag(false);
    });
    connect(getView(), &DesktopIconView::destroyed, edit, [=](){
        disconnect(editDestroyConn);
    });

    return edit;
}

void DesktopIconViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    IconViewEditor *edit = qobject_cast<IconViewEditor*>(editor);
    if (!edit)
        return;

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

void DesktopIconViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
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

void DesktopIconViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
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
           QStringList lists = getView()->getSelections();
            auto renameOp = new FileBatchRenameOperation(lists, newName);
            getView()->setRenaming(true);

            //select file when rename finished
            connect(renameOp, &FileBatchRenameOperation::operationFinished, getView(), [=](){
                auto info = renameOp->getOperationInfo().get();
                auto uri = info->target();
                QTimer::singleShot(100, getView(), [=](){
                    getView()->setSelections(QStringList()<<uri);
                    getView()->scrollToSelection(uri);
                    getView()->setFocus();
                });
            }, Qt::BlockingQueuedConnection);

            fileOpMgr->startOperation(renameOp, true);
        } else {
            auto fileOpMgr = FileOperationManager::getInstance();
            auto renameOp = new FileRenameOperation(index.data(Qt::UserRole).toString(), newName);
            getView()->setRenaming(true);

            //select file when rename finished
            connect(renameOp, &FileRenameOperation::operationFinished, getView(), [=](){
                auto info = renameOp->getOperationInfo().get();
                auto uri = info->target();
                QTimer::singleShot(100, getView(), [=](){
                    getView()->setSelections(QStringList()<<uri);
                    getView()->scrollToSelection(uri);
                    getView()->setFocus();
                });
            }, Qt::BlockingQueuedConnection);

            fileOpMgr->startOperation(renameOp, true);
        }
    }
    else if (newName == oldName)
    {
        //create new file, should select the file or folder
        getView()->selectionModel()->select(index, QItemSelectionModel::Select);
        getView()->setFocus();
    }
}

void DesktopIconViewDelegate::slot_finishEdit()
{
    auto edit = qobject_cast<QWidget *>(sender());
    commitData(edit);
    closeEditor(edit, QAbstractItemDelegate::SubmitModelCache);
    if(edit){
        delete edit;
        edit = nullptr;
    }
    getView()->setEditFlag(false);
}

DesktopIconView *DesktopIconViewDelegate::getView() const
{
    auto view = qobject_cast<Peony::DesktopIconView*>(parent());
    return view;
}

void DesktopIconViewDelegate::initIndexOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    return initStyleOption(option, index);
}
