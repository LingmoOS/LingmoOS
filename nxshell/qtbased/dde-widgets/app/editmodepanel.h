// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include "instancepanel.h"

DWIDGET_USE_NAMESPACE

WIDGETS_FRAME_BEGIN_NAMESPACE
class WidgetManager;
class EditModePanelCell : public InstancePanelCell {
    Q_OBJECT
public:
    explicit EditModePanelCell(Instance *instance, QWidget *parent = nullptr);
    void init(const QString &title);
    virtual void setView() override;
    virtual QList<QWidget *> focusWidgetList() const override;

Q_SIGNALS:
    void removeWidget(const InstanceId &id);
private:
    QWidget *m_action = nullptr;
};

class EditModePanel : public InstancePanel {
    Q_OBJECT
public:
    explicit EditModePanel (WidgetManager *manager, QWidget *parent = nullptr);
    void init();

    virtual InstancePanelCell *createWidget(Instance *instance) override;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

Q_SIGNALS:
    void editCompleted();
};
WIDGETS_FRAME_END_NAMESPACE
