#include "qvariantmap.h"

void registerQVariantMapMetaType()
{
    qRegisterMetaType<QVariantMap>("QVariantMap");
    qDBusRegisterMetaType<QVariantMap>();
}
