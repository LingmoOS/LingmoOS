// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <DWidget>
#include <DPlainTextEdit>
#include <QPainter>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

class TextEdit : public DPlainTextEdit
{
    Q_OBJECT
public:
    TextEdit(int index, DWidget *parent);
    ~TextEdit();

public slots:
    void setColor(QColor c);
    //QColor getColor();
    int getIndex();
    void updateCursor();
    void setCursorVisible(bool visible);
    //void keepReadOnlyStatus();
    void setFontSize(int fontsize);
    void updateContentSize(QString content);
    void setEditing(bool edit);
    void setSelecting(bool select);

signals:
    void repaintTextRect(TextEdit *edit,  QRectF newPositiRect);
    void backToEditing();
    void clickToEditing(int index);
    void textEditSelected(int index);
    void textEditFinish(int index);

protected:
    void mousePressEvent(QMouseEvent *e);
    //void wheelEvent(QWheelEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void inputMethodEvent(QInputMethodEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
//    void focusOutEvent(QFocusEvent *e);
private:
    int m_index;
    QColor m_textColor;
    QPainter *m_painter;
    bool m_editing = false;

    QPointF m_pressPoint;
    bool m_isPressed;
    /**
     * @brief 当前光标的位置
     */
    QPoint m_currentCursor;
    /**
     * @brief 当前光标的大小
     */
    int m_cursorHeight = 0;

//    int m_count = 0;

};

#endif // TEXTEDIT_H
