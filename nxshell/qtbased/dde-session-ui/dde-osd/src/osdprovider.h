// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OSDPROVIDER_H
#define OSDPROVIDER_H

#include "abstractosdprovider.h"

class OSDProvider : public AbstractOSDProvider
{
    Q_OBJECT
public:
    explicit OSDProvider(QObject *parent = nullptr);

    // model
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    // delegate
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize contentSize() const override;
    bool match(const QString &param) override;

private:
    void initParamsAndSrc();
    void drawText(QPainter *painter, const QStyleOptionViewItem &option, const QString &txt, const QColor &color) const;

private:
    QMap<QString, QStringList> m_suitableParamsMap;
    QString m_param;
};

#endif // OSDPROVIDER_H
