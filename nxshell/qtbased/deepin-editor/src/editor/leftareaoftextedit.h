// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEFTAREAOFTEXTEDIT_H
#define LEFTAREAOFTEXTEDIT_H

#include <QWidget>

class CodeFlodArea;
class BookMarkWidget;
class LineNumberArea;
class TextEdit;

class LeftAreaTextEdit : public QWidget
{
    Q_OBJECT
public:
    explicit LeftAreaTextEdit(TextEdit *textEdit);
    ~LeftAreaTextEdit() override;
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void bookMarkAreaPaintEvent(QPaintEvent *event);
    void codeFlodAreaPaintEvent(QPaintEvent *event);
    void updateLineNumber();
    void updateBookMark();
    void updateCodeFlod();
    void updateAll();
    TextEdit* getEdit();
protected:
    void paintEvent(QPaintEvent *event);
public:
    LineNumberArea *m_pLineNumberArea = nullptr;
    BookMarkWidget *m_pBookMarkArea = nullptr;
    CodeFlodArea *m_pFlodArea = nullptr;

private:
    TextEdit *m_pTextEdit = nullptr;
};

#endif // LEFTAREAOFTEXTEDIT_H
