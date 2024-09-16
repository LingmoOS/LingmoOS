// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ATTRIBUTIONWIDGET_H
#define ATTRIBUTIONWIDGET_H

#include <QWidget>

#include "attributemanager.h"

class AttributionWidget : public QWidget, public AttributionManager
{
    Q_OBJECT
public:
    explicit AttributionWidget(QWidget *parent = nullptr);
    void addAttriWidget(QWidget *w);
    void removeAttriWidget(QWidget *w);
    void removeAll();
protected:
    void showAt(int active, const QPoint &pos) override;
    void showWidgets(int active,
                     const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets) override;
private:
    void initWidgetOrder();
signals:

private:
    QList<QWidget *> m_childWidgets;
    QList<int> m_attriShowOrder;
};

#endif // ATTRIBUTIONWIDGET_H
