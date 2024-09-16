// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KBLAYOUTLISTVIEW
#define KBLAYOUTLISTVIEW

#include <DListView>

#include "constants.h"
#include "util_updateui.h"
#include "xkbparser.h"

DWIDGET_USE_NAMESPACE

class KBLayoutListView: public DListView
{
    Q_OBJECT

public:
    KBLayoutListView(const QString &language = QString(), QWidget *parent = nullptr);
    ~KBLayoutListView() override;

    void initData(const QStringList &buttons);
    void updateList(const QString &str);

signals:
    void itemClicked(const QString &str);

public slots:
    void onItemClick(const QModelIndex &index);

private:
    void initUI();
    void updateSelectState(const QString &name);
    void addItem(const QString &name);

private:
    QStringList m_buttons;
    XkbParser *m_xkbParse;
    QStringList m_kbdParseList;

    QStandardItemModel *m_buttonModel;
    QString m_curLanguage;
    bool m_clickState;
};
#endif // KBLAYOUTLISTVIEW
