// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARKWIDGET_H
#define BOOKMARKWIDGET_H

#include <QWidget>

class LeftAreaTextEdit;
class BookMarkWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BookMarkWidget(LeftAreaTextEdit *leftAreaWidget);
    ~BookMarkWidget() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    LeftAreaTextEdit *m_leftAreaWidget;
};

#endif // BOOKMARKWIDGET_H
