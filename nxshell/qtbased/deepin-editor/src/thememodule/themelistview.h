// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THEMELISTVIEW_H
#define THEMELISTVIEW_H

#include <QListView>
#include <QEvent>

class ThemeListView : public QListView
{
    Q_OBJECT

public:
    ThemeListView(QWidget *parent = nullptr);
    ~ThemeListView();

    void adjustScrollbarMargins();

signals:
    void themeChanged(const QString &path);

protected:
    bool eventFilter(QObject *, QEvent *event);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

signals:
    void focusOut();
};

#endif
