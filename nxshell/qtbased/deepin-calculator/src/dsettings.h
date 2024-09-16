// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class DSettingsAlt : public QObject
{
    Q_OBJECT
private:
    explicit DSettingsAlt(QObject *parent = nullptr);
    ~DSettingsAlt();
public:
    static DSettingsAlt *instance();

    QVariant getOption(const QString &key);
    void setOption(const QString &key, const QVariant &value);
    static void deleteInstance();

    int getSeparate();  //根据配置文件中的计算器类型自动获取计算器分隔位数
    void setSeparate(int separate); //根据配置文件中的计算器类型自动设置相应的计算器分隔位数
    int getStandardSeparate(); //获取标准计算器分隔位数
    void setStandardSeparate(int separate); //设置标准计算器分隔位数
    int getScientificSeparate(); //获取科学计算器分隔位数
    void setScientificSeparate(int separate); //设置科学计算器分隔位数
    int getProgrammerSeparate(); //获取程序员计算器分隔位数
    void setProgrammerSeparate(int separate); //设置程序员计算器分隔位数

private:
    void initConfig();
    void loadConfig();
    void saveConfig();

private:
    QSettings *m_settings;

    int m_standardSeparate = 3;   //标准计算器分隔位数
    int m_scientificSeparate = 3; //科学计算器分隔位数
    int m_programmerSeparate = 3; //程序员计算器分隔位数
};

#endif
