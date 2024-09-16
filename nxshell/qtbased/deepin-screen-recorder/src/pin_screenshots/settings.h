// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H
#include <QSettings>
#include <QObject>

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr);
    static Settings *m_settingsInstance;
    static Settings *instance();
    static void release();

    bool setSaveOption(const QPair<int, int> &value);
    QPair<int, int> getSaveOption();
    void setSavePath(const QString savePathDir);
    QString getSavePath();
    void setIsChangeSavePath(const bool isChange);
    bool getIsChangeSavePath();
private:
    QSettings *m_settings;
};

#endif // SETTINGS_H
