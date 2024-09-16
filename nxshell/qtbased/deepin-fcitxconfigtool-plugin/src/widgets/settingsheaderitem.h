// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSHEADERITEM_H
#define SETTINGSHEADERITEM_H

#include "settingsitem.h"
#include "titlelabel.h"

#include <QHBoxLayout>
#include <QLabel>

namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxSettingsHeaderItem : public FcitxSettingsItem
{
    Q_OBJECT

public:
    explicit FcitxSettingsHeaderItem(QWidget *parent = 0);

    FcitxTitleLabel *textLabel() { return m_headerText; }
    QHBoxLayout *layout() const { return m_mainLayout; }

    void setTitle(const QString &title);
    void setRightWidget(QWidget *widget);

private:
    QHBoxLayout *m_mainLayout;
    FcitxTitleLabel *m_headerText;
};

} // namespace widgets
} // namespace dcc

#endif // SETTINGSHEADERITEM_H
