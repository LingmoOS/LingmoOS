/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "list-view-delegate.h"
#include "file-operation-manager.h"
#include "file-rename-operation.h"
#include "file-batch-rename-operation.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-item.h"

#include "list-view.h"
#include "clipboard-utils.h"

#include "file-info.h"
#include "file-info-job.h"
#include "emblem-provider.h"

#include <QTimer>
#include <QPushButton>

#include <QPainter>
#include <QDBusReply>

#include <QKeyEvent>
#include <QItemDelegate>
#include <file-label-model.h>
#include <QHeaderView>
#include <QApplication>

using namespace Peony;

ListViewDelegate::ListViewDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_styled_button = new QPushButton;
}

ListViewDelegate::~ListViewDelegate()
{
    m_styled_button->deleteLater();
}

void ListViewDelegate::initIndexOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    return initStyleOption(option, index);
}

void ListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.displayAlignment = Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter);

    auto view = qobject_cast<DirectoryView::ListView *>(parent());
    opt.decorationSize = view->iconSize();

    auto model = static_cast<FileItemProxyFilterSortModel*>(view->model());
    auto item = model->itemFromIndex(index);
    if (!item) {
        return;
    }
    auto info = item->info();
    auto colors = info->getColors();

    /* 此处以中文命名的文件保护箱标记实时同步还存在问题，是由于uri编码（尽管使用FileUtils::urlEncoded进行转换）与底层(info的uri)不匹配 */
    QString uri = index.data(Qt::UserRole).toString();
    if(uri.startsWith("favorite://")){/* 快速访问须特殊处理 */
        //快速访问目录，颜色标记设置后更新不及时问题单独处理,修复bug#118015
        uri =FileUtils::getEncodedUri(FileUtils::getTargetUri(uri));
        auto matchInfo = FileInfo::fromUri(uri);
        colors = matchInfo->getColors();
    }

    if (index.column() == 0 && colors.count() >0) {
        if (!view->isDragging() || !view->selectionModel()->selectedIndexes().contains(index)) {
            paintLabel(opt, view->getLabelAlignment(), colors, painter);
        }
    }


    auto clipedUris = ClipboardUtils::getInstance()->getCutFileUris();
    if (!clipedUris.isEmpty()){
        QString actualDirUri = view->getDirectoryUri();
        bool bSearchTab = false;
        if(actualDirUri.startsWith("search:///search_uris")){
            actualDirUri = FileUtils::getActualDirFromSearchUri(actualDirUri);
            bSearchTab = true;
        }

        QString clipedFilesParentUri = ClipboardUtils::getClipedFilesParentUri();
        if ((FileUtils::isSamePath(clipedFilesParentUri, actualDirUri) || (bSearchTab && clipedFilesParentUri.startsWith(actualDirUri)) )
                && !clipedUris.isEmpty()) {
            if (clipedUris.contains(index.data(Qt::UserRole).toString())) {
                painter->setOpacity(0.5);
                //qDebug()<<"cut item in list view"<<index.data();
            }
            else
                painter->setOpacity(1.0);
        }
    }
    else
       painter->setOpacity(1.0);
//    if (!opt.state.testFlag(QStyle::State_Selected)) {
//        if (opt.state & QStyle::State_Sunken) {
//            opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
//        }
//        if (opt.state & QStyle::State_MouseOver) {
//            opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
//        }
//    }

    if (index.column() == 0 && !m_regFindKeyWords.isEmpty()) {
        QString text1 = opt.text;
        opt.text = QString();
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget); //绘制非文本区域内容
        opt.text = text1;
        painter->save();

        QString text = opt.text;
        QFont font = opt.font;
        QFontMetrics fontMetrics = opt.fontMetrics;
        int lineSpacing = fontMetrics.lineSpacing();
        //计算text的长度
        QRect textRect = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
        QString elidedText = fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width());
        painter->translate(textRect.topLeft());
        int yoffset = (textRect.height() - lineSpacing)/2;
        painter->translate(0, yoffset);
        textRect.moveTo(0,0);

        painter->setPen(opt.palette.highlightedText().color());
        QTextOption textOpt;
        textOpt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textOpt.setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

        QTextDocument document;
        document.setDefaultTextOption(textOpt);
        document.setTextWidth(textRect.width());
        document.setDefaultFont(font);
        document.setIndentWidth(0);
        document.setDocumentMargin(0);
        //此处应该设置elidled text
        document.setPlainText(elidedText);

        QTextCursor highlightCursor(&document);
        QTextCursor cursor(&document);
        cursor.beginEditBlock();
        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setBackground(QBrush(QColor(95,208,101)));
        if (opt.state.testFlag(QStyle::State_Selected)) {
            QTextCharFormat selectColorFormat(cursor.charFormat());
            selectColorFormat.setForeground(Qt::white);
            cursor.select(QTextCursor::Document);
            cursor.mergeCharFormat(selectColorFormat);
        }

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
            highlightCursor = document.find(m_regFindKeyWords, highlightCursor);
            if (!highlightCursor.isNull()) {
                highlightCursor.mergeCharFormat(colorFormat);
            }
        }

        cursor.endEditBlock();
        document.drawContents(painter, textRect);
        painter->restore();
    } else {
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    }
    if(view->isEnableMultiSelect()) {
        int selectBox = 0;
        //get current checkbox positon and draw them.
        selectBox = view->getCurrentCheckboxColumn();
        QRect rect = opt.rect;
        int selectBoxPosion = view->viewport()->width()+view->viewport()->x()-view->header()->sectionViewportPosition(selectBox)-48;
        if(index.column() == selectBox)
        {
            if(view->selectionModel()->selectedIndexes().contains(index))
            {
                QIcon icon = QIcon(":/icons/icon-selected.png");
                icon.paint(painter, rect.x()+selectBoxPosion, rect.y()+rect.height()/2-8, 16, 16, Qt::AlignCenter);
            }
            else
            {
                QIcon icon = QIcon(":/icons/icon-select.png");
                icon.paint(painter, rect.x()+selectBoxPosion, rect.y()+rect.height()/2-8, 16, 16, Qt::AlignCenter);
            }
        }
    }
    QList<int> emblemPoses = {4, 3, 2, 1}; //bottom right, bottom left, top right, top left

    //add link and read only icon support
    if (index.column() == 0) {
        auto rect = view->visualRect(index);
        auto iconSize = view->iconSize();
        auto size = iconSize.width()/2;
        bool isSymbolicLink = info->isSymbolLink();
        auto loc_x = rect.x() + iconSize.width() - size/2;
        auto loc_y = rect.y();
        auto iconSizeHeight = iconSize.height();
        //paint symbolic link emblems
        if (isSymbolicLink) {
            emblemPoses.removeOne(3);
            QIcon icon = QIcon::fromTheme("emblem-link-symbolic");
            //qDebug()<<info->symbolicIconName();
            //icon.paint(painter, loc_x, loc_y, size, size);
            //Adjust link emblem to topLeft.link story#8354
            loc_x = rect.x();
            //Special calculation emblems coordinates
            if(iconSize.height() < 28){
                iconSizeHeight = 28;
            }
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            icon.paint(painter, loc_x, loc_y + iconSizeHeight - size/2 - 5, size, size, Qt::AlignCenter);
            painter->restore();
        }

        //paint access emblems
        //NOTE: we can not query the file attribute in smb:///(samba) and network:///.
        loc_x = rect.x();
        if (info->uri().startsWith("file:")) {
            if (!info->canRead()) {
                emblemPoses.removeOne(1);
                QIcon icon = QIcon::fromTheme("emblem-unreadable");
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                icon.paint(painter, loc_x, loc_y, size, size);
                painter->restore();
            } else if (!info->canWrite()/* && !info->canExecute()*/) {
                //只读图标对应可读不可写情况，与可执行权限无关，link to bug#99998
                emblemPoses.removeOne(1);
                QIcon icon = QIcon::fromTheme("emblem-readonly");
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                icon.paint(painter, loc_x, loc_y, size, size);
                painter->restore();
            }
        }

    // paint extension emblems, FIXME: adjust layout, and implemet on indexwidget, other view.
        auto extensionsEmblems = EmblemProviderManager::getInstance()->getAllEmblemsForUri(info->uri());

        //Special calculation emblems coordinates
        if(iconSize.height() < 28){
            iconSizeHeight = 28;
        }

        for (auto extensionsEmblem : extensionsEmblems) {
            if (emblemPoses.isEmpty()) {
               break;
            }

            QIcon icon = QIcon::fromTheme(extensionsEmblem);
            if (!icon.isNull()) {
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                int pos = emblemPoses.takeFirst();
                switch (pos) {
                case 1: {
                   icon.paint(painter, loc_x, loc_y, size, size, Qt::AlignCenter);
                   break;
                }
                case 2: {
                   icon.paint(painter, loc_x + iconSize.width() - size/2, loc_y, size, size, Qt::AlignCenter);
                   break;
                }
                case 3: {
                   icon.paint(painter, loc_x, loc_y + iconSizeHeight - size/2 - 5, size, size, Qt::AlignCenter);
                   break;
                }
                case 4: {
                   icon.paint(painter, loc_x + iconSize.width() - size/2, loc_y + iconSizeHeight - size/2 - 5, size, size, Qt::AlignCenter);
                   break;
                }
                default:
                   break;
                }
                painter->restore();
            }
        }
    }

}

QWidget *ListViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    TextEdit *edit = new TextEdit(parent);
    edit->setAcceptRichText(false);
    //edit->setContextMenuPolicy(Qt::CustomContextMenu);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setWordWrapMode(QTextOption::NoWrap);

    edit->blockSignals(true);
    auto displayString = index.data(Qt::DisplayRole).toString();
    auto displayName = index.data(Qt::UserRole + 1).toString();
    auto uri = index.data(Qt::UserRole).toString();
    auto suffix = displayName.remove(displayString);
    auto fsType = FileUtils::getFsTypeFromFile(uri);
    auto info = FileInfo::fromUri(uri);
    int32_t maxLength = 255;
    if (info->isDesktopFile()) {
        suffix = ".desktop";
    }
    if (FileUtils::isFuseFileSystem(uri)) {
        fsType = "fuse.kyfs";
    }
    if (fsType.contains("ext")) {
        maxLength = 255 - suffix.toLocal8Bit().length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType.contains("ntfs")) {
        edit->setLimitBytes(false);
        maxLength = 255 - suffix.length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType.contains("fuse.kyfs")) {
        edit->setLimitBytes(false);
        QDBusInterface iface ("com.lingmo.file.system.fuse","/com/lingmo/file/system/fuse","com.lingmo.file.system.fuse",QDBusConnection::systemBus());
        QDBusReply<int32_t> reply = iface.call("GetFilenameLength");
        if (reply.isValid()) {
            maxLength = reply.value();
        }
        maxLength = maxLength - suffix.length();
        edit->setMaxLengthLimit(maxLength);
    }
    edit->blockSignals(false);

//    QTimer::singleShot(1, parent, [=]() {
//        this->updateEditorGeometry(edit, option, index);
//    });

//    connect(edit, &TextEdit::textChanged, this, [=]() {
//        updateEditorGeometry(edit, option, index);
//    });

    connect(edit, &TextEdit::textChanged, this, [=]() {
        auto text = edit->toPlainText();
        //fix bug#220283, rename edit position wrong issue
        //short file name no need update to avoid position wrong
        if (text.length() >= maxLength) {
            edit->adjustText();
            updateEditorGeometry(edit, option, index);
        }
    });

    connect(edit, &TextEdit::finishEditRequest, this, &ListViewDelegate::slot_finishEdit);

    connect(edit, &QWidget::destroyed, this, [=]() {
        Q_EMIT isEditing(false);
    });

    return edit;
}

void ListViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    TextEdit *edit = qobject_cast<TextEdit *>(editor);
    if (!edit)
        return;

    Q_EMIT isEditing(true);
    edit->setText(index.data(Qt::DisplayRole).toString());
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

//void ListViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
//    TextEdit *edit = qobject_cast<TextEdit*>(editor);
//    edit->setFixedHeight(editor->height());
//    edit->resize(edit->document()->size().width(), -1);
//}

void ListViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    TextEdit *edit = qobject_cast<TextEdit*>(editor);
    if (!edit)
        return;

    auto text = edit->toPlainText();
    if (text.isEmpty())
        return;
    if (text == index.data(Qt::DisplayRole).toString())
        return;
    //process special name . or .. or only space
    if (text == "." || text == ".." || text.trimmed() == "")
        return;

    if (! index.isValid())
        return;

    auto view = qobject_cast<DirectoryView::ListView *>(parent());
    auto oldName = index.data(Qt::DisplayRole).toString();
    if (text == oldName)
    {
        //create new file, should select the file or folder
        auto flags = QItemSelectionModel::Select|QItemSelectionModel::Rows;
        view->selectionModel()->select(index, flags);
        view->setFocus();
        return;
    }

    if (view->getSelections().count() > 1) {
        auto fileOpMgr = FileOperationManager::getInstance();
        QStringList lists = view->getSelections();
        auto renameOp = new FileBatchRenameOperation(lists, text);

        connect(renameOp, &FileBatchRenameOperation::operationFinished, view, [=](){
            auto info = renameOp->getOperationInfo().get();
            auto uri = info->target();
            QTimer::singleShot(100, view, [=](){
                view->setSelections(QStringList()<<uri);
                //after rename will nor sort immediately, comment to fix bug#60482
                //view->scrollToSelection(uri);
                view->setFocus();
            });
        }, Qt::BlockingQueuedConnection);

        fileOpMgr->startOperation(renameOp, true);
    } else {
        auto fileOpMgr = FileOperationManager::getInstance();
        auto renameOp = new FileRenameOperation(index.data(FileItemModel::UriRole).toString(), text);

        connect(renameOp, &FileRenameOperation::operationFinished, view, [=](){
            auto info = renameOp->getOperationInfo().get();
            auto uri = info->target();
            QTimer::singleShot(100, view, [=](){
                view->setSelections(QStringList()<<uri);
                //after rename will nor sort immediately, comment to fix bug#60482
                //view->scrollToSelection(uri);
                view->setFocus();
            });
        }, Qt::BlockingQueuedConnection);

        fileOpMgr->startOperation(renameOp, true);
    }
}

//not comment this bug to fix bug#93314
QSize ListViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    auto view = qobject_cast<DirectoryView::ListView *>(parent());
    int expectedHeight = view->iconSize().height() + 4;
    size.setHeight(qMax(expectedHeight, size.height()));
    return size;
}

void ListViewDelegate::slot_finishEdit()
{
    auto edit = qobject_cast<QWidget *>(sender());
    commitData(edit);
    closeEditor(edit, QAbstractItemDelegate::SubmitModelCache);
    if(edit){
        delete edit;
        edit = nullptr;
    }
}

void ListViewDelegate::setSearchKeyword(QString regFindKeyWords)
{
    m_regFindKeyWords = regFindKeyWords;
}

void ListViewDelegate::paintLabel(QStyleOptionViewItem &opt, int aalignment, QList<QColor> colors, QPainter *painter) const
{
    //修改标记个数最多为3个，以及标记位置
    int xOffSet = 0;
    int yOffSet = 0;
    int labelSize = 12;
    const int MAX_LABEL_NUM = 3;
    switch(aalignment) {
    case DirectoryView::ListView::LabelAlignment::AlignVertical: {
        xOffSet = 5;
        int index = 0;
        int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
        int num = colors.count() - startIndex + 1;

        //set color label on center, fix bug#40609
        auto iconSize = opt.decorationSize;
        labelSize = iconSize.height()/3;
        if (labelSize > 10)
            labelSize = 10;
        if (labelSize <6)
            labelSize = 6;

        yOffSet = (opt.rect.height()-labelSize*num/2)/2;
        if(yOffSet < 2)
        {
            yOffSet = 2;
        }
        for (int i = startIndex; i < colors.count(); ++i, ++index) {
            auto color = colors.at(i);
            painter->save();
            //fix bug#147348
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            painter->translate(0, opt.rect.topLeft().y());
            painter->translate(2, 0);
            painter->setPen(opt.palette.highlightedText().color());
            painter->setBrush(color);
            painter->drawEllipse(QRectF(xOffSet, yOffSet, labelSize, labelSize));
            painter->restore();
            yOffSet += labelSize/2;
        }
        break;
    }
    case DirectoryView::ListView::LabelAlignment::AlignHorizontal:{
        QRect rect = opt.rect;
        xOffSet = rect.topRight().x() - labelSize/2 - 20;
        yOffSet = rect.height()/2 - labelSize/2;
        int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
        int num =  colors.count() - startIndex;
        int width = rect.width();
        if(num > 0){
            //bug#94242 修改标记位置后和名称重叠，设置标记位置的背景颜色
            QRect markRect = rect;
            markRect.setLeft(rect.width() - (num+1)*labelSize/2 );
            bool isHover = (opt.state & QStyle::State_MouseOver) && (opt.state & ~QStyle::State_Selected);
            bool isSelected = opt.state & QStyle::State_Selected;
            bool enable = opt.state & QStyle::State_Enabled;
            QColor color = opt.palette.color(enable? QPalette::Active: QPalette::Disabled,
                                                 QPalette::Highlight);

            if (isSelected) {
                color.setAlpha(255);
            } else if (isHover) {
                color = opt.palette.color(QPalette::Active, QPalette::BrightText);
                color.setAlphaF(0.05);
            } else {
                color.setAlpha(0);
            }

            painter->save();
            painter->fillRect(markRect, color);
            painter->restore();
            width = width - (num+1)*labelSize/2 - 20;
        }
        for (int i = startIndex; i < colors.count(); ++i) {
            auto color = colors.at(i);
            painter->save();
            //fix bug#147348
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            painter->translate(0, rect.topLeft().y());
            painter->translate(2, 2);
            painter->setPen(opt.palette.highlightedText().color());
            painter->setBrush(color);
            painter->drawEllipse(QRectF(xOffSet, yOffSet, labelSize, labelSize));
            painter->restore();
            xOffSet -= labelSize/2;
        }
        //bug#94242 修改标记位置后和名称重叠，设置汉字宽度
        opt.rect.setWidth(width);
        break;
    }
    }
}

//TextEdit
TextEdit::TextEdit(QWidget *parent) : QTextEdit (parent)
{
    // fix #164278, icon view text editor doesn't cover view item.
    // note on lingmo platform theme, style panel frame is not visible.
    setFrameShape(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    setViewportMargins(1, 2, 1, 2);
}

void TextEdit::adjustText()
{
    if (m_max_length_limit) {
        //fix #154584
        blockSignals(true);
        auto position = textCursor().position();
        while (true) {
            if (m_limit_bytes) {
                auto local8Bit = toPlainText().toLocal8Bit();
                if (local8Bit.length() <= m_max_length_limit) {
                    break;
                }
            } else {
                if (toPlainText().length() <= m_max_length_limit) {
                    break;
                }
            }
            if (position > 0) {
                position--;
                textCursor().beginEditBlock();
                textCursor().setPosition(position);
                textCursor().deletePreviousChar();
                textCursor().endEditBlock();
            } else {
                break;
            }
        }
        blockSignals(false);
    }
}

void TextEdit::setMaxLengthLimit(int length)
{
    m_max_length_limit = length;
}

void TextEdit::setLimitBytes(bool limitBytes)
{
    m_limit_bytes = limitBytes;
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        Q_EMIT finishEditRequest();
        return;
    }
    return QTextEdit::keyPressEvent(e);
}
