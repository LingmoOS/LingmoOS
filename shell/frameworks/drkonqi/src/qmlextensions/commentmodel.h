// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include "qobjectlistmodel.h"

#include "bugzillaintegration/libbugzilla/models/comment.h"

class CommentsModel : public QObjectListModel
{
public:
    using QObjectListModel::QObjectListModel;

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

private:
    QList<Bugzilla::Comment::Ptr> m_list;
};
