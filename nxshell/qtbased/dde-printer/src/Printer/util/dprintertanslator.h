// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPRINTERTANSLATOR_H
#define DPRINTERTANSLATOR_H
#include <QObject>
#include <QMap>

class DPrinterTanslator : public QObject
{
    Q_OBJECT
public:
    DPrinterTanslator();

public:
    void init();
    void addTranslate(const QString &, const QString &, const QString &);
    QString translateLocal(const QString &, const QString &, const QString &);

private:
    QMap<QString, QMap<QString, QString>> m_mapTrans;
};

#endif // DPRINTERTANSLATOR_H
