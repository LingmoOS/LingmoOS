/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2021 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_RESULT_H
#define BALOO_RESULT_H

#include <vector>
#include <QByteArray>

namespace Baloo
{
class Result {
public:
    Result(QByteArray&& path, quint64 id) : filePath(path), documentId(id) {};

    QByteArray filePath;
    quint64 documentId = 0;
};

class ResultList : public std::vector<Baloo::Result> {};

}

#endif // BALOO_RESULT_H
