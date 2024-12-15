// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FORMLABEL_H
#define FORMLABEL_H

#include <DLabel>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;

class SimpleFormLabel : public QLbtoDLabel
{
    Q_OBJECT
public:
    explicit SimpleFormLabel(const QString &t, QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *event);
};

class SimpleFormField : public QLbtoDLabel
{
    Q_OBJECT
public:
    explicit SimpleFormField(QWidget *parent = nullptr);
protected:
    void resizeEvent(QResizeEvent *event);
};
#endif // FORMLABEL_H
