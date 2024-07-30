// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <KFilePlacesModel>

class FilePlacesModel : public KFilePlacesModel
{
    Q_OBJECT

public:
    FilePlacesModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
};
