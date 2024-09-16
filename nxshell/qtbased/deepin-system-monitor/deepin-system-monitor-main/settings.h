// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QVariant>

const QString kSettingKeyDisplayMode = {"display_mode"};
const QString kSettingKeyWindowWidth = {"window_width"};
const QString kSettingKeyWindowHeight = {"window_height"};
const QString kSettingKeyProcessTabIndex = {"process_tab_index_new"};
const QString kSettingKeyProcessAttributeDialogWidth = {"process_attribute_dialog_width"};
const QString kSettingKeyProcessAttributeDialogHeight = {"process_attribute_dialog_height"};
const QString kSettingKeyTimePeriod = {"time_period"};

class QSettings;
class Settings
{
public:
    explicit Settings();
    ~Settings();

    static Settings *instance();
    void flush();

    QVariant getOption(const QString &key, const QVariant &defaultValue = QVariant());
    void setOption(const QString &key, const QVariant &value);

private:
    QSettings *m_settings;
};

#endif  // SETTINGS_H
