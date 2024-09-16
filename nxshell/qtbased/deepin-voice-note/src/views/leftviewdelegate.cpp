// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "leftview.h"
#include "leftviewdelegate.h"
#include "common/vnoteforlder.h"
#include "common/standarditemcommon.h"

#include "db/vnotefolderoper.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DLog>

#include <QPainter>
#include <QPainterPath>
#include <QLineEdit>

/**
 * @brief LeftViewDelegate::LeftViewDelegate
 * @param parent
 */
//首个index高度参数
static const int firstIndexHeightParam = 51;
//尾个index高度参数
static const int lastIndexHeightParam = 52;
LeftViewDelegate::LeftViewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_treeView(parent)
{
    init();
}

/**
 * @brief LeftViewDelegate::init
 */
void LeftViewDelegate::init()
{
    m_parentPb = DApplicationHelper::instance()->palette(m_treeView);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this,
            &LeftViewDelegate::handleChangeTheme);
}

/**
 * @brief LeftViewDelegate::createEditor
 * @param parent
 * @param option
 * @param index
 * @return 创建的编辑器
 */
QWidget *LeftViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(index)
    Q_UNUSED(option)
    QLineEdit *editBox = new QLineEdit(parent);
    editBox->setMaxLength(MAX_FOLDER_NAME_LEN);
    editBox->resize(parent->geometry().width() - 68, 30);
    return editBox;
}

/**
 * @brief LeftViewDelegate::setEditorData
 * @param editor
 * @param index
 */
void LeftViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QVariant var = index.data(Qt::UserRole + 2);
    VNoteFolder *data = static_cast<VNoteFolder *>(var.value<void *>());
    if (nullptr != data) {
        QLineEdit *edit = static_cast<QLineEdit *>(editor);
        edit->setText(data->name);
    }
}

/**
 * @brief LeftViewDelegate::setModelData
 * @param editor
 * @param model
 * @param index
 */
void LeftViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    Q_UNUSED(model);
    QLineEdit *edit = static_cast<QLineEdit *>(editor);
    QString text = edit->displayText();

    //Truncate the name if name length exceed 64 chars
    if (text.length() > MAX_FOLDER_NAME_LEN) {
        text = text.left(MAX_FOLDER_NAME_LEN);
    }

    if (!text.isEmpty()) {
        if (StandardItemCommon::getStandardItemType(index) == StandardItemCommon::NOTEPADITEM) {
            VNoteFolder *folderdata = static_cast<VNoteFolder *>(StandardItemCommon::getStandardItemData(index));
            if (folderdata && folderdata->name != text) {
                VNoteFolderOper folderOper(folderdata);
                folderOper.renameVNoteFolder(text);
                static_cast<LeftView*>(m_treeView)->renameVNote(text);
            }
        }
    }
}

/**
 * @brief LeftViewDelegate::updateEditorGeometry
 * @param editor
 * @param option
 * @param index
 */
void LeftViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    QLineEdit *edit = static_cast<QLineEdit *>(editor);
    edit->resize(static_cast<QWidget*>(editor->parent())->geometry().width() - 68, 30);
    int y = 8;
    if (0 == index.row()) {
        y += 5;
    }
    edit->move(option.rect.x() + 50, option.rect.y() + y);
}

/**
 * @brief LeftViewDelegate::handleChangeTheme
 */
void LeftViewDelegate::handleChangeTheme()
{
    m_parentPb = DApplicationHelper::instance()->palette(m_treeView);
    m_treeView->update(m_treeView->currentIndex());
}

/**
 * @brief LeftViewDelegate::sizeHint
 * @param option
 * @param index
 * @return 列表项大小
 */
QSize LeftViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    StandardItemCommon::StandardItemType type = StandardItemCommon::getStandardItemType(index);
    QModelIndex nextIndex = index.siblingAtRow(index.row() + 1);

    //待移动记事本列表行高36,左侧记事本列表行高47px
    int height = (m_isPendingList) ? 36 : 47;
    switch (type) {
    case StandardItemCommon::NOTEPADROOT:
        return QSize(option.rect.width(), 1); //隐藏记事本一级目录
    case StandardItemCommon::NOTEPADITEM:
        //左侧记事本列表调整高度
        if (!m_isPendingList) {
            //第一行空间+4
            if (index.row() == 0) {
                height += 4;
            }
            //最后一行空间+5
            else if (!nextIndex.isValid()) {
                height += 5;
            }
        }
        return QSize(option.rect.width(), height);
    default:
        return DStyledItemDelegate::sizeHint(option, index);
    }
}

/**
 * @brief LeftViewDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void LeftViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    StandardItemCommon::StandardItemType type = StandardItemCommon::getStandardItemType(index);
    switch (type) {
    case StandardItemCommon::NOTEPADROOT:
        return paintNoteRoot(painter, option, index);
    case StandardItemCommon::NOTEPADITEM:
        return paintNoteItem(painter, option, index);
    default:
        return DStyledItemDelegate::paint(painter, option, index);
    }
}

/**
 * @brief LeftViewDelegate::paintNoteRoot
 * @param painter
 * @param option
 * @param index
 */
void LeftViewDelegate::paintNoteRoot(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(painter)
    Q_UNUSED(index)
    Q_UNUSED(option)
}

/**
 * @brief LeftViewDelegate::paintNoteItem
 * @param painter
 * @param option
 * @param index
 */
void LeftViewDelegate::paintNoteItem(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHints(QPainter::SmoothPixmapTransform);
    //绘制区域
    QRect paintRect;
    //区分左侧记事本列表和待移动记事本列表
    if (m_isPendingList) {
        paintRect = QRect(option.rect.x(), option.rect.y(), option.rect.width() - 5, option.rect.height());
    } else {
        if (firstIndexHeightParam == option.rect.height()) {
            paintRect = QRect(option.rect.x() + 10, option.rect.y() + 10,
                              option.rect.width() - 20, option.rect.height() - 14);
        } else if (lastIndexHeightParam == option.rect.height()) {
            paintRect = QRect(option.rect.x() + 10, option.rect.y() + 5,
                              option.rect.width() - 20, option.rect.height() - 14);
        } else {
            paintRect = QRect(option.rect.x() + 10, option.rect.y() + 5,
                              option.rect.width() - 20, option.rect.height() - 10);
        }
    }
    int param = (option.rect.height() == 51) ? 5 : 0;
    param = (option.rect.height() == 52) ? 4 : param;
    QPainterPath path;
    setPaintPath(paintRect, path, 0, 0, 8);
    bool enable = true;
    if (option.state & QStyle::State_Selected) {
        if (!m_tabFocus) {
            QColor fillColor = option.palette.color(DPalette::Normal, DPalette::Highlight);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, painter->brush());
        } else {
            paintTabFocusBackground(painter, option, paintRect);
        }
        painter->setPen(QPen(Qt::white));
    } else {
        if (m_enableItem == false || !(option.state & QStyle::State_Enabled)) {
            painter->setBrush(QBrush(m_parentPb.color(DPalette::Disabled, DPalette::ItemBackground)));
            painter->fillPath(path, painter->brush());
            painter->setPen(QPen(m_parentPb.color(DPalette::Disabled, DPalette::TextTitle)));
            enable = false;
        } else {
            if ((option.state & QStyle::State_MouseOver)) {
                if (m_draging) {
                    QPoint point = m_treeView->mapFromGlobal(QCursor::pos());
                    QModelIndex dstIndex = m_treeView->indexAt(point);
                    QModelIndex srcIndex = m_treeView->currentIndex();
                    QColor fillColor = option.palette.color(DPalette::Normal, DPalette::Highlight);
                    painter->setBrush(QBrush(fillColor));
                    bool underCurrentRect = false;
                    QRect rect = option.rect;
                    if (point.y() > option.rect.topLeft().y() + 0.5 * option.rect.height()) {
                        underCurrentRect = true;
                    } else if (point.y() < option.rect.topLeft().y() + 0.5 * option.rect.height()) {
                        underCurrentRect = false;
                    }
                    if (dstIndex.row() > srcIndex.row()) {
                        if (!underCurrentRect && dstIndex.row() != srcIndex.row() + 1) {
                            int upperParam = lastIndexHeightParam == option.rect.height() ? 4 : 0;
                            rect = QRect(QPoint(option.rect.topLeft().x(), option.rect.topLeft().y() - option.rect.height() + upperParam), QPoint(option.rect.bottomRight().x(), option.rect.bottomRight().y() - option.rect.height() + upperParam));
                        }
                        //左下triangle
                        QRect rc(rect.x() + 12, rect.y() + rect.height() - param - 3, 6, 7);
                        paintTriangle(painter, rc, painter->brush(), true);
                        painter->fillRect(QRect(rect.x() + 15, rect.y() - 1 + rect.height() - param, paintRect.width() - 10, 2), painter->brush());
                        //右下triangle
                        rc = QRect(rect.right() - 15, rect.y() + rect.height() - param - 3, 6, 7);
                        paintTriangle(painter, rc, painter->brush(), false);
                    }
                    //目标索引与当前索引相当时，不绘制横线效果
                    else if (dstIndex.row() < srcIndex.row()) {
                        if (underCurrentRect && dstIndex.row() != srcIndex.row() - 1) {
                            int lowerParam = index.row() == 0 ? 5 : 0;
                            rect = QRect(QPoint(option.rect.topLeft().x(), option.rect.topLeft().y() + option.rect.height() - lowerParam), QPoint(option.rect.bottomRight().x(), option.rect.bottomRight().y() + option.rect.height() - lowerParam));
                        }
                        //左上triangle
                        QRect rc(rect.x() + 12, rect.y() + param - 3, 6, 7);
                        paintTriangle(painter, rc, painter->brush(), true);
                        //调整横线长度
                        painter->fillRect(QRect(rect.x() + 15, rect.y() + param - 1, paintRect.width() - 10, 2), painter->brush());
                        //右上triangle
                        rc = QRect(rect.right() - 15, rect.y() + param - 3, 6, 7);
                        paintTriangle(painter, rc, painter->brush(), false);
                    }
                    painter->setBrush(QBrush(m_parentPb.color(DPalette::Normal, DPalette::ItemBackground)));
                    painter->fillPath(path, painter->brush());
                    painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::TextTitle)));
                } else if (m_drawHover) {
                    painter->setBrush(QBrush(m_parentPb.color(DPalette::Light)));
                    painter->fillPath(path, painter->brush());
                    painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::TextTitle)));
                } else {
                    painter->setBrush(QBrush(m_parentPb.color(DPalette::Normal, DPalette::ItemBackground)));
                    painter->fillPath(path, painter->brush());
                    painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::TextTitle)));
                }
            } else {
                painter->setPen(QPen(m_parentPb.color(DPalette::Normal, DPalette::TextTitle)));
                if (index.row() % 2 == 0 || !m_isPendingList) {
                    painter->setBrush(QBrush(m_parentPb.color(DPalette::Normal, DPalette::ItemBackground)));
                    painter->fillPath(path, painter->brush());
                }
            }
        }
    }

    VNoteFolder *data = static_cast<VNoteFolder *>(StandardItemCommon::getStandardItemData(index));
    if (data != nullptr) {
        VNoteFolderOper folderOps(data);
        QFontMetrics fontMetrics = painter->fontMetrics();
        QString strNum = QString::number(folderOps.getNotesCount());
        int numWidth = fontMetrics.width(strNum);
        int iconSpace = (paintRect.height() - 24) / 2;
        QRect iconRect(paintRect.left() + 11, paintRect.top() + iconSpace, 24, 24);
        QRect numRect(paintRect.right() - numWidth - 7, paintRect.top(), numWidth, paintRect.height());
        QRect nameRect(iconRect.right() + 12, paintRect.top(),
                       numRect.left() - iconRect.right() - 15, paintRect.height());

        if (m_drawNotesNum) {
            painter->drawText(numRect, Qt::AlignRight | Qt::AlignVCenter, strNum);
        }

        if (enable == false) {
            painter->drawPixmap(iconRect, data->UI.grayIcon);
        } else {
            painter->drawPixmap(iconRect, data->UI.icon);
        }

        QString elideText = fontMetrics.elidedText(data->name, Qt::ElideRight, nameRect.width());
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elideText);
    }
    painter->restore();
}

/**
 * @brief LeftViewDelegate::paintTriangle
 * @param painter
 * @param rc
 * @param brush
 * @param left true:三角向右，false:三角向左
 */
void LeftViewDelegate::paintTriangle(QPainter *painter, const QRect &rc, const QBrush &brush, bool left) const
{
    QPainterPath path;
    if (left) {
        path.moveTo(rc.topLeft());
        path.lineTo(rc.right(), rc.top() + (rc.height() / 2));
        path.lineTo(rc.bottomLeft());
    } else {
        path.moveTo(rc.left(), rc.top() + (rc.height() / 2));
        path.lineTo(rc.bottomRight());
        path.lineTo(rc.topRight());
    }
    painter->fillPath(path, brush);
}

/**
 * @brief LeftViewDelegate::setSelectView
 * @param selectView : 是否为待移动记事本列表
 * 更新判断标志
 */
void LeftViewDelegate::setSelectView(bool selectView)
{
    m_isPendingList = selectView;
}

/**
 * @brief LeftViewDelegate::setEnableItem
 * @param enable true 列表项可用
 */
void LeftViewDelegate::setEnableItem(bool enable)
{
    m_enableItem = enable;
}

/**
 * @brief LeftViewDelegate::setDrawNotesNum
 * @param enable true 绘制笔记数目
 */
void LeftViewDelegate::setDrawNotesNum(bool enable)
{
    m_drawNotesNum = enable;
}

/**
 * @brief LeftViewDelegate::setDragState
 * @param state true　正在拖动状态
 */
void LeftViewDelegate::setDragState(bool state)
{
    m_draging = state;
}

/**
 * @brief LeftViewDelegate::setDrawHover
 * @param enable true 绘制hover效果
 */
void LeftViewDelegate::setDrawHover(bool enable)
{
    m_drawHover = enable;
}

/**
 * @brief LeftViewDelegate::setTabFocus
 * @param focus true 有tab焦点，false 无tab焦点
 */
void LeftViewDelegate::setTabFocus(bool focus)
{
    m_tabFocus = focus;
}

/**
 * @brief LeftViewDelegate::isTabFocus
 * @return
 */
bool LeftViewDelegate::isTabFocus()
{
    return m_tabFocus;
}

void LeftViewDelegate::setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius) const
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

void LeftViewDelegate::paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &bgRect) const
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
