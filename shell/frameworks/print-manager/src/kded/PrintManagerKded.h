/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTMANAGERKDED_H
#define PRINTMANAGERKDED_H

#include <KDEDModule>

class PrintManagerKded : public KDEDModule
{
    Q_OBJECT
public:
    explicit PrintManagerKded(QObject *parent, const QVariantList &args);
    ~PrintManagerKded() override;
};

#endif // PRINTMANAGERKDED_H
