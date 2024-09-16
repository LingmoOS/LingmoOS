// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSGROUP_H
#define SETTINGSGROUP_H

#include "translucentframe.h"

#include <DBackgroundGroup>

#include <QFrame>
#include <QTimer>

class QVBoxLayout;

DWIDGET_USE_NAMESPACE

namespace def {
namespace widgets {

class SettingsItem;
class SettingsHeaderItem;

class SettingsGroup : public TranslucentFrame
{
    Q_OBJECT

public:
    enum BackgroundStyle { ItemBackground = 0, GroupBackground, NoneBackground };

    explicit SettingsGroup(QFrame *parent = nullptr, BackgroundStyle bgStyle = ItemBackground);
    explicit SettingsGroup(const QString &title, QFrame *parent = nullptr);
    ~SettingsGroup();

    SettingsHeaderItem *headerItem() const { return m_headerItem; }

    void setHeaderVisible(const bool visible);

    SettingsItem *getItem(int index);
    void insertItem(const int index, SettingsItem *item);
    void appendItem(SettingsItem *item);
    void appendItem(SettingsItem *item, BackgroundStyle bgStyle);
    void removeItem(SettingsItem *item);
    void moveItem(SettingsItem *item, const int index);
    void setSpacing(const int spaceing);

    int itemCount() const;
    void clear();

    QVBoxLayout *getLayout() const { return m_layout; }

private:
    BackgroundStyle m_bgStyle{ ItemBackground };
    QVBoxLayout *m_layout;
    SettingsHeaderItem *m_headerItem;
    DTK_WIDGET_NAMESPACE::DBackgroundGroup *m_bggroup{ nullptr };
};

} // namespace widgets
} // namespace def

#endif // SETTINGSGROUP_H
