// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PERMISSIONOPTION
#define PERMISSIONOPTION

#include <QString>
#include <QMap>


struct OptionData;

class PermissionOption {

public:
    static PermissionOption *getInstance();

    QString getTs(QString optionName);

    int getId(QString optionName);

    bool isNeedSaved(QString optionName);

    bool isNeedSavedByIndex(int index);

private:
    PermissionOption();
    QMap<QString, OptionData> m_data;
};

#endif // PERMISSIONOPTION
