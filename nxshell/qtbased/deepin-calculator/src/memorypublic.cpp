// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "memorypublic.h"

#include "utils.h"
#include "../3rdparty/core/settings.h"

static MemoryPublic *INSTANCE = nullptr;
const int MAXSIZE = 500; //内存保存最大数

static void s_deletemempub()
{
    delete INSTANCE;
    INSTANCE = nullptr;
}

/**
 * @brief 实例
 */
MemoryPublic *MemoryPublic::instance()
{
    if (!INSTANCE) {
        INSTANCE = new MemoryPublic;
        qAddPostRoutine(s_deletemempub);
    }
    return INSTANCE;
}

void MemoryPublic::deleteInstance()
{
    delete INSTANCE;
    INSTANCE = nullptr;
}

MemoryPublic::MemoryPublic(QObject *parent)
    : QObject(parent)
{
    m_standard_l = new MemoryWidget(0);
    m_scientific_r = new MemoryWidget(1);
    m_programmer_l = new MemoryWidget(2);
    m_evaluator = Evaluator::instance();
    m_standard_l->setMemoryPublic(this);
    m_scientific_r->setMemoryPublic(this);
    m_programmer_l->setMemoryPublic(this);
    initconnects();
    memoryclean();
}

/**
 * @brief 相关信号槽
 */
void MemoryPublic::initconnects()
{
    connect(m_standard_l, &MemoryWidget::widgetclean, this, &MemoryPublic::widgetclean);
    connect(m_scientific_r, &MemoryWidget::widgetclean, this, &MemoryPublic::widgetclean);
    connect(m_programmer_l, &MemoryWidget::widgetclean, this, &MemoryPublic::widgetclean);
}

/**
 * @brief 根据模式获取不同memorywidget
 */
MemoryWidget *MemoryPublic::getwidget(memorymode mode)
{
    switch (mode) {
    case standardleft:
        return m_standard_l;
    case programmerleft:
        return m_programmer_l;
    default:
        return m_scientific_r;
    }
}

QList<Quantity> MemoryPublic::getList()
{
    return m_list;
}

/**
 * @brief 向数字内存中同时增加一个数字
 */
void MemoryPublic::generateData(Quantity answer)
{
    bool ismax = false;
    if (m_list.count() == MAXSIZE) {
        m_list.pop_back();
        ismax = true;
    }
    emit generateDataSig(answer, ismax);
    m_list.insert(0, answer);
}

/**
 * @brief 同时进行M+操作
 */
void MemoryPublic::memoryplus(Quantity answer)
{
    if (!m_list.isEmpty()) {
        Quantity ans;
        ans = m_list.value(0) + answer;
        emit memoryAnsSig(0, ans);
        m_list.replace(0, ans);
    } else
        generateData(answer);
}

/**
 * @brief 同时进行M-操作
 */
void MemoryPublic::memoryminus(Quantity answer)
{
    if (!m_list.isEmpty()) {
        Quantity ans;
        ans = m_list.value(0) - answer;
        emit memoryAnsSig(0, ans);
        m_list.replace(0, ans);
    } else
        generateData(Quantity(-1)*answer);

}

/**
 * @brief 同时进行清除内存操作
 */
void MemoryPublic::memoryclean()
{
    m_list.clear();
    emit memorycleanSig();
}

/**
 * @brief 用于从列表中item里的按钮，对指定行号的内存数据进行M+
 */
void MemoryPublic::widgetplus(int row, Quantity answer)
{
    if (!answer.isZero()) {
        Quantity ans;
        ans = m_list.value(row) + answer;
        emit memoryAnsSig(row, ans);
        m_list.replace(row, ans);
    }
}

/**
 * @brief 用于从列表中item里的按钮，对指定行号的内存数据进行M-
 */
void MemoryPublic::widgetminus(int row, Quantity answer)
{
    if (!answer.isZero()) {
        Quantity ans;
        ans = m_list.value(row) - answer;
        emit memoryAnsSig(row, ans);
        m_list.replace(row, ans);
    }
}

/**
 * @brief 用于从列表中item里的按钮，对指定行号的内存数据进行MC
 */
void MemoryPublic::widgetclean(int row, int mode, bool ismenu)
{
    m_list.removeAt(row);
    emit widgetcleanSig(row, mode, ismenu);
}

/**
 * @brief 根据主题变换垃圾桶及内存中没有数据字体颜色
 */
void MemoryPublic::setThemeType(int type)
{
    emit setThemeTypeSig(type);
}

/**
 * @brief 返回内存中是否有数据
 */
bool MemoryPublic::isEmpty()
{
    return m_list.isEmpty();
}

MemoryPublic::~MemoryPublic()
{

}
