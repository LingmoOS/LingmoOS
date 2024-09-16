// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACE_P_H
#define WORKSPACE_P_H

#include "global_defines.h"
#include "workspacewidget.h"
#include "gui/utils/sortfilterworker.h"

#include <QStackedLayout>
#include <QThread>

#ifndef linux
#    include <gui/win/cooperationsearchedit.h>
#endif

namespace cooperation_core {

class WorkspaceWidget;
class LookingForDeviceWidget;
class NoNetworkWidget;
class NoResultWidget;
class DeviceListWidget;
class FirstTipWidget;
class BottomLabel;
class WorkspaceWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    explicit WorkspaceWidgetPrivate(WorkspaceWidget *qq);
    ~WorkspaceWidgetPrivate();

    void initUI();
    void initConnect();

public Q_SLOTS:
    void onSearchValueChanged(const QString &text);
    void onSortFilterResult(int index, const DeviceInfoPointer info);
    void onFilterFinished();
    void onDeviceRemoved(int index);
    void onDeviceUpdated(int index, const DeviceInfoPointer info);
    void onDeviceMoved(int from, int to, const DeviceInfoPointer info);
    void onSearchDevice();

Q_SIGNALS:
    void devicesAdded(const QList<DeviceInfoPointer> &infoList);
    void devicesRemoved(const QString &ip);
    void filterDevice(const QString &str);
    void clearDevice();

public:
    WorkspaceWidget *q { nullptr };
    QStackedLayout *stackedLayout { nullptr };
    QLabel *deviceLabel { nullptr };
    CooperationSearchEdit *searchEdit { nullptr };
    FirstTipWidget *tipWidget { nullptr };
    BottomLabel *bottomLabel { nullptr };
    LookingForDeviceWidget *lfdWidget { nullptr };
    NoNetworkWidget *nnWidget { nullptr };
    NoResultWidget *nrWidget { nullptr };
    DeviceListWidget *dlWidget { nullptr };

    WorkspaceWidget::PageName currentPage { WorkspaceWidget::kUnknownPage };
    QSharedPointer<SortFilterWorker> sortFilterWorker { nullptr };
    QSharedPointer<QThread> workThread { nullptr };
};

}   // namespace cooperation_core {

#endif   // WORKSPACE_P_H
