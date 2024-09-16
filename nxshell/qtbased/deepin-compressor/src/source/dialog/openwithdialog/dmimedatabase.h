// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMIMEDATABASE_H
#define DMIMEDATABASE_H

#include <QMimeDatabase>

class DMimeDatabase : public QMimeDatabase
{
    Q_DISABLE_COPY(DMimeDatabase)

public:
    DMimeDatabase();

    QMimeType mimeTypeForFile(const QString &fileName, MatchMode mode = MatchDefault) const;
    QMimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode = MatchDefault) const;
    // QMimeType mimeTypeForUrl(const QUrl &url) const;
};



#endif // DMIMEDATABASE_H
