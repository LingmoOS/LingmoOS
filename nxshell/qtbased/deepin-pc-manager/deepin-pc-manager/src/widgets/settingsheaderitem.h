// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSHEADERITEM_H
#define SETTINGSHEADERITEM_H

#include "settingsitem.h"
#include "titlelabel.h"

#include <QHBoxLayout>
#include <QLabel>

namespace def {
namespace widgets {

class SettingsHeaderItem : public SettingsItem
{
    Q_OBJECT

public:
    explicit SettingsHeaderItem(QWidget *parent = nullptr);

    TitleLabel *textLabel() const { return m_headerText; }

    QHBoxLayout *layout() const { return m_mainLayout; }

    void setTitle(const QString &title);
    void setRightWidget(QWidget *widget);

private:
    QHBoxLayout *m_mainLayout;
    TitleLabel *m_headerText;
};

} // namespace widgets
} // namespace def

#endif // SETTINGSHEADERITEM_H
