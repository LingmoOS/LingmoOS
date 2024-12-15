// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "middleviewdelegate.h"
#include "middleview.h"

#include "common/vnoteitem.h"
#include "common/vnoteforlder.h"
#include "common/utils.h"
#include "common/standarditemcommon.h"
#include "db/vnoteitemoper.h"

#include "db/vnotefolderoper.h"

#include <DApplicationHelper>

#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

static VNoteFolderOper FolderOper;
/**
 * @brief The VNoteTextPHelper struct
 * 搜索关键字分割字符串高亮显示
 */
struct VNoteTextPHelper {
    VNoteTextPHelper(QPainter *painter, QFontMetrics fontMetrics, QRect nameRect)
        : m_fontMetrics(fontMetrics)
        , m_nameRect(nameRect)
        , m_painter(painter)
    {
        if (nullptr != m_painter) {
            m_pens[OldPen] = m_painter->pen();

            DPalette pl;
            QColor folderNamePenColor = pl.color(DPalette::Highlight);
            m_pens[FolderPen] = folderNamePenColor;
        }
    }

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
    struct Text {
        QString text;
        QRect rect;
        bool isKeyword {false};
    };
#pragma pack(pop) /* restore original alignment from stack */

    enum {
        OldPen,
        FolderPen,
        PenCount
    };
    //分割字符串
    void spiltByKeyword(const QString &text, const QString &keyword);
    //绘制文本
    void paintText(bool isSelected = false);

    QVector<Text> m_textsVector;
    QFontMetrics m_fontMetrics;
    QPen m_pens[PenCount];
    QRect m_nameRect;
    QPainter *m_painter {nullptr};
};

/**
 * @brief VNoteTextPHelper::spiltByKeyword
 * @param text 记事项名称
 * @param keyword 搜索关键字
 */
void VNoteTextPHelper::spiltByKeyword(const QString &text, const QString &keyword)
{
    //Check if text exceed the name rect, elide the
    //text first
    QString elideText = m_fontMetrics.elidedText(text, Qt::ElideRight, m_nameRect.width());

    int keyLen = keyword.length();
    int textLen = text.length();
    int startPos = 0;
    m_textsVector.clear();

    if (!keyword.isEmpty()) {
        int pos = 0;
        while ((pos = elideText.indexOf(keyword, startPos, Qt::CaseInsensitive)) != -1) {
            Text tb;

            if (startPos != pos) {
                int extraLen = pos - startPos;
                tb.text = elideText.mid(startPos, extraLen);
                startPos += extraLen;

                tb.rect = QRect(0, 0, m_fontMetrics.width(tb.text), m_fontMetrics.height());

                m_textsVector.push_back(tb);

                tb.text = elideText.mid(pos, keyLen);
                tb.rect = QRect(0, 0, m_fontMetrics.width(tb.text), m_fontMetrics.height());
                tb.isKeyword = true;
                m_textsVector.push_back(tb);
            } else {
                tb.text = elideText.mid(pos, keyLen);
                tb.rect = QRect(0, 0, m_fontMetrics.width(tb.text), m_fontMetrics.height());
                tb.isKeyword = true;
                m_textsVector.push_back(tb);
            }

            startPos += keyLen;
        }
    }

    if (startPos < elideText.length()) {
        Text tb;

        tb.text = elideText.mid(startPos, (textLen - startPos));
        tb.rect = QRect(0, 0, m_fontMetrics.width(tb.text), m_fontMetrics.height());

        m_textsVector.push_back(tb);
    }
}

/**
 * @brief VNoteTextPHelper::paintText
 * @param isSelected true 绘制项为选中项
 */
void VNoteTextPHelper::paintText(bool isSelected)
{
    int currentX = m_nameRect.x();
    int currentY = m_nameRect.y();

    for (auto it : m_textsVector) {
        int w = it.rect.width();
        int h = m_nameRect.height();

        it.rect.setX(currentX);
        it.rect.setY(currentY);
        it.rect.setSize(QSize(w, h));
        if (it.isKeyword) {
            //If the item is selected,don't need highlight keyword
            m_painter->setPen((isSelected ? m_pens[OldPen] : m_pens[FolderPen]));
        } else {
            m_painter->setPen(m_pens[OldPen]);
        }

        m_painter->drawText(it.rect, Qt::AlignJustify, it.text);

        currentX += it.rect.width();
    }

    //Restore default pen
    m_painter->setPen(m_pens[OldPen]);
}

/**
 * @brief MiddleViewDelegate::MiddleViewDelegate
 * @param parent
 */
MiddleViewDelegate::MiddleViewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{
    m_parentPb = DApplicationHelper::instance()->palette(m_parentView);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this,
            &MiddleViewDelegate::handleChangeTheme);

    handleChangeTheme();
}

/**
 * @brief MiddleViewDelegate::setSearchKey
 * @param key
 */
void MiddleViewDelegate::setSearchKey(const QString &key)
{
    m_searchKey = key;
}

/**
 * @brief MiddleViewDelegate::handleChangeTheme
 */
void MiddleViewDelegate::handleChangeTheme()
{
    m_topIcon = Utils::loadSVG("top.svg");
    m_parentPb = DApplicationHelper::instance()->palette(m_parentView);
    m_parentView->update(m_parentView->currentIndex());
}

/**
 * @brief MiddleViewDelegate::setEnableItem
 * @param enable
 */
void MiddleViewDelegate::setEnableItem(bool enable)
{
    m_enableItem = enable;
}

/**
 * @brief MiddleViewDelegate::setEditIsVisible
 * @param isVisible
 */
void MiddleViewDelegate::setEditIsVisible(bool isVisible)
{
    m_editVisible = isVisible;
}

/**
 * @brief MiddleViewDelegate::sizeHint
 * @param option
 * @param index
 * @return 记事项大小
 */
QSize MiddleViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    if (m_searchKey.isEmpty()) {
        VNoteItem *data = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(index));
        QModelIndex nextIndex = index.siblingAtRow(index.row() + 1);
        VNoteItem *nextData = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(nextIndex));
        int height = 74;
        if (data->isTop) { //需要置顶
            if (index.row() == 0) {
                height += 25; //置顶第一项留出绘制置顶图标位置
            }
            if (nextData) {
                if (!nextData->isTop) {
                    height += 10; //下一个笔记没有置顶留出绘制置顶分割线位置
                }
            } else {
                height += 10; //没有下个笔记留出绘制置顶分割线位置
            }
        }
        return QSize(option.rect.width(), height);
    } else {
        return QSize(option.rect.width(), 102);
    }
}

/**
 * @brief MiddleViewDelegate::createEditor
 * @param parent
 * @param option
 * @param index
 * @return 编辑器窗口
 */
QWidget *MiddleViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QLineEdit *editBox = new QLineEdit(parent);
    editBox->setMaxLength(MAX_TITLE_LEN);
    editBox->resize(parent->geometry().width() - 56, 38);
    editBox->installEventFilter(m_parentView);
    return editBox;
}

/**
 * @brief MiddleViewDelegate::setEditorData
 * @param editor
 * @param index
 */
void MiddleViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QVariant var = index.data(Qt::UserRole + 1);
    VNoteItem *data = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(index));
    if (data) {
        QLineEdit *edit = static_cast<QLineEdit *>(editor);
        edit->setText(data->noteTitle);
    }
}

/**
 * @brief MiddleViewDelegate::setModelData
 * @param editor
 * @param model
 * @param index
 */
void MiddleViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    Q_UNUSED(model);
    QLineEdit *edit = static_cast<QLineEdit *>(editor);
    QString newTitle = edit->text();
    MiddleView *view = static_cast<MiddleView *>(m_parentView);
    //Update note title
    VNoteItem *note = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(index));
    if (note) {
        //Truncate the title name if exceed 64 chars
        if (newTitle.length() > MAX_TITLE_LEN) {
            newTitle = newTitle.left(MAX_TITLE_LEN);
        }

        if (!newTitle.isEmpty() && (note->noteTitle != newTitle)) {
            VNoteItemOper noteOps(note);
            noteOps.modifyNoteTitle(newTitle);
            view->onNoteChanged();
        }
    }
}

/**
 * @brief MiddleViewDelegate::updateEditorGeometry
 * @param editor
 * @param option
 * @param index
 */
void MiddleViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    QLineEdit *edit = static_cast<QLineEdit *>(editor);
    edit->resize(m_parentView->geometry().width() - 56, 38);
    if (m_searchKey.isEmpty() && index.isValid() && index.row() == 0) {
        VNoteItem *data = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(index));
        if (data && data->isTop) {
            return edit->move(option.rect.x() + 28, option.rect.y() + 43);
        }
    }
    edit->move(option.rect.x() + 28, option.rect.y() + 17);
}

/**
 * @brief MiddleViewDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void MiddleViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    if (m_searchKey.isEmpty()) {
        paintNormalItem(painter, option, index);
    } else {
        paintSearchItem(painter, option, index);
    }
}

/**
 * @brief MiddleViewDelegate::paintItemBase
 * @param painter
 * @param option
 * @param rect
 * @param isSelect
 */
void MiddleViewDelegate::paintItemBase(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QRect &rect, bool &isSelect) const
{
    QPainterPath path;
    setPaintPath(rect, path, 0, 0, 8);
    if (option.state & QStyle::State_Selected) {
        if (!m_tabFocus) {
            QColor fillColor = option.palette.color(DPalette::Normal, DPalette::Highlight);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, painter->brush());
        } else {
            paintTabFocusBackground(painter, option, rect);
        }
        painter->setPen(QPen(Qt::white));
        isSelect = true;
    } else {
        isSelect = false;
        if (m_enableItem == false || !(option.state & QStyle::State_Enabled)) {
            painter->setBrush(QBrush(m_parentPb.color(DPalette::Disabled, DPalette::ItemBackground)));
            painter->fillPath(path, painter->brush());
            painter->setPen(QPen(m_parentPb.color(DPalette::Disabled, DPalette::WindowText)));
        } else {
            if (option.state & QStyle::State_MouseOver) {
                painter->setBrush(QBrush(m_parentPb.color(DPalette::Light)));
                painter->fillPath(path, painter->brush());
                painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::WindowText)));
            } else {
                // 计算是否是底部Item
                int h = m_parentView->geometry().height() - 54 - 6;
                if (rect.y() <= h && (rect.y() + rect.height()) >= h) {
                    QLinearGradient linearGradient(QPoint(rect.x(), rect.y()), QPoint(rect.x(), rect.y() + rect.height()));
                    linearGradient.setColorAt(0.1, option.palette.color(DPalette::Normal, DPalette::Highlight));
                    linearGradient.setColorAt(1, Qt::white);
                    painter->setBrush(QBrush(linearGradient));
                    painter->fillPath(path, painter->brush());
                    painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::WindowText)));
                } else {
                    painter->setBrush(QBrush(m_parentPb.color(DPalette::Normal, DPalette::ItemBackground)));
                    painter->fillPath(path, painter->brush());
                    painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::WindowText)));
                }
            }
        }
    }
}

/**
 * @brief MiddleViewDelegate::paintNormalItem
 * @param painter
 * @param option
 * @param index
 */
void MiddleViewDelegate::paintNormalItem(QPainter *painter, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    VNoteItem *data = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(index));
    if (!data) {
        return;
    }

    QRect lineRect;
    QRect topInfoRect;
    QRect itemBaseRect;

    int height = option.rect.height();
    if (height == 99 || height == 109) {
        topInfoRect = QRect(QRect(option.rect.x() + 10, option.rect.y(), option.rect.width() - 20, 30));
        itemBaseRect = QRect(QRect(option.rect.x() + 10, option.rect.y() + 30, option.rect.width() - 20, 64));
        if (height == 109) {
            lineRect = QRect(QRect(option.rect.x() + 10, option.rect.y() + 102, option.rect.width() - 20, 2));
        }
    } else if (height == 84) {
        itemBaseRect = QRect(option.rect.x() + 10, option.rect.y() + 5,
                             option.rect.width() - 20, option.rect.height() - 20);
        lineRect = QRect(QRect(option.rect.x() + 10, option.rect.y() + 78, option.rect.width() - 20, 2));

    } else {
        itemBaseRect = QRect(option.rect.x() + 10, option.rect.y() + 5,
                             qMin(m_parentView->geometry().width(), option.rect.width()) - 20, option.rect.height() - 10);
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHints(QPainter::SmoothPixmapTransform);

    if (topInfoRect.isValid()) {
        //绘制置顶信息
        painter->drawPixmap(QRect(topInfoRect.x(), topInfoRect.y() + 10, 10, 10), m_topIcon);
        topInfoRect.setX(topInfoRect.x() + 15);
        painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::TextTips)));
        painter->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
        painter->drawText(topInfoRect, Qt::AlignLeft | Qt::AlignVCenter, DApplication::translate("MiddleViewDelegate", "Sticky Notes"));
    }

    if (lineRect.isValid()) {
        //绘制分割线
        painter->fillRect(lineRect, m_parentPb.color(DPalette::Normal, DPalette::FrameBorder));
    }

    bool isSelect = false;
    paintItemBase(painter, option, itemBaseRect, isSelect);

    if (isSelect == false || m_editVisible == false) {
        painter->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T6));
        QFontMetrics fontMetrics = painter->fontMetrics();
        int space = (itemBaseRect.height() - fontMetrics.height() * 2) / 2 + itemBaseRect.top();
        QRect nameRect(itemBaseRect.left() + 20, space, itemBaseRect.width() - 40, fontMetrics.height());
        space += fontMetrics.height();
        QRect timeRect(itemBaseRect.left() + 20, space, itemBaseRect.width() - 40, fontMetrics.height());
        QString elideText = fontMetrics.elidedText(data->noteTitle, Qt::ElideRight, nameRect.width());
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elideText);
        if (!isSelect) {
            if (m_enableItem == false || !(option.state & QStyle::State_Enabled)) {
                painter->setPen(QPen(m_parentPb.color(DPalette::Disabled, DPalette::TextTips)));
            } else {
                painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::TextTips)));
            }
        }
        painter->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
        painter->drawText(timeRect, Qt::AlignLeft | Qt::AlignVCenter, Utils::convertDateTime(data->modifyTime));
    }

    painter->restore();
}

/**
 * @brief MiddleViewDelegate::paintSearchItem
 * @param painter
 * @param option
 * @param index
 */
void MiddleViewDelegate::paintSearchItem(QPainter *painter, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    VNoteItem *noteData = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(index));
    if (!noteData) {
        return;
    }
    QRect paintRect(option.rect.x() + 10, option.rect.y() + 5,
                    qMin(m_parentView->geometry().width(), option.rect.width()) - 20, option.rect.height() - 10);
    bool isSelect = false;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHints(QPainter::SmoothPixmapTransform);
    painter->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T6));
    paintItemBase(painter, option, paintRect, isSelect);

    QRect itemRect = paintRect;
    itemRect.setHeight(itemRect.height() - 34);
    painter->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T6));
    QFontMetrics fontMetrics = painter->fontMetrics();

    if (isSelect == false || m_editVisible == false) {
        int space = (itemRect.height() - fontMetrics.height() * 2) / 2 + itemRect.top();
        QRect nameRect(itemRect.left() + 20, space, itemRect.width() - 40, fontMetrics.height());
        space += fontMetrics.height();
        QRect timeRect(itemRect.left() + 20, space, itemRect.width() - 40, fontMetrics.height());
        VNoteTextPHelper vfnphelper(painter, fontMetrics, nameRect);
        vfnphelper.spiltByKeyword(noteData->noteTitle, m_searchKey);
        vfnphelper.paintText(isSelect);

        if (!isSelect) {
            if (m_enableItem == false || !(option.state & QStyle::State_Enabled)) {
                painter->setPen(QPen(m_parentPb.color(DPalette::Disabled, DPalette::TextTips)));
            } else {
                painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::TextTips)));
            }
        }
        painter->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
        painter->drawText(timeRect, Qt::AlignLeft | Qt::AlignTop, Utils::convertDateTime(noteData->modifyTime));
    } else {
        painter->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
    }

    VNoteFolder *folderData = FolderOper.getFolder(noteData->folderId);
    if (folderData) {
        fontMetrics = painter->fontMetrics();
        QRect folderRect = itemRect;
        folderRect.setY(itemRect.bottom());
        QRect iconRect(folderRect.left() + 20, folderRect.top() + (fontMetrics.height() - 24) / 2, 24, 24);
        painter->drawPixmap(iconRect, folderData->UI.icon);
        QRect folderNameRect(iconRect.right() + 12, folderRect.top(),
                             paintRect.width() - iconRect.width() - 50, fontMetrics.height());
        QString elideText = fontMetrics.elidedText(folderData->name, Qt::ElideRight, folderNameRect.width());
        painter->drawText(folderNameRect, Qt::AlignLeft | Qt::AlignVCenter, elideText);
    }
    painter->restore();
}

/**
 * @brief MiddleViewDelegate::setTabFocus
 * @param focus
 */
void MiddleViewDelegate::setTabFocus(bool focus)
{
    m_tabFocus = focus;
}

bool MiddleViewDelegate::isTabFocus()
{
    return m_tabFocus;
}

void MiddleViewDelegate::setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius) const
{
    QPoint path_bottomRight(bgRect.bottomRight().x() - xDifference, bgRect.bottomRight().y() - yDifference);
    QPoint path_topRight(bgRect.topRight().x() - xDifference, bgRect.topRight().y() + yDifference);
    QPoint path_topLeft(bgRect.topLeft().x() + xDifference, bgRect.topLeft().y() + yDifference);
    QPoint path_bottomLeft(bgRect.bottomLeft().x() + xDifference, bgRect.bottomLeft().y() - yDifference);
    path.moveTo(path_bottomRight - QPoint(0, 10));
    path.lineTo(path_topRight + QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_topRight - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(path_topLeft + QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_topLeft), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(path_bottomLeft - QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_bottomLeft - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(path_bottomRight - QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_bottomRight - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);
}

void MiddleViewDelegate::paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &bgRect) const
{
    //初始化绘制高亮色区域路径
    QPainterPath path;
    setPaintPath(bgRect, path, 0, 0, 8);

    //初始化绘制窗口色区域路径
    QPainterPath path2;
    setPaintPath(bgRect, path2, 2, 2, 6);

    //初始化绘制背景色区域路径
    QPainterPath path3;
    setPaintPath(bgRect, path3, 3, 3, 6);

    painter->fillPath(path, option.palette.color(DPalette::Normal, DPalette::Highlight));
    painter->fillPath(path2, option.palette.color(DPalette::Normal, DPalette::Window));
    painter->fillPath(path3, option.palette.color(DPalette::Normal, DPalette::Highlight));
}
