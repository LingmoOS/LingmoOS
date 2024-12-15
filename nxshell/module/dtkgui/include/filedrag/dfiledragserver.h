// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEDRAGSERVER_H
#define DFILEDRAGSERVER_H

#include "dfiledragcommon.h"

#include <dtkgui_global.h>
#include <DObject>

#include <QObject>

class QMimeData;

DGUI_BEGIN_NAMESPACE

class DFileDragServerPrivate;

class DFileDragServer : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
public:
    explicit DFileDragServer(QObject *parent = nullptr);
    ~DFileDragServer();
    QVariant targetData(const QString &key) const;

public Q_SLOTS:
    void setProgress(int progress);
    void setState(DFileDragState state);

Q_SIGNALS:
    void targetDataChanged(const QString &key);

private:
    D_DECLARE_PRIVATE(DFileDragServer)
    friend class DDndSourceInterface;
    friend class DFileDrag;
};

DGUI_END_NAMESPACE

#endif // DFILEDRAGSERVER_H
