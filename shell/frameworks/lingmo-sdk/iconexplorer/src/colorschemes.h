/*
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUTTLEFISHCOLORSCHEMES_H
#define CUTTLEFISHCOLORSCHEMES_H

#include <QAbstractItemModel>
#include <QObject>
#include <QString>

class KColorSchemeManager;

namespace CuttleFish
{
class ColorSchemes : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel *colorSchemes READ colorSchemes CONSTANT)

public:
    Q_INVOKABLE void activateColorScheme(const QString &name);

    ColorSchemes(QObject *parent);

    QAbstractItemModel *colorSchemes();

private:
    KColorSchemeManager *m_colorManager;
};
}
#endif
