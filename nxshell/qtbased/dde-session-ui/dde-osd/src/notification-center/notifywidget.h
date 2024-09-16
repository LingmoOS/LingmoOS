// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFYWIDGET_H
#define NOTIFYWIDGET_H

#include "notifylistview.h"

#include <QWidget>

class AbstractPersistence;
class AppGroupModel;
class AppGroupDelegate;
class QLabel;
class QListView;
class NotifyModel;

/*!
 * \~chinese \class NotifyWidget
 * \~chinese \brief 通知中心用来存放所有App组信息的窗口
 */
class NotifyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NotifyWidget(QWidget *parent = nullptr, AbstractPersistence *database = nullptr);
    QListView *view() { return m_mainList; }
    NotifyModel *model() { return m_model; }

private:
    void initView(AbstractPersistence *database);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

signals:
    void focusOnButton();

private:
    NotifyListView *m_mainList = nullptr;
    QLabel *m_noNotify = nullptr;
    NotifyModel *m_model;
};

#endif // NOTIFYWIDGET_H
