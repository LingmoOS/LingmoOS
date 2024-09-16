// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "window/namespace.h"
#include "dtkwidget_global.h"

#include <DTipLabel>

#include <QVBoxLayout>
#include <QEvent>

DEF_NAMESPACE_BEGIN
DWIDGET_USE_NAMESPACE

class UsbInfoLabel : public DTipLabel
{
    Q_OBJECT
public:
    explicit UsbInfoLabel(const QString &text = QString(), QWidget *parent = nullptr);
    ~UsbInfoLabel();
    void setOriginText(QString text);

protected:
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private:
Q_SIGNALS:

public Q_SLOTS:
private:
    QString m_originText;
};

DEF_NAMESPACE_END
