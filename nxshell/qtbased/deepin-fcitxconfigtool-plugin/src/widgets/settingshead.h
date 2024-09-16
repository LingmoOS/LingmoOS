// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSHEAD_H
#define SETTINGSHEAD_H

#include "settingsitem.h"
#include "titlelabel.h"
#include <QPushButton>

DWIDGET_BEGIN_NAMESPACE
class DCommandLinkButton;
DWIDGET_END_NAMESPACE
class QGSettings;
namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxNormalLabel;

class FcitxSettingsHead : public FcitxSettingsItem
{
    Q_OBJECT

public:
    enum State {
        Edit,
        Cancel
    };

public:
    explicit FcitxSettingsHead(QFrame *parent = nullptr);
    ~FcitxSettingsHead();

    void setTitle(const QString &title);
    void setEditEnable(bool state = true);
    FcitxTitleLabel* getTitleLabel();
public Q_SLOTS:
    void toEdit();
    void toCancel();
    void setEdit(bool flag);

Q_SIGNALS:
    void editChanged(bool edit);

private Q_SLOTS:
    void refershButton();
    void onClicked();
    void onStatusModeChanged(const QString &key);
private:
    FcitxTitleLabel *m_title;
    DTK_WIDGET_NAMESPACE::DCommandLinkButton *m_edit;
    bool m_editVisible;
    QGSettings *m_gsetting;
    State m_state;
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // SETTINGSHEAD_H
