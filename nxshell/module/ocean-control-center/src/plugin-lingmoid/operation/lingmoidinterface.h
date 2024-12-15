// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMOIDINTERFACE_H
#define LINGMOIDINTERFACE_H

#include "lingmoidmodel.h"
#include "lingmoidworker.h"

class LingmoIDInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(LingmoidModel *model READ getModel NOTIFY lingmoidModelChanged)
    Q_PROPERTY(LingmoWorker *worker READ getWorker NOTIFY lingmoidWorkerChanged)

public:
    explicit LingmoIDInterface(QObject *parent = nullptr);

    LingmoidModel *getModel() const { return m_model; };
    LingmoWorker *getWorker() const { return m_worker; };

    Q_INVOKABLE QString editionName() const;

signals:
    void lingmoidModelChanged(LingmoidModel *model);
    void lingmoidWorkerChanged(LingmoWorker *worker);

private:
    LingmoidModel *m_model;
    LingmoWorker *m_worker;

};

#endif // LINGMOIDINTERFACE_H
