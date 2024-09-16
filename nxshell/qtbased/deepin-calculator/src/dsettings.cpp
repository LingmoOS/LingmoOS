// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsettings.h"

#include <QDir>
#include <QDesktopWidget>
#include <QDebug>
#include <DApplication>

#include "utils.h"

DWIDGET_USE_NAMESPACE
static DSettingsAlt *INSTANCE = nullptr;

/**
 * @brief 初始化
 */
DSettingsAlt *DSettingsAlt::instance()
{
    if (!INSTANCE) {
        INSTANCE = new DSettingsAlt();
        qAddPostRoutine(deleteInstance);
    }

    return INSTANCE;
}

/**
 * @brief 相关信息保存在config.config文件中
 */
DSettingsAlt::DSettingsAlt(QObject *parent)
    : QObject(parent),
      m_settings(new QSettings(QDir(Utils::getConfigDir()).filePath("config.config"), QSettings::IniFormat, this))
{
    initConfig();
    loadConfig();
}

DSettingsAlt::~DSettingsAlt()
{
//    delete m_settings;
    //saveConfig();
}

void DSettingsAlt::loadConfig()
{
    m_standardSeparate = m_settings->value("standardSeparate", 3).toInt();  //默认3位
    m_scientificSeparate = m_settings->value("scientificSeparate", 3).toInt();
    m_programmerSeparate = m_settings->value("programmerSeparate", 3).toInt();
}

void DSettingsAlt::saveConfig()
{
    m_settings->setValue("standardSeparate", m_standardSeparate);
    m_settings->setValue("scientificSeparate", m_scientificSeparate);
    m_settings->setValue("programmerSeparate", m_programmerSeparate);
}

void DSettingsAlt::initConfig()
{
    if (m_settings->value("theme").toString().isEmpty()) {
        setOption("theme", "light"); //主题初始化为浅色
    }

    if (m_settings->value("mode").toString().isEmpty()) {
        setOption("mode", 0); //模式初始化为标准模式
    }

    if (m_settings->value("history").toString().isEmpty()) {
        setOption("history", 0); //历史记录初始化关闭
    }
    if (m_settings->value("windowX").toString().isEmpty()) {
        setOption("windowX", 0);  //保存当前打开位置x
    }
    if (m_settings->value("windowY").toString().isEmpty()) {
        setOption("windowY", 0); //保存当前打开位置y
    }
    if (m_settings->value("windowWidth").toString().isEmpty()) {
        setOption("windowWidth", 0); //保存窗口width
    }
    if (m_settings->value("windowHeight").toString().isEmpty()) {
        setOption("windowHeight", 0); //保存窗口height
    }
}

/**
 * @brief 获取相关参数
 */
QVariant DSettingsAlt::getOption(const QString &key)
{
    return m_settings->value(key);
}

/**
 * @brief 以相关键值保存参数
 * @param key 键
 * @param value 值
 */
void DSettingsAlt::setOption(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
}

void DSettingsAlt::deleteInstance()
{
    delete INSTANCE;
    INSTANCE = nullptr;
}

/**
 * @brief DSettingsAlt::getSeparate
 * @return 根据配置文件中的计算器类型返回相应的计算器分隔位数
 */
int DSettingsAlt::getSeparate()
{
    int separate = 3;   //数字分割位数
    switch (m_settings->value("mode", 0).toInt()) {
    case 0: separate = getStandardSeparate();
        break;
    case 1: separate = getScientificSeparate();
        break;
    case 2: separate = getProgrammerSeparate();
        break;
    }

    //间隔位数小于1时转为3，防止发生一些意外情况
    if (separate < 1) {
        separate = 3;
    }
    return separate;
}

/**
 * @brief DSettingsAlt::setSeparate
 * 根据配置文件中的计算器类型设置相应的计算器分隔位数
 * @param separate  间隔位数
 */
void DSettingsAlt::setSeparate(int separate)
{
    switch (m_settings->value("mode", 0).toInt()) {
    case 0: setStandardSeparate(separate);
        break;
    case 1: setScientificSeparate(separate);
        break;
    case 2: setProgrammerSeparate(separate);
        break;
    }

    saveConfig();
}

/**
 * @brief getStandardSeparate
 * 获取标准计算器分隔位数
 * @return 标准计算器分隔位数
 */
int DSettingsAlt::getStandardSeparate()
{
    return m_standardSeparate;
}

/**
 * @brief setStandardSeparate
 * 设置标准计算器分隔位数
 */
void DSettingsAlt::setStandardSeparate(int separate)
{
    this->m_standardSeparate = separate;
}

/**
 * @brief getScientificSeparate
 * 获取科学计算器分隔位数
 * @return
 */
int DSettingsAlt::getScientificSeparate()
{
    return m_scientificSeparate;
}

/**
 * @brief setScientificSeparate
 * 设置科学计算器分隔位数
 */
void DSettingsAlt::setScientificSeparate(int separate)
{
    this->m_scientificSeparate = separate;
}

/**
 * @brief getProgrammerSeparate
 * @return 程序员计算器分隔位数
 */
int DSettingsAlt::getProgrammerSeparate()
{
    return m_programmerSeparate;
}

/**
 * @brief setProgrammerSeparate
 * 设置程序员计算器分隔位数
 */
void DSettingsAlt::setProgrammerSeparate(int separate)
{
    this->m_programmerSeparate = separate;
}
