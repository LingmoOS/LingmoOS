#ifndef QVARIANTMAP_H
#define QVARIANTMAP_H

#include <QList>
#include <QVariant>
#include <QDBusMetaType>

typedef QMap<QString, QVariant> QVariantMap;

void registerQVariantMapMetaType();

#endif // QVARIANTMAP_H
