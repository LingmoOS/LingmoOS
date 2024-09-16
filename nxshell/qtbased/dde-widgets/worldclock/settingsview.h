// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DDialog>
#include <DListView>
DWIDGET_USE_NAMESPACE

namespace dwclock {
class TimezoneModel;
class ZoneChooseView;
class SettingsView : public DDialog {
    Q_OBJECT
public:
    explicit SettingsView(TimezoneModel *model, QWidget *parent = nullptr);
    TimezoneModel *model() const;

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void showModifyLocation(const QModelIndex &index);

public:
    DListView *m_clockView = nullptr;
    TimezoneModel *m_model = nullptr;
    ZoneChooseView* m_chooseView = nullptr;
};
}
