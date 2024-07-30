/*
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KTEXTEDITOR_DOC_TIP_H
#define KTEXTEDITOR_DOC_TIP_H

#include <QStackedWidget>
#include <QTextBrowser>

class DocTip final : public QFrame
{
public:
    explicit DocTip(QWidget *parent = nullptr);
    void updatePosition(QWidget *completionWidget);

    QWidget *currentWidget();

    void setText(const QString &);
    void setWidget(QWidget *w);

    void clearWidgets();

private:
    QStackedWidget m_stack;
    QTextBrowser *const m_textView;
    std::vector<QWidget *> m_widgets;
};

#endif
