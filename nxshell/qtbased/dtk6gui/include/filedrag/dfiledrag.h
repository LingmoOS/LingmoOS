// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEDRAG_H
#define DFILEDRAG_H

#include <dtkgui_global.h>

#include <DObject>

#include <QDrag>
#include <QUrl>

DGUI_BEGIN_NAMESPACE

class DFileDragServer;
class DFileDragPrivate;

class DFileDrag : public QDrag, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
public:
    explicit DFileDrag(QObject *source, DFileDragServer *server);

    QUrl targetUrl();
    void setMimeData(QMimeData *data);

Q_SIGNALS:
    void targetUrlChanged(QUrl url);

private:
    D_DECLARE_PRIVATE(DFileDrag)
};

DGUI_END_NAMESPACE

#endif // DFILEDRAG_H
