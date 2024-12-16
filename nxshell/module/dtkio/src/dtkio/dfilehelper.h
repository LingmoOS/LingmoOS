// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEHELPER_H
#define DFILEHELPER_H

#include <map>

#include <QVariant>
#include <QUrl>
#include <QEventLoop>
#include <QTimer>
#include <QSet>

#include <gio/gio.h>

#include "dtkio_global.h"
#include "dtkiotypes.h"
#include "dfileinfo.h"

DIO_BEGIN_NAMESPACE

namespace DFileHelper {

std::map<AttributeID, AttributeDetails> attributeDetailsMap();

std::string attributeKey(AttributeID id);
QVariant attributeDefaultValue(AttributeID id);
AttributeType attributeType(AttributeID id);
QVariant attribute(const QByteArray &key, const AttributeType type, GFileInfo *gfileinfo);
QVariant attributeFromInfo(AttributeID id, GFileInfo *fileinfo, const QUrl &url = QUrl());
QVariant customAttributeFromInfo(AttributeID id, GFileInfo *fileinfo, const QUrl &url = QUrl());
GFile *fileNewForUrl(const QUrl &url);
QSet<QString> hideListFromUrl(const QUrl &url);
bool fileIsHidden(const DFileInfo *dfileinfo, const QSet<QString> &hideList);

bool setAttribute(GFile *gfile, const char *key, AttributeType type, const QVariant &value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);

};   // namespace DFileHelper

class TimeoutHelper
{
    Q_DISABLE_COPY(TimeoutHelper)

public:
    enum {
        NoError,
        Failed,
        Timeout,
    };
    explicit TimeoutHelper(int timeout);
    ~TimeoutHelper();

    inline QVariant result() { return ret; }
    inline void setResult(QVariant result) { ret = result; }
    inline int exec()
    {
        timer->start();
        return blocker->exec();
    }
    inline void exit(int code = NoError) { blocker->exit(code); }
    inline void setTimeout(int msec) { timer->setInterval(msec); }

private:
    QVariant ret;
    QEventLoop *blocker { nullptr };
    QScopedPointer<QTimer> timer { nullptr };
};

DIO_END_NAMESPACE
#endif   // DFILEHELPER_H
