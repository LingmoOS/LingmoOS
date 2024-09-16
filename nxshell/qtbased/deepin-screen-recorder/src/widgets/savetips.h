// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SAVETIPS_H
#define SAVETIPS_H

#include <DLabel>
#include <QPropertyAnimation>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class SaveTips : public DLabel
{
    Q_OBJECT
    Q_PROPERTY(int tipWidth READ tipWidth WRITE setTipWidth NOTIFY tipWidthChanged)

public:
    explicit SaveTips(DWidget *parent = 0);
    ~SaveTips();

signals:
    void tipWidthChanged(int value);

public slots:
    void setSaveText(QString text);
    void startAnimation();
    void endAnimation();

private:
    int m_tipsWidth = 0;
    int tipWidth() const;
    QString m_text;
    void setTipWidth(int tipsWidth);

    QPropertyAnimation *m_startAni;
    QPropertyAnimation *m_stopAni;
};
#endif // SAVETIPS_H
