// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef S_FILTER_H_C
#define S_FILTER_H_C


#include <QObject>
#include <QScopedPointer>
#include <QTimer>

class HoverFilter : public QObject
{
    Q_OBJECT
public:
    explicit HoverFilter(QObject *parent = 0);
    bool eventFilter(QObject *obj, QEvent *event);
};

class HoverShadowFilter : public QObject
{
    Q_OBJECT
public:
    explicit HoverShadowFilter(QObject *parent = 0);
    bool eventFilter(QObject *obj, QEvent *event);
};


//class HintFilterPrivate;
class HintFilter;
class HintFilterPrivate
{
public:
    explicit HintFilterPrivate(HintFilter *parent) : q_ptr(parent) {}
    ~HintFilterPrivate(){
        if(nullptr != delayShowTimer){
            delete delayShowTimer;
            delayShowTimer = nullptr;
        }
    }

    void showHint(QWidget *hint);

    QTimer  *delayShowTimer = nullptr;

    QWidget *parentWidget = nullptr;
    QWidget *hintWidget = nullptr;

    HintFilter *q_ptr;
    Q_DECLARE_PUBLIC(HintFilter)
};

class HintFilter: public QObject
{
    Q_OBJECT
public:
    explicit HintFilter(QObject *parent = 0);
    ~HintFilter();

    //void hideAll();
    bool eventFilter(QObject *obj, QEvent *event);
    //void showHitsFor(QWidget *w, QWidget *hint);
private:
    QScopedPointer<HintFilterPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), HintFilter)
};
#endif // S_FILTER_H_C
