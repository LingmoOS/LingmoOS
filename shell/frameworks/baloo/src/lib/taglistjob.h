/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_TAGLISTJOB_H
#define BALOO_TAGLISTJOB_H

#include <KJob>
#include "core_export.h"

#include <memory>

namespace Baloo {

/**
 * @class TagListJob taglisthjob.h <Baloo/TagListJob>
 */
class BALOO_CORE_EXPORT TagListJob : public KJob
{
    Q_OBJECT
public:
    explicit TagListJob(QObject* parent = nullptr);
    ~TagListJob() override;

    void start() override;
    QStringList tags();

private:
    class Private;
    std::unique_ptr<Private> const d;
};

}

#endif // BALOO_TAGLISTJOB_H
