/*
    SPDX-FileCopyrightText: 2024 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEARGUMENTHINTTREE_H
#define KATEARGUMENTHINTTREE_H

#include <QFrame>

class KateCompletionWidget;
class KateArgumentHintModel;
class QPlainTextEdit;
class QLabel;
class QFont;
class ArgumentHighlighter;

class ArgumentHintWidget : public QFrame
{
public:
    explicit ArgumentHintWidget(KateArgumentHintModel *model, const QFont &font, KateCompletionWidget *completion, QWidget *parent);

    void positionAndShow();
    void clearAndHide();

    void selectNext();
    void selectPrevious();

    void updateGeometry();

private:
    void activateHint(int i, int rowCount);

    KateCompletionWidget *const m_completionWidget;
    QPlainTextEdit *const m_view;
    QLabel *const m_currentIndicator;
    int m_current = -1;
    KateArgumentHintModel *const m_model;
    ArgumentHighlighter *const m_highlighter;
    QWidget *const m_leftSide;
};

#endif
