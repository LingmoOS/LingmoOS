// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLEENGINE_H
#define COMPATIBLEENGINE_H

#include <QIconEngine>

class CompatibleEnginePrivate;
class CompatibleEngine : public QIconEngine
{
public:
    CompatibleEngine(QString name);

    virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    virtual QIconEngine *clone() const override;
    void virtual_hook(int id, void *data) override;

private:
    QScopedPointer<CompatibleEnginePrivate> d_ptrCompatibleEngine;
    Q_DECLARE_PRIVATE_D(d_ptrCompatibleEngine, CompatibleEngine)
    Q_DISABLE_COPY(CompatibleEngine)
};

#endif // COMPATIBLEENGINE_H
