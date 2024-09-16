// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETVIEW_H
#define NETVIEW_H

#include "netmanager.h"

#include <QTreeView>

class QSortFilterProxyModel;

// 网络列表，仅界面无业务
namespace dde {
namespace network {

class NetModel;
class NetDelegate;
class NetManager;

class NetView : public QTreeView
{
    Q_OBJECT
public:
    explicit NetView(NetManager *manager);
    ~NetView() override;
    void setForegroundRole(QPalette::ColorRole role);

    bool closeOnClear() const;
    void setCloseOnClear(bool closeOnClear);
    void clear();

    void setMaxHeight(int h) { m_maxHeight = h; }

Q_SIGNALS:
    void requestShow();
    void updateSize();

protected:
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    bool viewportEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void verticalScrollbarValueChanged(int) override;
    void horizontalScrollbarValueChanged(int) override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void updateGeometries() override;
    QModelIndex indexAt(const QPoint &p) const override;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

protected Q_SLOTS:
    void updateByScrollbar();

    void onExec(NetManager::CmdType cmd, const QString &id, const QVariantMap &param);
    void onActivated(const QModelIndex &index);

    void updateLayout();
    void onExpandStatusChanged();
    void updateItemExpand(NetItem *item);
    void scrollToItem(const QString &id);

private:
    QModelIndex traverseAndSearch(const QModelIndex &parent, const QString &id);

private:
    NetManager *m_manager;
    bool m_updateCurrent;
    QSortFilterProxyModel *m_proxyModel;
    NetModel *m_model;
    NetDelegate *m_delegate;
    bool m_closeOnClear;
    bool m_shouldUpdateExpand;
    int m_maxHeight;
};

} // namespace network
} // namespace dde

#endif // NETVIEW_H
