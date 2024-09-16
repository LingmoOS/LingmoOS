// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include "../../3rdparty/core/evaluator.h"
#include "../control/iconbutton.h"
#include "memoryitemwidget.h"
#include "memorylistwidget.h"
#include "memoryitemdelegate.h"

#include <DPushButton>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QList>
#include <QObject>

/**
 * @brief 标准模式内存界面
 */
class MemoryPublic;
class MemoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemoryWidget(int mode = 0, QWidget *parent = nullptr);
    ~MemoryWidget() override;

    void generateData(const Quantity answer, bool ismax);
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void memoryAnsChanged(int row, const Quantity answer);
    void memoryclean();//内存列表为空时插入一个item
    QPair<QString, Quantity> getfirstnumber();//MR
    MemoryListWidget *getMemoryWidget();
    QString programmerResult(const Quantity answer);
    void resetAllLabelByBase();
    void resetLabelBySeparator();

public slots:
    void setThemeType(int type);
    void expressionempty(bool b);
    void widgetcleanslot(int row, int mode, bool ismenu);
    void setMemoryPublic(MemoryPublic *pub);

signals:
    void widgetplus(int row);
    void widgetminus(int row);
    void insidewidget();
    void itemclick(const QPair<QString, Quantity>);
    void themechange(int type);
    void widgetclean(int row, int mode, bool ismenu); //是否通过menu点击发出，用于区分是否下一个item直接进入hover状态
    void hideWidget();

private:
    void initConnect();
    QString formatExpression(const QString &text);
    QString setitemwordwrap(const QString &text, int row = 0);
    void emptymemoryfontcolor();
    QString programmerWrap(QString result);
    int fontHeight();

private slots:
    void resetItemHeight();

private:
    MemoryListWidget *m_listwidget;
    QPoint m_mousepoint;
    IconButton *m_clearbutton;
    Evaluator *m_evaluator;
    Quantity m_ans;
    bool m_ansVaild;
    int m_themetype = 0;
    int m_calculatormode;//0-标准下拉 1-科学右侧
    int m_line = 1; //item数字行数
    MemoryItemDelegate *m_memoryDelegate;
    int m_itemwidth = 0;
    int m_precision = -1;//计算精度
    QLabel *m_label;//内存为空item
    int m_currentrow = 0;//listwidget当前行
    MemoryPublic *m_memorypublic = nullptr;
};


#endif // MEMORYWIDGET_H
