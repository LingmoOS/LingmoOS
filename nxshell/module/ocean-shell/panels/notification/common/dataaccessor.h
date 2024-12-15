// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QString>

#include "notifyentity.h"

namespace notification {

class DataAccessor
{
public:
    virtual ~DataAccessor()
    {
    }
    virtual qint64 addEntity(const NotifyEntity &entity) { Q_UNUSED(entity); return 0; }
    virtual qint64 replaceEntity(qint64 id, const NotifyEntity &entity)
    {
        Q_UNUSED(id);
        Q_UNUSED(entity);
        return 0;
    }

    virtual void updateEntityProcessedType(qint64 id, int processedType) { Q_UNUSED(id); Q_UNUSED(processedType); }

    virtual NotifyEntity fetchEntity(qint64 id) { Q_UNUSED(id); return {}; }
    virtual int fetchEntityCount(const QString &appName, int processedType) const { Q_UNUSED(appName); Q_UNUSED(processedType); return 0; }
    virtual NotifyEntity fetchLastEntity(const QString &appName, int processedType) { Q_UNUSED(appName); Q_UNUSED(processedType); return {}; }
    virtual NotifyEntity fetchLastEntity(uint notifyId) { Q_UNUSED(notifyId); return {}; }
    virtual QList<NotifyEntity> fetchEntities(const QString &appName, int processedType, int maxCount)
    {
        Q_UNUSED(appName)
        Q_UNUSED(processedType);
        Q_UNUSED(maxCount)
        return {};
    }
    virtual QList<QString> fetchApps(int maxCount) const { Q_UNUSED(maxCount); return {}; }

    virtual void removeEntity(qint64 id) { Q_UNUSED(id); }
    virtual void removeEntityByApp(const QString &appName) { Q_UNUSED(appName); }
    virtual void clear() {}
    inline static QString AllApp()
    {
        return QLatin1String("AllApp");
    }
};

}
