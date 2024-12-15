// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONPOPUPWIDGET_H
#define SESSIONPOPUPWIDGET_H

#include <DListView>

DWIDGET_USE_NAMESPACE

class QStandardItemModel;
class SessionPopupWidget : public DListView
{
    Q_OBJECT
public:
    explicit SessionPopupWidget(QWidget *parent = nullptr);

public:
    void setSessionInfo(const QMap<QString, QString> &infos, const QString &curSession);
    void updateCurrentSession(const QString &curSession);

signals:
    void sessionItemClicked(const QString &session);

private:
    void initUI();
    void addItem(const QString &icon, const QString &itemName);
    void itemClicked(const QModelIndex &index);
    void updateSelectedState();

private:
    QStandardItemModel *m_model;
    QString m_curSession;
};

#endif // SESSIONPOPUPWIDGET_H
