// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "moveview.h"
#include "db/vnotefolderoper.h"
#include "common/vnoteforlder.h"
#include "common/vnoteitem.h"
#include "globaldef.h"

#include <DApplicationHelper>
#include <DWindowManagerHelper>

#include <QPainter>
#include <QPainterPath>

MoveView::MoveView(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasCompositeChanged, this, &MoveView::onCompositeChange);
    onCompositeChange();
}

/**
 * @brief MoveView::setFolder
 * @param folder 记事本数据
 */
void MoveView::setFolder(VNoteFolder *folder)
{
    m_folder = folder;
}

/**
 * @brief MoveView::setNote
 * @param note 笔记数据
 */
void MoveView::setNote(VNoteItem *note)
{
    m_note = note;
}

/**
 * @brief MoveView::setNoteList
 * 设置笔记数据列表
 */
void MoveView::setNoteList(const QList<VNoteItem *> &noteList)
{
    m_noteList = noteList;
}

/**
 * @brief MoveView::paintEvent
 */
void MoveView::paintEvent(QPaintEvent *e)
{
    if (m_hasComposite) {
        paintComposite(e);
    } else {
        paintNormal(e);
    }
}

/**
 * @brief MiddleView::mouseDoubleClickEvent
 * @param event
 * 设置笔记数量
 */
void MoveView::setNotesNumber(int value)
{
    m_notesNumber = value;
}

void MoveView::onCompositeChange()
{
    m_hasComposite = DWindowManagerHelper::instance()->hasComposite();
    setAttribute(Qt::WA_TranslucentBackground, m_hasComposite);
}

void MoveView::paintNormal(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter painter(this);
    //从系统获取画板
    DPalette pb = DApplicationHelper::instance()->applicationPalette();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    QFontMetrics fontMetrics = painter.fontMetrics();
    QRect paintRect = rect();
    QColor background, contour;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        background = "#2A2A2A";
        contour = "#1B1B1B";
    } else {
        background = "#F7F7F7";
        contour = "#E8E8E8";
    }
    painter.fillRect(paintRect, contour);
    paintRect.setTopLeft(paintRect.topLeft() + QPoint(2, 2));
    paintRect.setBottomRight(paintRect.bottomRight() - QPoint(2, 2));
    painter.fillRect(paintRect, background);
    if (m_folder) {
        QColor color;
        color = pb.color(DPalette::Normal, DPalette::Window);
        color.setAlphaF(0.80);
        painter.setBrush(color);
        VNoteFolderOper folderOps(m_folder);
        QString strNum = QString::number(folderOps.getNotesCount());
        int numWidth = fontMetrics.width(strNum);
        int iconSpace = (paintRect.height() - 24) / 2;
        //绘制内部信息
        painter.setPen(QPen(pb.color(DPalette::Normal, DPalette::Text)));
        QRect iconRect(paintRect.left() + 10, paintRect.top() + iconSpace, 24, 24);
        QRect numRect(paintRect.right() - numWidth - 10, paintRect.top(), numWidth, paintRect.height());
        QRect nameRect(iconRect.right() + 12, paintRect.top(),
                       numRect.left() - iconRect.right() - 14, paintRect.height());
        //绘制记事本笔记数量
        painter.drawText(numRect, Qt::AlignRight | Qt::AlignVCenter, strNum);
        //绘制记事本图标
        painter.drawPixmap(iconRect, m_folder->UI.icon);
        //绘制记事本名称
        QString elideText = fontMetrics.elidedText(m_folder->name, Qt::ElideRight, nameRect.width());
        painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elideText);
    } else {
        if (m_notesNumber == 1) {
            QColor color;
            color = pb.color(DPalette::Normal, DPalette::Window);
            painter.setBrush(color);
            painter.setPen(QPen(pb.color(DPalette::Normal, DPalette::Text)));
            paintRect.setLeft(paintRect.left() + 20);
            paintRect.setRight(paintRect.right() - 20);
            //绘制笔记名称
            QString elideText = fontMetrics.elidedText(m_noteList[0]->noteTitle, Qt::ElideRight, paintRect.width());
            painter.drawText(paintRect, Qt::AlignLeft | Qt::AlignVCenter, elideText);
        } else {
            QColor color;
            color = pb.color(DPalette::Normal, DPalette::Window);
            painter.setBrush(color);
            painter.setPen(QPen(pb.color(DPalette::Normal, DPalette::Text)));
            paintRect.setLeft(paintRect.left() + 20);
            paintRect.setRight(paintRect.right() - 40);
            //绘制笔记名称
            QString elideText = fontMetrics.elidedText(m_noteList[0]->noteTitle, Qt::ElideRight, paintRect.width());
            painter.drawText(paintRect, Qt::AlignLeft | Qt::AlignVCenter, elideText);

            //字体大小设置成固定大小，以免显示遮挡
            int widths = 0;
            QString numString = "";
            if (99 < m_notesNumber) {
                widths = fontMetrics.width(("..."));
                numString = "...";
            } else {
                widths = fontMetrics.width(QString::number(m_notesNumber));
                numString = QString::number(m_notesNumber);
            }
            //绘制笔记数量背景圆
            QColor color2("#FD5E5E");
            painter.setPen(QPen(color2));
            painter.setBrush(color2);
            if (m_notesNumber < 10) {
                painter.drawEllipse(QPointF(220, 18), 10, 10);
            } else {
                painter.drawEllipse(QPointF(220, 18), 10 + (widths - 7) / 4, 10 + (widths - 7) / 4);
            }
            QColor color3("#FFFFFF");
            painter.setPen(QPen(color3));
            painter.setBrush(color3);
            //绘制笔记数量
            QRect numberRect(QPoint(212 - ((widths - 7) / 2), 7), QSize(16 + (widths - 7), 19));
            painter.drawText(numberRect, Qt::AlignCenter, numString);
        }
    }
}

void MoveView::paintComposite(QPaintEvent *e)
{
    Q_UNUSED(e)
    bool darkThemeType = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType ? true : false;
    QString imagePath = QString(STAND_ICON_PAHT);
    //深色主题
    if (darkThemeType) {
        imagePath.append("dark/");
    }
    //浅色主题
    else {
        imagePath.append("light/");
    }
    if (m_folder) {
        m_backGroundPixMap = QPixmap(imagePath.append("drag_notePad.svg"));
    } else {
        if (1 < m_notesNumber) {
            m_backGroundPixMap = QPixmap(imagePath.append("drag_notes.svg"));
        } else if (1 == m_notesNumber) {
            m_backGroundPixMap = QPixmap(imagePath.append("drag_note.svg"));
        }
    }
    QPainter painter(this);
    //从系统获取画板
    DPalette pb = DApplicationHelper::instance()->applicationPalette();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    QFontMetrics fontMetrics = painter.fontMetrics();
    QRect paintRect = rect();
    if (m_folder) {
        //设置透明度
        if (!darkThemeType) {
            painter.setOpacity(0.9);
        }
        painter.drawPixmap(rect(), m_backGroundPixMap);
        painter.setOpacity(1);
        //设置背景颜色
        QColor color;
        color = pb.color(DPalette::Normal, DPalette::Window);
        color.setAlphaF(0.80);
        painter.setBrush(color);
        QPainterPath PainterPath;

        VNoteFolderOper folderOps(m_folder);
        QString strNum = QString::number(folderOps.getNotesCount());
        int numWidth = fontMetrics.width(strNum);
        int iconSpace = (paintRect.height() - 24) / 2;
        //绘制内部信息
        painter.setPen(QPen(pb.color(DPalette::Normal, DPalette::Text)));
        QRect iconRect(paintRect.left() + 32, paintRect.top() + iconSpace, 24, 24);
        QRect numRect(paintRect.right() - numWidth - 30, paintRect.top(), numWidth, paintRect.height());
        QRect nameRect(iconRect.right() + 12, paintRect.top(),
                       numRect.left() - iconRect.right() - 14, paintRect.height());
        //绘制记事本笔记数量
        painter.drawText(numRect, Qt::AlignRight | Qt::AlignVCenter, strNum);
        //绘制记事本图标
        painter.drawPixmap(iconRect, m_folder->UI.icon);
        //绘制记事本名称
        QString elideText = fontMetrics.elidedText(m_folder->name, Qt::ElideRight, nameRect.width());
        painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elideText);
    } else {
        //多个笔记拖拽
        if (m_notesNumber > 1) {
            if (darkThemeType) {
                painter.setOpacity(0.98);
            }
            //设置透明度
            painter.drawPixmap(rect(), m_backGroundPixMap);
            painter.setOpacity(1);

            QColor color;
            color = pb.color(DPalette::Normal, DPalette::Window);
            painter.setBrush(color);
            painter.setPen(QPen(pb.color(DPalette::Normal, DPalette::Text)));
            paintRect.setLeft(paintRect.left() + 42);
            paintRect.setRight(paintRect.right() - 42);
            //绘制笔记名称
            QString elideText = fontMetrics.elidedText(m_noteList[0]->noteTitle, Qt::ElideRight, paintRect.width());
            painter.drawText(paintRect, Qt::AlignLeft | Qt::AlignVCenter, elideText);

            //字体大小设置成固定大小，以免显示遮挡
            int widths = 0;
            QString numString = "";
            if (99 < m_notesNumber) {
                widths = fontMetrics.width(("..."));
                numString = "...";
            } else {
                widths = fontMetrics.width(QString::number(m_notesNumber));
                numString = QString::number(m_notesNumber);
            }
            //绘制笔记数量背景圆
            QColor color2("#FD5E5E");
            painter.setPen(QPen(color2));
            painter.setBrush(color2);
            if (m_notesNumber < 10) {
                painter.drawEllipse(QPointF(248, 28), 12.5, 12.5);
            } else {
                painter.drawEllipse(QPointF(248, 28), 12.5 + (widths - 7) / 4, 12.5 + (widths - 7) / 4);
            }
            QColor color3("#FFFFFF");
            painter.setPen(QPen(color3));
            painter.setBrush(color3);
            //绘制笔记数量
            QRect numberRect(QPoint(240 - ((widths - 7) / 2), 19), QSize(16 + (widths - 7), 19));
            painter.drawText(numberRect, Qt::AlignCenter, numString);
        }
        //单个笔记拖拽
        else if (1 == m_notesNumber) {
            if (!darkThemeType) {
                painter.setOpacity(0.9);
            }
            painter.drawPixmap(rect(), m_backGroundPixMap);
            painter.setOpacity(1);
            QColor color;
            color = pb.color(DPalette::Normal, DPalette::Window);
            painter.setBrush(color);
            painter.setPen(QPen(pb.color(DPalette::Normal, DPalette::Text)));
            paintRect.setLeft(paintRect.left() + 42);
            paintRect.setRight(paintRect.right() - 42);
            //绘制笔记名称
            QString elideText = fontMetrics.elidedText(m_noteList[0]->noteTitle, Qt::ElideRight, paintRect.width());
            painter.drawText(paintRect, Qt::AlignLeft | Qt::AlignVCenter, elideText);
        }
    }
}
