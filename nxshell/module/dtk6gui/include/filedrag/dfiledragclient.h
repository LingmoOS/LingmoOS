// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEDRAGCLIENT_H
#define DFILEDRAGCLIENT_H

#include "dfiledragcommon.h"

#include <dtkgui_global.h>
#include <DObject>

#include <QObject>

class QMimeData;

DGUI_BEGIN_NAMESPACE

class DFileDragClientPrivate;
class DFileDragClient : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
public:
    explicit DFileDragClient(const QMimeData *data, QObject *parent = nullptr);
    int progress() const;
    DFileDragState state() const;

Q_SIGNALS:
    void progressChanged(int progress);
    void stateChanged(DFileDragState state);
    void serverDestroyed();

public:
    static bool checkMimeData(const QMimeData *data);
    static void setTargetData(const QMimeData *data, QString key, QVariant value);
    static void setTargetUrl(const QMimeData *data, QUrl url);

private:
    D_DECLARE_PRIVATE(DFileDragClient)
};

DGUI_END_NAMESPACE

#endif // DFILEDRAGCLIENT_H
