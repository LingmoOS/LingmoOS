// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUBBLETITLEWIDGET_H
#define BUBBLETITLEWIDGET_H
#include "notification/constants.h"
#include "notifymodel.h"
#include "bubbleitem.h"
#include <DWidget>
#include <DIconButton>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class NotifyListView;
class CicleIconButton;

class BubbleTitleWidget : public BubbleBase
{
    Q_OBJECT
public:
    BubbleTitleWidget(NotifyModel *model = nullptr, EntityPtr entity = nullptr, QWidget *parent = nullptr);

public:
    void setIndexRow(int row);
    int indexRow() { return m_indexRow; }
    QList<QPointer<QWidget>> bubbleElements() const override;
    static int bubbleTitleWidgetHeight();

private Q_SLOTS:
    void toggleNotificationFolding();

private:
    void updateNotificationFoldingStatus();

    DLabel *m_titleLabel;
    CicleIconButton *m_toggleNotificationFolding = nullptr;
    int m_indexRow = 0;
};

#endif // BUBBLETITLEWIDGET_H
