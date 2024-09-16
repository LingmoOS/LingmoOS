// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLACKLISTWIDGET_H
#define BLACKLISTWIDGET_H

#include <DWidget>
#include <DIconButton>

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QItemSelection>

namespace GrandSearch {

class BlackListWrapper;
class DeleteDialog;
class BlackListWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit BlackListWidget(QWidget *parent = nullptr);
    ~BlackListWidget();

private slots:
    void addButtonClicked();
    void deleteButtonClicked();
    void selectedChanged(const QItemSelection &selected, const QItemSelection &deselected) const;

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_childHLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    QLabel *m_contentLabel = nullptr;

    Dtk::Widget::DIconButton *m_addButton = nullptr;
    Dtk::Widget::DIconButton *m_deleteButton = nullptr;

    BlackListWrapper *m_listWrapper = nullptr;
};
}
#endif // BLACKLISTWIDGET_H
