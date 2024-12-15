// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEMORYPUBLIC_H
#define MEMORYPUBLIC_H

#include "../3rdparty/core/evaluator.h"
#include "views/memorywidget.h"

#include <QObject>
#include <QList>

/**
 * @brief 同步处理数字内存
 */
class MemoryPublic : public QObject
{
    Q_OBJECT
private:
    explicit MemoryPublic(QObject *parent = nullptr);
    void initconnects();
public:
    /**
     * @brief 当前内存界面所在计算器模式
     */
    enum memorymode {
        standardleft,  //标准模式
        scientificright, //科学模式
        programmerleft //程序员
    };
    static MemoryPublic *instance();
    static void deleteInstance();
    ~MemoryPublic();
    MemoryWidget *getwidget(memorymode mode);
    QList<Quantity> getList();

    void generateData(Quantity answer);
    void memoryplus(Quantity answer);
    void memoryminus(Quantity answer);
    void memoryclean();
    void widgetplus(int row, Quantity answer);
    void widgetminus(int row, Quantity answer);
    void widgetclean(int row, int mode, bool ismenu);
    void setThemeType(int type);
    bool isEmpty();
private:
    MemoryWidget *m_standard_l;
    MemoryWidget *m_scientific_r;
    MemoryWidget *m_programmer_l;

    QList<Quantity> m_list;
    Evaluator *m_evaluator;

signals:
    void generateDataSig(Quantity answer, bool ismax);
    void memorycleanSig();
    void memoryAnsSig(int row, Quantity answer);
    void widgetcleanSig(int row, int mode, bool ismenu);
    void setThemeTypeSig(int type);

public slots:
};

#endif // MEMORYPUBLIC_H
